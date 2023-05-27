#include "Visit.h"
using namespace std;

koopa_raw_function_t f; // be used in `store` to get the number of parameter
int LabelCounter = 0;   // riscv生成时，使用到的临时标号
string RiscvString;
StackAddressAllocator saa;

void Visit(const koopa_raw_program_t &program) {
    // 访问所有全局变量
    Visit(program.values);
    // 访问所有函数
    Visit(program.funcs);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) {
    for (size_t i = 0; i < slice.len; ++i) {
        auto ptr = slice.buffer[i];
        // 根据 slice 的 kind 决定将 ptr 视作何种元素
        switch (slice.kind) {
        case KOOPA_RSIK_FUNCTION:
            // 访问函数
            Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
            break;
        case KOOPA_RSIK_BASIC_BLOCK:
            // 访问基本块
            Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
            break;
        case KOOPA_RSIK_VALUE:
            // 访问指令/全局变量
            Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
            break;
        default:
            // 我们暂时不会遇到其他内容, 于是不对其做任何处理
            assert(false);
        }
    }
}

// 访问函数
void Visit(const koopa_raw_function_t &func) {
    if(func->bbs.len == 0) return;
    f=func;

    RiscvString += "  .text\n";
    RiscvString += "  .globl " + string(func->name + 1) + "\n";
    RiscvString += string(func->name + 1)+ ":\n";

    saa.clear();
    // 先扫一遍完成局部变量分配
    for(size_t i = 0; i < func->bbs.len; ++i){
        auto bb = reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[i]);
        for(int j = 0; j < bb->insts.len; ++j){
            auto value = reinterpret_cast<koopa_raw_value_t>(bb->insts.buffer[j]);
            // 特判alloc
            if(value->kind.tag == KOOPA_RVT_ALLOC ){
                int sz = getTypeSize(value->ty->data.pointer.base);
                saa.alloc(value, sz);
                continue;
            }
            if(value->kind.tag == KOOPA_RVT_CALL){
                koopa_raw_call_t c = value->kind.data.call;
                saa.setReturnAddress();      // 保存恢复ra
                saa.setAllocatedPara((size_t)max(0, ((int)c.args.len - 8 ) * 4));    // all on stack
            }
            size_t sz = getTypeSize(value->ty);
            if(sz){
                saa.alloc(value, sz);
            }
        }
    }
    saa.getDelta();

    //  函数的 prologue
    if(saa.delta){
        if((-(int)saa.delta) >= -2048 && (-(int)saa.delta) < 2048){
            RiscvString += "  addi  sp, sp, " + std::to_string(-(int)saa.delta) + "\n";
        }else{
            RiscvString += "  li    t0, " + std::to_string(-(int)saa.delta) + "\n";
            RiscvString += "  add   sp, sp, t0\n";
        }
    }
    if(saa.ReturnAddress){
        if(((int)saa.delta - 4) >= -2048 && ((int)saa.delta - 4) < 2048)
            RiscvString += "  sw    ra, " + std::to_string((int)saa.delta - 4) + "(sp)\n";    
        else{
            RiscvString += "  li    t3, " + std::to_string((int)saa.delta - 4) + "\n";
            RiscvString += "  add   t3, t3, sp\n";
            RiscvString += "  sw    ra, 0(t3)\n";
        }
    }


    // 找到entry block
    size_t e = 0;
    for(e = 0; e < func->bbs.len; ++e){
        auto ptr = reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[e]);
        if(ptr->name && !strcmp(ptr->name, "%entry")){
            break;
        }
    }
    // 访问 entry block
    Visit(reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[e]));

    for(size_t i = 0; i < func->bbs.len; ++i){
        if(i == e) continue;
        auto ptr = func->bbs.buffer[i];
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
    }

    // 函数的 epilogue 在ret指令完成
    RiscvString += "\n\n";
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
    if(bb->name && strcmp(bb->name, "%entry")){
        RiscvString += string(bb->name + 1) + ":\n";
    }
    for(size_t i = 0; i < bb->insts.len; ++i){
        auto ptr = bb->insts.buffer[i];
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
    }
}

// 访问指令
void Visit(const koopa_raw_value_t &value) {
    // 根据指令类型判断后续需要如何访问
    const auto &kind = value->kind;

    switch (kind.tag) {
        case KOOPA_RVT_RETURN:
            // 访问 return 指令
            Visit(kind.data.ret);
            break;
        case KOOPA_RVT_INTEGER:
            // 访问 integer 指令
            RiscvString += "Control flow should never reach here.\n";
            Visit(kind.data.integer);
            break;
        case KOOPA_RVT_BINARY:
            // 访问二元运算
            Visit(kind.data.binary);
            if(saa.getOffset(value) >= -2048 && saa.getOffset(value) < 2048)
                RiscvString += "  sw    t0, " + std::to_string(saa.getOffset(value)) + "(sp)\n";    
            else{
                RiscvString += "  li    t3, " + std::to_string(saa.getOffset(value)) + "\n";
                RiscvString += "  add   t3, t3, sp\n";
                RiscvString += "  sw    t0, 0(t3)\n";
            }
            break;
        case KOOPA_RVT_ALLOC:
            // 访问栈分配指令，啥都不用管
            break;
        
        case KOOPA_RVT_LOAD:
            // 加载指令
            Visit(kind.data.load);
            if(saa.getOffset(value) >= -2048 && saa.getOffset(value) < 2048)
                RiscvString += "  sw    t0, " + std::to_string(saa.getOffset(value)) + "(sp)\n";    
            else{
                RiscvString += "  li    t3, " + std::to_string(saa.getOffset(value)) + "\n";
                RiscvString += "  add   t3, t3, sp\n";
                RiscvString += "  sw    t0, 0(t3)\n";
            }
            break;

        case KOOPA_RVT_STORE:
            // 存储指令
            Visit(kind.data.store);
            break;
        case KOOPA_RVT_BRANCH:
            // 条件分支指令
            Visit(kind.data.branch);
            break;
        case KOOPA_RVT_JUMP:
            // 无条件跳转指令
            Visit(kind.data.jump);
            break;
        case KOOPA_RVT_CALL:
            // 访问函数调用
            Visit(kind.data.call);
            if(kind.data.call.callee->ty->data.function.ret->tag == KOOPA_RTT_INT32){
                if(saa.getOffset(value) >= -2048 && saa.getOffset(value) < 2048)
                    RiscvString += "  sw    a0, " + std::to_string(saa.getOffset(value)) + "(sp)\n";    
                else{
                    RiscvString += "  li    t3, " + std::to_string(saa.getOffset(value)) + "\n";
                    RiscvString += "  add   t3, t3, sp\n";
                    RiscvString += "  sw    a0, 0(t3)\n";
                }
            }
            break;
        case KOOPA_RVT_GLOBAL_ALLOC:
            // 访问全局变量
            VisitGlobalVar(value);
            break;
        case KOOPA_RVT_GET_ELEM_PTR:
            // 访问getelemptr指令
            Visit(kind.data.get_elem_ptr);
            if(saa.getOffset(value) >= -2048 && saa.getOffset(value) < 2048)
                RiscvString += "  sw    t0, " + std::to_string(saa.getOffset(value)) + "(sp)\n";    
            else{
                RiscvString += "  li    t3, " + std::to_string(saa.getOffset(value)) + "\n";
                RiscvString += "  add   t3, t3, sp\n";
                RiscvString += "  sw    t0, 0(t3)\n";
            }
            break;
        case KOOPA_RVT_GET_PTR:
            Visit(kind.data.get_ptr);
            if(saa.getOffset(value) >= -2048 && saa.getOffset(value) < 2048)
                RiscvString += "  sw    t0, " + std::to_string(saa.getOffset(value)) + "(sp)\n";    
            else{
                RiscvString += "  li    t3, " + std::to_string(saa.getOffset(value)) + "\n";
                RiscvString += "  add   t3, t3, sp\n";
                RiscvString += "  sw    t0, 0(t3)\n";
            }
        default:
            // 其他类型暂时遇不到
            break;
    }
}

// 访问return指令
void Visit(const koopa_raw_return_t &value) {
    if(value.value != nullptr) {
        koopa_raw_value_t ret_value = value.value;
        // 特判return一个整数情况
        if(ret_value->kind.tag == KOOPA_RVT_INTEGER){
            int i = Visit(ret_value->kind.data.integer);
            RiscvString += "  li    a0, " + std::to_string(i) + "\n";
        } else{
            int i = saa.getOffset(ret_value);
            if(i >= -2048 && i < 2048)
                RiscvString += "  lw    a0, " + std::to_string(i) + "(sp)\n";    
            else{
                RiscvString += "  li    t3, " + std::to_string(i) + "\n";
                RiscvString += "  add   t3, t3, sp\n";
                RiscvString += "  lw    a0, 0(t3)\n";    
            }
        }
    }
    if(saa.ReturnAddress){
        if((saa.delta - 4) >= -2048 && (saa.delta - 4) < 2048)
            RiscvString += "  lw    ra, " + std::to_string(saa.delta - 4) + "(sp)\n";    
        else{
            RiscvString += "  li    t3, " + std::to_string(saa.delta - 4) + "\n";
            RiscvString += "  add   t3, t3, sp\n";
            RiscvString += "  lw    ra, 0(t3)\n";    
        }
    }
    if(saa.delta){
        if((int)saa.delta >= -2048 && (int)saa.delta < 2048){
            RiscvString += "  addi  sp, sp, " + std::to_string(saa.delta) + "\n";
        }else{
            RiscvString += "  li    t0, " + std::to_string(saa.delta) + "\n";
            RiscvString += "  add   sp, sp, t0\n";
        }
    }
    RiscvString += "  ret\n";
}

// 访问koopa_raw_integer_t,结果返回数值
int Visit(const koopa_raw_integer_t &value){
    return value.value;
}

// 访问koopa_raw_binary_t
void Visit(const koopa_raw_binary_t &value){

    // 把左右操作数加载到t0,t1寄存器
    koopa_raw_value_t l = value.lhs, r = value.rhs;
    int i;
    if(l->kind.tag == KOOPA_RVT_INTEGER){
        i = Visit(l->kind.data.integer);
        RiscvString += "  li    t0, " + std::to_string(i) + "\n";
    }else{
        i = saa.getOffset(l);
        if(i >= -2048 && i < 2048)
            RiscvString += "  lw    t0, " + std::to_string(i) + "(sp)\n";    
        else{
            RiscvString += "  li    t3, " + std::to_string(i) + "\n";
            RiscvString += "  add   t3, t3, sp\n";
            RiscvString += "  lw    t0, 0(t3)\n";    
        }
    }
    if(r->kind.tag == KOOPA_RVT_INTEGER){
        i = Visit(r->kind.data.integer);
        RiscvString += "  li    t1, " + std::to_string(i) + "\n";
    }else {
        i = saa.getOffset(r);
        if(i >= -2048 && i < 2048)
            RiscvString += "  lw    t1, " + std::to_string(i) + "(sp)\n";    
        else{
            RiscvString += "  li    t3, " + std::to_string(i) + "\n";
            RiscvString += "  add   t3, t3, sp\n";
            RiscvString += "  lw    t1, 0(t3)\n";    
        }
    }
    // 判断操作符
    switch (value.op) {
        case KOOPA_RBO_NOT_EQ:
            RiscvString += "  xor   t0, t0, t1\n";
            RiscvString += "  snez  t0, t0\n";
            break;
        case KOOPA_RBO_EQ:
            RiscvString += "  xor   t0, t0, t1\n";
            RiscvString += "  seqz  t0, t0\n";
            break;
        case KOOPA_RBO_GT:
            RiscvString += "  sgt   t0, t0, t1\n";
            break;
        case KOOPA_RBO_LT:
            RiscvString += "  slt   t0, t0, t1\n";
            break;
        case KOOPA_RBO_GE:
            RiscvString += "  slt   t0, t0, t1\n";
            RiscvString += "  seqz  t0, t0\n";
            break;
        case KOOPA_RBO_LE:
            RiscvString += "  sgt   t0, t0, t1\n";
            RiscvString += "  seqz  t0, t0\n";
            break;
        case KOOPA_RBO_ADD:
            RiscvString += "  add   t0, t0, t1\n";
            break;
        case KOOPA_RBO_SUB:
            RiscvString += "  sub   t0, t0, t1\n";
            break;
        case KOOPA_RBO_MUL:
            RiscvString += "  mul   t0, t0, t1\n";
            break;
        case KOOPA_RBO_DIV:
            RiscvString += "  div   t0, t0, t1\n";
            break;
        case KOOPA_RBO_MOD:
            RiscvString += "  rem   t0, t0, t1\n";
            break;
        case KOOPA_RBO_AND:
            RiscvString += "  and   t0, t0, t1\n";
            break;
        case KOOPA_RBO_OR:
            RiscvString += "  or    t0, t0, t1\n";
            break;
        case KOOPA_RBO_XOR:
            RiscvString += "  xor   t0, t0, t1\n";
            break;
        case KOOPA_RBO_SHL:
            RiscvString += "  sll   t0, t0, t1\n";
            break;
        case KOOPA_RBO_SHR:
            RiscvString += "  srl   t0, t0, t1\n";
            break;
        case KOOPA_RBO_SAR:
            RiscvString += "  sra   t0, t0, t1\n";
            break;
        default:
            // 我们暂时不会遇到其他内容, 于是不对其做任何处理
            assert(false);
    }
}

// 访问load指令
void Visit(const koopa_raw_load_t &load){
    koopa_raw_value_t src = load.src;

    if(src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC){
        // 全局变量
        RiscvString += "  la    t0, " + string(src->name + 1) + "\n";
        RiscvString += "  lw    t0, 0(t0)\n";
    } else if(src->kind.tag == KOOPA_RVT_ALLOC){
        // 栈分配
        int i = saa.getOffset(src);
        if(i >= -2048 && i < 2048)
            RiscvString += "  lw    t0, " + std::to_string(i) + "(sp)\n";    
        else{
            RiscvString += "  li    t3, " + std::to_string(i) + "\n";
            RiscvString += "  add   t3, t3, sp\n";
            RiscvString += "  lw    t0, 0(t3)\n";    
        }
    } else{
        if(saa.getOffset(src) >= -2048 && saa.getOffset(src) < 2048)
            RiscvString += "  lw    t0, " + std::to_string(saa.getOffset(src)) + "(sp)\n";    
        else{
            RiscvString += "  li    t3, " + std::to_string(saa.getOffset(src)) + "\n";
            RiscvString += "  add   t3, t3, sp\n";
            RiscvString += "  lw    t0, 0(t3)\n";    
        }
        RiscvString += "  lw    t0, 0(t0)\n";
    }
}

// 访问store指令
void Visit(const koopa_raw_store_t &store){
    koopa_raw_value_t v = store.value, d = store.dest;

    if(v->kind.tag == KOOPA_RVT_FUNC_ARG_REF){
        int i;
        for(i = 0; i < f->params.len; ++i){
            if(f->params.buffer[i] == (void *)v)
                break;
        }
        if(i < 8){
            string reg = "a" + to_string(i);
            if(d->kind.tag == KOOPA_RVT_GLOBAL_ALLOC){
                RiscvString += "  la    t0, " + string(d->name + 1) + "\n";
                RiscvString += "  sw    " + reg + ", 0(t0)\n";
            }else if(d->kind.tag == KOOPA_RVT_ALLOC){
                if(saa.getOffset(d) >= -2048 && saa.getOffset(d) < 2048)
                    RiscvString += "  sw    " + reg + ", " + std::to_string(saa.getOffset(d)) + "(sp)\n";    
                else{
                    RiscvString += "  li    t3, " + std::to_string(saa.getOffset(d)) + "\n";
                    RiscvString += "  add   t3, t3, sp\n";
                    RiscvString += "  sw    " + reg + ", 0(t3)\n";  
                }
            }else{
                // 间接引用
                if(saa.getOffset(d) >= -2048 && saa.getOffset(d) < 2048)
                    RiscvString += "  lw    t0, " + std::to_string(saa.getOffset(d)) + "(sp)\n";    
                else{
                    RiscvString += "  li    t3, " + std::to_string(saa.getOffset(d)) + "\n";
                    RiscvString += "  add   t3, t3, sp\n";
                    RiscvString += "  lw    t0, 0(t3)\n";    
                }
                RiscvString += "  sw    " + reg + ", 0(t0)\n";
            }
            return;
        } else{
            i = (i - 8) * 4;
            if((i + saa.delta) >= -2048 && (i + saa.delta) < 2048)
                RiscvString += "  lw    t0, " + std::to_string(i + saa.delta) + "(sp)\n";    
            else{
                RiscvString += "  li    t3, " + std::to_string(i + saa.delta) + "\n";
                RiscvString += "  add   t3, t3, sp\n";
                RiscvString += "  lw    t0, 0(t3)\n";    
            } // caller 栈帧中
        }
    }else if(v->kind.tag == KOOPA_RVT_INTEGER){
        RiscvString += "  li    t0, " + std::to_string(Visit(v->kind.data.integer)) + "\n";
    } else{
        if(saa.getOffset(v) >= -2048 && saa.getOffset(v) < 2048)
            RiscvString += "  lw    t0, " + std::to_string(saa.getOffset(v)) + "(sp)\n";    
        else{
            RiscvString += "  li    t3, " + std::to_string(saa.getOffset(v)) + "\n";
            RiscvString += "  add   t3, t3, sp\n";
            RiscvString += "  lw    t0, 0(t3)\n";    
        }
    }
    if(d->kind.tag == KOOPA_RVT_GLOBAL_ALLOC){
        RiscvString += "  la    t1, " + string(d->name + 1) + "\n";
        RiscvString += "  sw    t0, 0(t1)\n";
    } else if(d->kind.tag == KOOPA_RVT_ALLOC){
        if(saa.getOffset(d) >= -2048 && saa.getOffset(d) < 2048)
                RiscvString += "  sw    t0, " + std::to_string(saa.getOffset(d)) + "(sp)\n";    
            else{
                RiscvString += "  li    t3, " + std::to_string(saa.getOffset(d)) + "\n";
                RiscvString += "  add   t3, t3, sp\n";
                RiscvString += "  sw    t0, 0(t3)\n";
            }
    } else {
        if(saa.getOffset(d) >= -2048 && saa.getOffset(d) < 2048)
            RiscvString += "  lw    t1, " + std::to_string(saa.getOffset(d)) + "(sp)\n";    
        else{
            RiscvString += "  li    t3, " + std::to_string(saa.getOffset(d)) + "\n";
            RiscvString += "  add   t3, t3, sp\n";
            RiscvString += "  lw    t1, 0(t3)\n";    
        }
        RiscvString += "  sw    t0, 0(t1)\n";
    }
    
    return;
}

// 访问branch指令
void Visit(const koopa_raw_branch_t &branch){
    auto true_bb = branch.true_bb;
    auto false_bb = branch.false_bb;
    koopa_raw_value_t v = branch.cond;
    if(v->kind.tag == KOOPA_RVT_INTEGER){
        RiscvString += "  li    t0, " + std::to_string(Visit(v->kind.data.integer)) + "\n";
    }else{
        if(saa.getOffset(v) >= -2048 && saa.getOffset(v) < 2048)
            RiscvString += "  lw    t0, " + std::to_string(saa.getOffset(v)) + "(sp)\n";    
        else{
            RiscvString += "  li    t3, " + std::to_string(saa.getOffset(v)) + "\n";
            RiscvString += "  add   t3, t3, sp\n";
            RiscvString += "  lw    t0, 0(t3)\n";    
        }
    }
    // 这里，用条件跳转指令跳转范围只有4KB，过不了long_func测试用例1MB。
    // 因此只用bnez实现分支，然后用jump调到目的地。
    string tmp_label = "Label" + std::to_string(LabelCounter++);
    RiscvString += "  bnez  t0, " + tmp_label + "\n";
    RiscvString += "  j     " + string(false_bb->name + 1) + "\n";
    RiscvString += tmp_label + ":\n";
    RiscvString += "  j     " + string(true_bb->name + 1) + "\n";
    return;
}

// 访问jump指令
void Visit(const koopa_raw_jump_t &jump){
    auto name = string(jump.target->name + 1);
    RiscvString += "  j     " + name + "\n";
    return;
}

// 访问 call 指令
void Visit(const koopa_raw_call_t &call){
    for(int i = 0; i < call.args.len; ++i){
        koopa_raw_value_t v = (koopa_raw_value_t)call.args.buffer[i];
        if(v->kind.tag == KOOPA_RVT_INTEGER){
            int j = Visit(v->kind.data.integer);
            if(i < 8){
                RiscvString += "  li    a" + to_string(i) + ", " + std::to_string(j) + "\n";
            } else {
                RiscvString += "  li    t0, " + std::to_string(j) + "\n";
                if(((i - 8) * 4) >= -2048 && ((i - 8) * 4) < 2048)
                    RiscvString += "  sw    t0, " + std::to_string((i - 8) * 4) + "(sp)\n";    
                else{
                    RiscvString += "  li    t3, " + std::to_string((i - 8) * 4) + "\n";
                    RiscvString += "  add   t3, t3, sp\n";
                    RiscvString += "  sw    t0, 0(t3)\n";
                }
            }
        } else{
            int off = saa.getOffset(v);
            if(i < 8){
                if(off >= -2048 && off < 2048)
                    RiscvString += "  lw    a" + to_string(i) + ", " + std::to_string(off) + "(sp)\n";    
                else{
                    RiscvString += "  li    t3, " + std::to_string(off) + "\n";
                    RiscvString += "  add   t3, t3, sp\n";
                    RiscvString += "  lw    a" + to_string(i) + ", 0(t3)\n";    
                }
            } else {
                if(off >= -2048 && off < 2048)
                    RiscvString += "  lw    t0, " + std::to_string(off) + "(sp)\n";    
                else{
                    RiscvString += "  li    t3, " + std::to_string(off) + "\n";
                    RiscvString += "  add   t3, t3, sp\n";
                    RiscvString += "  lw    t0, 0(t3)\n";    
                }
                if(((i - 8) * 4) >= -2048 && ((i - 8) * 4) < 2048)
                    RiscvString += "  sw    t0, " + std::to_string((i - 8) * 4) + "(sp)\n";    
                else{
                    RiscvString += "  li    t3, " + std::to_string((i - 8) * 4) + "\n";
                    RiscvString += "  add   t3, t3, sp\n";
                    RiscvString += "  sw    t0, 0(t3)\n";
                }
            }
        }
    }
    RiscvString += "  call " + string(call.callee->name + 1) + "\n";
  
    return;
}

// 访问getelemptr指令
void Visit(const koopa_raw_get_elem_ptr_t& get_elem_ptr){
    // getelemptr @arr, %2
        // la t0, arr
        // li t1 %2
        // li t2 arr.size
        // mul t1 t1 t2
        // add t0 t0 t1
    koopa_raw_value_t src = get_elem_ptr.src, index = get_elem_ptr.index;
    size_t sz = getTypeSize(src->ty->data.pointer.base->data.array.base);
        
    // 将src的地址放到t0
    if(src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC){
        RiscvString += "  la    t0, " + string(src->name + 1) + "\n";
    }else if(src->kind.tag == KOOPA_RVT_FUNC_ARG_REF){
        // 我们的KoopaIR保证遇不到
    } else if(src->kind.tag == KOOPA_RVT_ALLOC){
        // 栈上就是要找的地址
        int offset = (int)saa.getOffset(src);
        if(-2048 <= offset && offset < 2048){
            RiscvString += "  addi  t0, sp, " + to_string(offset) + "\n";
        } else {
            RiscvString += "  li    t0, " + std::to_string(offset) + "\n";
            RiscvString += "  add   t0, sp, t0\n";
        }
    } else {
        // 栈上存的是指针，间接索引
        if(saa.getOffset(src) >= -2048 && saa.getOffset(src) < 2048)
            RiscvString += "  lw    t0, " + std::to_string(saa.getOffset(src)) + "(sp)\n";    
        else{
            RiscvString += "  li    t3, " + std::to_string(saa.getOffset(src)) + "\n";
            RiscvString += "  add   t3, t3, sp\n";
            RiscvString += "  lw    t0, 0(t3)\n";    
        }
    }
    // 将index放到t1
    if(index->kind.tag == KOOPA_RVT_INTEGER){
        int v = Visit(index->kind.data.integer);
        RiscvString += "  li    t1, " + std::to_string(v) + "\n";
    } else {
        // 其他情况就是栈上的临时变量
        if(saa.getOffset(index) >= -2048 && saa.getOffset(index) < 2048)
            RiscvString += "  lw    t1, " + std::to_string(saa.getOffset(index)) + "(sp)\n";    
        else{
            RiscvString += "  li    t3, " + std::to_string(saa.getOffset(index)) + "\n";
            RiscvString += "  add   t3, t3, sp\n";
            RiscvString += "  lw    t1, 0(t3)\n";    
        }
    }
    // 将size放到t2
    RiscvString += "  li    t2, " + std::to_string(sz) + "\n";
    // 计算真实地址 index * size + base
    RiscvString += "  mul   t1, t1, t2\n";
    RiscvString += "  add   t0, t0, t1\n";
}

// 访问getptr指令
void Visit(const koopa_raw_get_ptr_t& get_ptr){
    koopa_raw_value_t src = get_ptr.src, index = get_ptr.index;
    size_t sz = getTypeSize(src->ty->data.pointer.base);

    // 将src的地址放到t0
    if(src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC){
        RiscvString += "  la    t0, " + string(src->name + 1) + "\n";
    }else if(src->kind.tag == KOOPA_RVT_FUNC_ARG_REF){
        // 我们的KoopaIR保证遇不到
    } else if(src->kind.tag == KOOPA_RVT_ALLOC){
        // 栈上就是要找的地址
        int offset = (int)saa.getOffset(src);
        if(-2048 <= offset && offset < 2048){
            RiscvString += "  addi  t0, sp, " + to_string(offset) + "\n";
        } else {
            RiscvString += "  li    t0, " + std::to_string(offset) + "\n";
            RiscvString += "  add   t0, sp, t0\n";
        }
    } else {
        // 栈上存的是指针，间接索引
        if(saa.getOffset(src) >= -2048 && saa.getOffset(src) < 2048)
            RiscvString += "  lw    t0, " + std::to_string(saa.getOffset(src)) + "(sp)\n";    
        else{
            RiscvString += "  li    t3, " + std::to_string(saa.getOffset(src)) + "\n";
            RiscvString += "  add   t3, t3, sp\n";
            RiscvString += "  lw    t0, 0(t3)\n";    
        }
    }
    // 将index放到t1
    if(index->kind.tag == KOOPA_RVT_INTEGER){
        int v = Visit(index->kind.data.integer);
        RiscvString += "  li    t1, " + std::to_string(v) + "\n";
    } else {
        // 其他情况就是栈上的临时变量
        if(saa.getOffset(index) >= -2048 && saa.getOffset(index) < 2048)
            RiscvString += "  lw    t1, " + std::to_string(saa.getOffset(index)) + "(sp)\n";    
        else{
            RiscvString += "  li    t3, " + std::to_string(saa.getOffset(index)) + "\n";
            RiscvString += "  add   t3, t3, sp\n";
            RiscvString += "  lw    t1, 0(t3)\n";    
        }
    }
    // 将size放到t2
    RiscvString += "  li    t2, " + std::to_string(sz) + "\n";
    // 计算真实地址 index * size + base
    RiscvString += "  mul   t1, t1, t2\n";
    RiscvString += "  add   t0, t0, t1\n";
}

// 访问全局变量
void VisitGlobalVar(koopa_raw_value_t value){
    RiscvString += "  .data\n";
    RiscvString += "  .globl " + string(value->name + 1) + "\n";
    RiscvString += string(value->name + 1) + ":\n";
    koopa_raw_value_t init = value->kind.data.global_alloc.init;
    auto ty = value->ty->data.pointer.base;
    if(ty->tag == KOOPA_RTT_INT32){
        if(init->kind.tag == KOOPA_RVT_ZERO_INIT){
            RiscvString += "  .zero 4\n";
        } else {
            int i = Visit(init->kind.data.integer);
            RiscvString += "  .word " + std::to_string(i) + "\n";
        }
    } else{
        // see aggragate
        assert (init->kind.tag == KOOPA_RVT_AGGREGATE) ;
        initGlobalArray(init);
    }
    RiscvString += "\n";
    return ;
}

void initGlobalArray(koopa_raw_value_t init){
    if(init->kind.tag == KOOPA_RVT_INTEGER){
        int v = Visit(init->kind.data.integer);
        RiscvString += "  .word " + std::to_string(v) + "\n";
    } else {
        // KOOPA_RVT_AGGREGATE
        auto elems = init->kind.data.aggregate.elems;
        for(int i = 0; i < elems.len; ++i){
            initGlobalArray(reinterpret_cast<koopa_raw_value_t>(elems.buffer[i]));
        }
    }
}

// 计算类型koopa_raw_type_t的大小
size_t getTypeSize(koopa_raw_type_t ty){
    switch(ty->tag){
        case KOOPA_RTT_INT32:
            return 4;
        case KOOPA_RTT_UNIT:
            return 0;
        case KOOPA_RTT_ARRAY:
            return ty->data.array.len * getTypeSize(ty->data.array.base);
        case KOOPA_RTT_POINTER:
            return 4;
        case KOOPA_RTT_FUNCTION:
            return 0;
    }
    return 0;
}

// StackAddressAllocator类方法定义
StackAddressAllocator::StackAddressAllocator() {
    ReturnAddress = 0;
    AllocatedPara = 0;
    StackSpace = 0;
    delta = 0;
}
void StackAddressAllocator::clear() {
    var_addr.clear();
    ReturnAddress = AllocatedPara = StackSpace = delta = 0;
}
void StackAddressAllocator::alloc(koopa_raw_value_t value, size_t width) {
    var_addr[value] = StackSpace;
    StackSpace += width;
}
void StackAddressAllocator::setReturnAddress() {
    ReturnAddress = 4;
}
void StackAddressAllocator::setAllocatedPara(size_t a) {
    AllocatedPara = std::max(AllocatedPara, a);
}
bool StackAddressAllocator::exists(koopa_raw_value_t value) {
    return var_addr.count(value) > 0;
}
size_t StackAddressAllocator::getOffset(koopa_raw_value_t value) {
    return var_addr[value] + AllocatedPara;
}
size_t StackAddressAllocator::getDelta() {
    size_t d = StackSpace + ReturnAddress + AllocatedPara;
    delta = d % 16 ? d + 16 - d % 16 : d;
    return delta;
}