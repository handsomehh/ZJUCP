#include "AST.h"
#include "SysTable.h"
#include <memory>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include "tools.h"
#include <string.h>

using namespace std;

/*
    生成的koopa IR字符串
*/
KoopaString ks;

/*
    符号表栈
*/
SymbolTableStack symbol_tb_stack;

/* ctrl: 严格来说，叫 is_need_to_add_jump_flag
   这玩意的功能在于，判断当前的IR代码块是否已经有跳转指令结尾
   如果不是以跳转指令结尾，我们要手动加一个jump语句
   用在if、while语句中
   例如： if (a > 1) { return 1 } else {return 0}
         那么在koopa IR的%then和%else语句块后面我们就不会增加 jump %end 指令，因为已经有 ret 作为跳转指令了
   在具体的实现中，每进入一个新的语句块（即一个新的label），我们就将ctrl置为1；当该语句块中出现ret、break等指令时，将其置0
*/
bool ctrl;

While_Stack while_stack;

void CompUnitAST::Dump()const {
    std::cout << "CompUnitAST { ";
    ks.func_decl();
    symbol_tb_stack.func_decl_init();

    // 定义全局变量
    for (auto &i : decls)
    {
        i->Dump_Global();
    }

    // 定义函数和局部变量
    for (auto &i : func_defs)
    {
        i->Dump();
    }

    std::cout << " }";
    // ks.declLibFunc();
}

void DeclAST::Dump() const
{
    std::cout << "DeclAST { ";
    // func_def->Dump();
    if (tag == DeclAST::CONST)
    {
        const_decl->Dump();
    }
    else if (tag == DeclAST::VAR)
    {
        var_decl->Dump();
    }
    std::cout << " }";
    // ks.declLibFunc();
}

void DeclAST::Dump_Global() const
{
    std::cout << "DeclAST { ";
    // func_def->Dump();
    if (tag == DeclAST::CONST)
    {
        const_decl->Dump_Global();
    }
    else if (tag == DeclAST::VAR)
    {
        var_decl->Dump_Global();
    }
    std::cout << " }";
    // ks.declLibFunc();
}

void ConstDeclAST::Dump() const
{
    std::cout << "ConstDeclAST { ";
    // func_def->Dump();
    for (auto &i : const_defs)
    {
        i->Dump();
    }
    std::cout << " }";
    // ks.declLibFunc();
}

void ConstDeclAST::Dump_Global() const
{
    std::cout << "ConstDeclAST { ";
    // func_def->Dump();
    for (auto &i : const_defs)
    {
        i->Dump_Global();
    }
    std::cout << " }";
    // ks.declLibFunc();
}

/**
 * 完成对Local数组初始化的IR生成
 * 其中 name: 数组在Koopa IR中的名字  ptr: 指向数组的内容，例如{"1", "%2"}  len: 描述数组类型，i.e. 各个维度的长
*/
void array_init(std::string name, std::string *ptr, const std::vector<int> &arr_size){    
    int n = arr_size[0];
    if(arr_size.size() == 1){
        for(int i = 0; i < n; ++i){
            string tmp = symbol_tb_stack.Get_count();
            ks.getelemptr(tmp, name, i);
            ks.appendaddtab("store " + ptr[i] + ", " + tmp + '\n');
        }

        return;
    } 

    vector<int> sublen(arr_size.begin() + 1, arr_size.end());
    int width = 1;
    for(auto l : sublen)  width *= l;
    for(int i = 0; i < n; ++i){
        string tmp = symbol_tb_stack.Get_count();
        ks.getelemptr(tmp, name, i);
        int offset = i * width;
        array_init(tmp, ptr + offset, sublen);
    }
    
}

void ConstDefAST::Dump() const
{
    std::cout << "ConstDefAST { def:";
    // 这个判断条件是为了保证在当前作用域，该常量只能被定义一次
    if (symbol_tb_stack.is_exist(ident) == symbol_tb_stack.size())
    {
        std::cout << "redefined const val: " << ident;
        std::cout << "}" ;
        return;
    }
    switch(tag){
        case ConstDefAST::SINGLE:
            {
                std::cout << "CONST " << ident;
                int value = const_init_val->Dump();
                std::string ir_name = symbol_tb_stack.Get_var_name(ident);
                symbol_tb_stack.insert(ident, ir_name, value, SymbolTable::CONST);
            }
            break;
        
        case ConstDefAST::ARRAY:
            {
                //[NEED MODIFY]
                std::cout << "CONST ARRAY " << ident;
                
                std::vector<int> arr_size; // a[5][4][3] <=> arr_size = {5, 4, 3}
                for(auto &i : const_exp_list){
                    arr_size.push_back(i->Get_value());
                    std::cout << '[' << i->Get_value() << "]";
                }
                
                std::string ir_name = symbol_tb_stack.Get_var_name(ident);
                symbol_tb_stack.insertArray(ident, ir_name, arr_size, SymbolTable::CONST_ARRAY);

                string arr_type = ks.getArrType(arr_size);

                int tot_len = 1;
                for(auto i : arr_size) 
                    tot_len *= i;

                string *init = new string[tot_len];
                for(int i = 0; i < tot_len; ++i)
                    init[i] = "0";
                
                const_init_val->getInitVal(init, arr_size);
                
                // Local Const Array IR
                ks.appendaddtab(ir_name + " = alloc " + arr_type + '\n');;
                array_init(ir_name, init, arr_size);
            }
            break;
    }

    std::cout << " }";

    
}

void ConstDefAST::Dump_Global() const
{
    std::cout << "[Global]ConstDefAST { def:" << ident;
    // func_def->Dump();
    // 这个判断条件是为了保证在当前作用域，该常量只能被定义一次
    if (symbol_tb_stack.is_exist_global(ident))
    {
        std::cout << "Global redefined const val: " << ident;
        std::cout << " }";
        return;
    }

    std::string ir_name = symbol_tb_stack.Get_var_name(ident);
    switch(tag){
        case ConstDefAST::SINGLE :
            {
                int value = const_init_val->Dump();
                symbol_tb_stack.insert_global(ident, ir_name, value, SymbolTable::CONST);
            }
            break;
        case ConstDefAST::ARRAY :
            {
                std::cout << "CONST ARRAY " << ident;
                
                std::vector<int> arr_size; // a[5][4][3] <=> arr_size = {5, 4, 3}
                for(auto &i : const_exp_list){
                    arr_size.push_back(i->Get_value());
                    std::cout << '[' << i->Get_value() << "]";
                }
                
                std::string ir_name = symbol_tb_stack.Get_var_name(ident);
                symbol_tb_stack.insertArray_global(ident, ir_name, arr_size, SymbolTable::CONST_ARRAY);

                string arr_type = ks.getArrType(arr_size);

                int total = 1;
                for(auto i : arr_size) 
                    total = total * i;

                string *init = new string[total];
                for(int i = 0; i < total; ++i)
                    init[i] = "0";
                
                const_init_val->getInitVal(init, arr_size);
                
                // Global Const Array IR
                std::string init_list = ks.getInitList(init, arr_size);
                ks.append("global " + ir_name + " = alloc " + arr_type + ", " + init_list + "\n");
                
            }
            break;
    }

    std::cout << " }";
   
}

int ConstInitValAST::Dump() const
{
    // ConstInitValAST::Dump函数，对于单一变量，返回值就是const的值；而对于数组的情况，
    int const_val = 0;
    switch (tag){
        case ConstInitValAST::SINGLE:
            const_val = const_exp->Get_value(); 
            break;
        case ConstInitValAST::ARRAY:
            // 数组的初始化放在函数 ConstInitValAST::getInitVal 中进行
            break;
    }

    return const_val;
    
}

// 对ptr指向的区域初始化，所指区域的数组类型由len规定
void ConstInitValAST::getInitVal(std::string *ptr, const std::vector<int> &len) const{
    int n = len.size();
    vector<int> width(n);
    width[n - 1] = len[n - 1];
    for(int i = n - 2; i >= 0; --i){
        width[i] = width[i + 1] * len[i];
    }
    int i = 0;  // 指向下一步要填写的内存位置
    for(auto &init_val : const_exp_list){
        if(init_val->tag == SINGLE){
            ptr[i++] = to_string(init_val->const_exp->Get_value());
        } else {
            assert(n > 1);  // 对一维数组初始化不可能再套一个Aggregate{{}}
            int j = n - 1;
            if(i == 0){
                j = 1;
            } else{
                j = n - 1;
                for(; j >= 0; --j){
                    if(i % width[j] != 0)
                        break;
                }
                assert(j < n - 1); // 保证整除最后一维
                ++j;    // j 指向最大的可除的维度
            }
            init_val->getInitVal(
                ptr + i, 
                vector<int>(len.begin() + j, len.end())
                );
            i += width[j];
        }
        if(i >= width[0]) break;
    }
}

void VarDeclAST::Dump() const
{
    std::cout << "VarDeclAST {";
    for (auto &i : var_defs)
    {
        i->Dump();
    }
    std::cout << "}";
    // return const_exp-Dump();
}
void VarDeclAST::Dump_Global() const
{
    std::cout << "VarDeclAST {";
    for (auto &i : var_defs)
    {
        i->Dump_Global();
    }
    std::cout << "}";
    // return const_exp-Dump();
}
void VarDefAST::Dump() const
{
    std::cout << "VaeDefAST { def:" << ident;

    // 这个判断条件是为了保证在当前作用域，该变量只能被定义一次
    if (symbol_tb_stack.is_exist(ident) == symbol_tb_stack.size())
    {
        std::cout << "Var " << ident << " has been defined" << endl;
        std::cout << " }";
        return;
    }

    std::string ir_name = symbol_tb_stack.Get_var_name(ident);

    switch(tag){
        case ConstDefAST::SINGLE :
            {
                ks.appendaddtab(ir_name + ks.alloc32i);
                symbol_tb_stack.insert(ident, ir_name, SymbolTable::INT);
                if (init_val)
                {
                    std::string res = init_val->exp->Dump();
                    ks.appendaddtab("store " + res + ", " + ir_name + '\n');
                }
            }
            break;
        case ConstDefAST::ARRAY :
            {
                std::cout << "ARRAY " << ident ;
                
                std::vector<int> arr_size; // a[5][4][3] <=> arr_size = {5, 4, 3}
                for(auto &i : const_exp_list){
                    arr_size.push_back(i->Get_value());
                    std::cout << '[' << i->Get_value() << "]";
                }
    
                symbol_tb_stack.insertArray(ident, ir_name, arr_size, SymbolTable::ARRAY);

                string arr_type = ks.getArrType(arr_size);

                int total = 1;
                for(auto i : arr_size) 
                    total = total * i;

                string *init = new string[total];
                for(int i = 0; i < total; ++i)
                    init[i] = "0";
                
                // Local Array IR
                ks.appendaddtab(ir_name + " = alloc " + arr_type + "\n");
                if (init_val){
                    init_val->getInitVal(init, arr_size, false);
                    array_init(ir_name, init, arr_size);
                }
            }
            break;
    }

    std::cout << " }";
}

void InitValAST::getInitVal(std::string *ptr, const std::vector<int> &len, bool is_global) const{
    // [NEED MODIFY]
    int n = len.size();
    vector<int> width(n);
    width[n - 1] = len[n - 1];
    for(int i = n - 2; i >= 0; --i){
        width[i] = width[i + 1] * len[i];
    }


    int i = 0;  // 指向下一步要填写的内存位置
    for(auto &init_val : exp_list) {
        if(init_val->tag == InitValAST::SINGLE) {
            if(is_global){
                ptr[i++] = to_string(init_val->exp->Get_value());
            } else{
                ptr[i++] = init_val->exp->Dump();
            }
        } else {
            assert(n > 1);  // 对一维数组初始化不可能再套一个Aggregate{{}}
            int j = n - 1;
            if(i == 0){
                j = 1;
            } else{
                j = n - 1;
                for(; j >= 0; --j){
                    if(i % width[j] != 0)
                        break;
                }
                assert(j < n - 1); // 保证整除最后一维
                ++j;    // j 指向最大的可除的维度
            }
            init_val->getInitVal(ptr + i, vector<int>(len.begin() + j, len.end()));
            i += width[j];
        }
        if(i >= width[0]) break;
    }
}

void VarDefAST::Dump_Global() const
{
    std::cout << "VaeDefAST { def:" << ident;

    // 这个判断条件是为了保证全局变量不重复
    if (symbol_tb_stack.is_exist_global(ident))
    {
        std::cout << "Global Var " << ident << " has been defined" << endl;
        return;
    }
    std::string ir_name = symbol_tb_stack.Get_var_name(ident);

    switch(tag){
        case VarDefAST::SINGLE :
            {
                symbol_tb_stack.insert_global(ident, ir_name, SymbolTable::INT);
                if (init_val)
                {
                    std::string res = std::to_string(init_val->exp->Get_value());
                    ks.append("global " + ir_name + ks.alloc32i_g + res + "\n");
                }
                else
                {
                    ks.append("global " + ir_name + ks.alloc32i_g + ks.zero + "\n");
                }
            }
            break;
        case VarDefAST::ARRAY :
            {
                std::cout << "ARRAY " << ident ;
                
                std::vector<int> arr_size; // a[5][4][3] <=> arr_size = {5, 4, 3}
                for(auto &i : const_exp_list){
                    arr_size.push_back(i->Get_value());
                    std::cout << '[' << i->Get_value() << "]";
                }
    
                symbol_tb_stack.insertArray(ident, ir_name, arr_size, SymbolTable::ARRAY);

                string arr_type = ks.getArrType(arr_size);

                int tot_len = 1;
                for(auto i : arr_size) 
                    tot_len *= i;

                string *init = new string[tot_len];
                for(int i = 0; i < tot_len; ++i)
                    init[i] = "0";
            
                // Global Const Array IR
                if (init_val) 
                    init_val->getInitVal(init, arr_size, true);

                std::string init_list = ks.getInitList(init, arr_size);
                ks.append("global " + ir_name + " = alloc " + arr_type + ", " + init_list + "\n");
               
            }
            break;
    }
    
    cout << " }";
}
int InitValAST::Get_value()
{
    return exp->Get_value();
    // return const_exp-Dump();
}
void FuncDefAST::Dump() const
{
    // 每个函数内部重置临时变量计数器（不知道有没有必要 似乎没有，但问题不大）
    // func1(){%1, %2 ...}
    // func2(){%1, %2 ...}
    symbol_tb_stack.Reset_count();
    symbol_tb_stack.alloc();

    std::cout << "FuncDefAST { ";
    if (func_type->tag == BTypeAST::INT)
    {
        symbol_tb_stack.insert_fun_name(ident, FunctionTable::INT);
    }
    else
    {
        symbol_tb_stack.insert_fun_name(ident, FunctionTable::VOID);
    }

    ks.append("fun @" + ident + "(");
    if (params)
    {
        params->Dump();
    }
    ks.append(")");
    if (func_type->tag == BTypeAST::INT)
    {
        ks.append(": i32");
    }
    ks.append(" {\n");
    ks.label("%entry");

    ctrl = true;
    if (params)
    {
        for (auto &i : params->params)
        {
            if (i->btype->tag == BTypeAST::INT)
            {
                ks.appendaddtab(symbol_tb_stack.Get_ir_name(i->ident) + ks.alloc32i);
                ks.appendaddtab("store %" + symbol_tb_stack.Get_ir_name(i->ident).substr(1) + "," + symbol_tb_stack.Get_ir_name(i->ident) + "\n");
            }
        }
    }
    // func_type->Dump();
    std::cout << ", " << ident << ", ";
    std::string res = block->Dump();
    std::cout << " }";
    if (func_type->tag == BTypeAST::VOID)
    {
        ks.ret("");
    }
    ks.append("}\n\n");
    symbol_tb_stack.quit();
}
void FuncFParamsAST::Dump() const
{

    int flag = 0;
    for (auto &i : params)
    {
        std::string res = i->Dump();
        if (flag == 0)
        {
            flag = 1;
            ks.append(res);
        }
        else
        {
            ks.append("," + res);
        }
    }
}
std::string FuncFParamAST::Dump() const
{
    if (btype->tag == BTypeAST::INT)
    {
        if (symbol_tb_stack.is_exist(ident) == symbol_tb_stack.size())
        {
            cout << "Func parameters " << ident << " has been defined" << endl;
            return "";
        }
        std::string ir_name = symbol_tb_stack.Get_var_name(ident);
        std::string xingcan = "%" + ir_name.substr(1);
        symbol_tb_stack.insert(ident, ir_name, SymbolTable::INT);
        std::string res = xingcan + ": " + "i32";
        return res;
    }
    cout << "no supported parameter function" << endl;
    return "";
}

void FuncTypeAST::Dump() const
{
    std::cout << "FuncTypeAST { ";
    // func_def->Dump();
    if (tag == FuncTypeAST::INT)
    {
        std::cout << "int";
    }
    std::cout << " }";
}

std::string BlockAST::Dump() const
{
    std::cout << "BlockAST { ";
    // func_def->Dump();

    symbol_tb_stack.alloc(); // 每进入一个新的block，新建一张symbol table

    std::string res;
    for (auto &i : blockitem)
    {
        res = i->Dump();
        // 在一个block中，如果出现了Return语句，那么该block后面的代码均可忽略，即该block可以提前结束
        if(i->tag == BlockItemAST::STMT){
            if(i->stmt->tag == StmtAST::RETURN || i->stmt->tag == StmtAST::BREAK || i->stmt->tag == StmtAST::CONTINUE){
                symbol_tb_stack.quit();
                return res;
            }
        }
    }
    std::cout << " }";

    symbol_tb_stack.quit(); // 每离开一个block，删除symbol table
    return res;
}

std::string BlockItemAST::Dump() const
{
    std::cout << "BlockItemAST { ";
    // func_def->Dump();
    std::string res = "-1";
    if (tag == BlockItemAST::DECL)
    {
        decl->Dump();
    }
    else if (tag == BlockItemAST::STMT)
    {
        res = stmt->Dump();
    }
    std::cout << " }; ";
    return res;
}

std::string StmtAST::Dump() const
{
    std::cout << "StmtAST { ";
    // func_def->Dump();
    std::string res = "-1";
    if (tag == StmtAST::RETURN)
    {
        std::cout << "RETURN ";
        res = exp->Dump();
        ks.ret(res);

        ctrl = false;
    }
    else if (tag == StmtAST::ASSIGN)
    {
        std::string res = exp->Dump(); // 右表达式结果
        std::string to = lval->ident;  // 目标变量
        std::cout << "ASSIGN";
        int tb_id = symbol_tb_stack.is_exist(to);
        if (tb_id > 0) // 符号表栈中存在该变量名
        {
            std::string ir_name = symbol_tb_stack.Get_ir_name(to);
            if (symbol_tb_stack.Get_type(to) == Symbol::INT){
                ks.appendaddtab("store " + res + ", " + ir_name + '\n');
            }
            else if (symbol_tb_stack.Get_type(to) == Symbol::ARRAY){
                std::string tmp = lval->Dump();
                ks.appendaddtab("store " + res + ", " + tmp + '\n');
            }
            else if (symbol_tb_stack.Get_type(to) == Symbol::CONST || 
                    symbol_tb_stack.Get_type(to) == Symbol::CONST_ARRAY ){
                std::cout << " assign to const " << endl;
            }
        }
        else
        {
            std::cout << "assgin to a undeclear var: " << to;
        }
    }
    else if (tag == StmtAST::EXP)
    {
        if (exp)
        {
            std::cout << "Expression_NULL";
            res = exp->Dump();
        }
    }
    else if (tag == StmtAST::BLOCK)
    {
        res = block->Dump();
    }
    else if (tag == StmtAST::IF)
    {
        std::cout << "IF (";
        std::string s = exp->Dump();
        std::cout << ") ";
        std::string if_label = symbol_tb_stack.Get_label_name(ks.if_label);
        std::string else_label = symbol_tb_stack.Get_label_name(ks.else_label);
        std::string end_label = symbol_tb_stack.Get_label_name(ks.end_label);

        if (else_stmt)
            ks.appendaddtab("br " + s + ", " + if_label + ", " + else_label + '\n');
        else
            ks.appendaddtab("br " + s + ", " + if_label + ", " + end_label + '\n');

        ctrl = true;
        ks.label(if_label);
        res = if_stmt->Dump();
        if (ctrl)
            ks.appendaddtab("jump " + end_label + '\n');

        if (else_stmt)
        {
            std::cout << " ELSE ";
            ctrl = true;
            ks.label(else_label);
            res = else_stmt->Dump();
            if (ctrl)
                ks.appendaddtab("jump " + end_label + '\n');
        }

        ctrl = true;
        ks.label(end_label);
    }
    else if (tag == StmtAST::WHILE)
    {
        std::string while_entry_label = symbol_tb_stack.Get_label_name(ks.while_entry_label);
        std::string while_body_label = symbol_tb_stack.Get_label_name(ks.while_body_label);
        std::string end_label = symbol_tb_stack.Get_label_name(ks.end_label);

        std::unique_ptr<While_Struct> while_stuct = 
            std::make_unique<While_Struct>(While_Struct(while_entry_label, while_body_label, end_label));

        while_stack.insert(while_stuct);

        ks.appendaddtab("jump " + while_entry_label + '\n');
        ks.label(while_entry_label);

        ctrl = true;
        std::cout << "WHILE (";
        std::string s = exp->Dump();
        ks.appendaddtab("br " + s + ", " + while_body_label + ", " + end_label + '\n');
        std::cout << ") {";

        ks.label(while_body_label);
        if (while_stmt)
        {
            ctrl = true;
            while_stmt->Dump();
            if (ctrl)
                ks.appendaddtab("jump " + while_entry_label + '\n');
        }
        else
        {
            std::cout << "while_stmt = null" << endl;
        }

        ctrl = true;
        ks.label(end_label);

        while_stack.pop();
        std::cout << "}";
    }
    else if (tag == StmtAST::BREAK){
        std::string end_name = while_stack.Get_end_name();
        ks.appendaddtab("jump " + end_name + '\n');
        ctrl = false;
    }
    else if (tag == StmtAST::CONTINUE){
        std::string entry_name = while_stack.Get_entry_name();
        ks.appendaddtab("jump " + entry_name + '\n');
        ctrl = false;
    }

    // exp->Dump();
    std::cout << " }";
    return res;
}

std::string ExpAST::Dump() const
{
    std::cout << "ExpAST { ";
    // func_def->Dump();
    std::string res = l_or_exp->Dump();
    std::cout << " }";
    return res;
}

int ExpAST::Get_value()
{

    return l_or_exp->Get_value();
}
int LOrExpAST::Get_value()
{
    if (tag == LOrExpAST::AND)
    {

        return l_and_exp->Get_value();
    }
    else if (tag == LOrExpAST::OR_AND)
    {

        return l_or_exp2->Get_value() || l_and_exp2->Get_value();
    }
}
std::string LOrExpAST::Dump() const
{
    std::cout << "LOrExpAST { ";
    // func_def->Dump();
    // l_or_exp->Dump();
    if (tag == LOrExpAST::AND)
    {
        return l_and_exp->Dump();
    }
    else if (tag == LOrExpAST::OR_AND)
    {
        std::string src1 = l_or_exp2->Dump();
        std::string src2 = l_and_exp2->Dump();

        std::string res1 = symbol_tb_stack.Get_count();
        ks.logic(res1, src1, "0", "ne");
        std::string res2 = symbol_tb_stack.Get_count();
        ks.logic(res2, src2, "0", "ne");
        std::string res3 = symbol_tb_stack.Get_count();
        ks.logic(res3, res1, res2, "or");
        return res3;
    }
    std::cout << " }";
}

int LAndExpAST::Get_value()
{
    if (tag == LAndExpAST::EQ)
    {

        return eq_exp->Get_value();
    }
    else if (tag == LAndExpAST::EQ_AND)
    {

        return l_and_exp2->Get_value() && eq_exp2->Get_value();
    }
}

std::string LAndExpAST::Dump() const
{
    std::cout << "LAndExpAST { ";
    // func_def->Dump();
    // l_or_exp->Dump();
    if (tag == LAndExpAST::EQ)
    {
        return eq_exp->Dump();
    }
    else if (tag == LAndExpAST::EQ_AND)
    {
        std::string src1 = l_and_exp2->Dump();
        std::string src2 = eq_exp2->Dump();

        std::string res1 = symbol_tb_stack.Get_count();
        ks.logic(res1, src1, "0", "ne");
        std::string res2 = symbol_tb_stack.Get_count();
        ks.logic(res2, src2, "0", "ne");
        std::string res3 = symbol_tb_stack.Get_count();
        ks.logic(res3, res1, res2, "and");
        return res3;
    }
    std::cout << " }";
}

int EqExpAST::Get_value()
{
    if (tag == EqExpAST::REL)
    {

        return rel_exp->Get_value();
    }
    else if (tag == EqExpAST::EQ_REL)
    {
        if (op == '=')
        {
            return eq_exp2->Get_value() == rel_exp2->Get_value();
        }
        else if (op == '!')
        {
            return eq_exp2->Get_value() != rel_exp2->Get_value();
        }
    }
}

std::string EqExpAST::Dump() const
{
    std::cout << "EqExpAST { ";
    // func_def->Dump();
    // l_or_exp->Dump();
    if (tag == EqExpAST::REL)
    {
        return rel_exp->Dump();
    }
    else if (tag == EqExpAST::EQ_REL)
    {
        std::string src1 = eq_exp2->Dump();
        std::string src2 = rel_exp2->Dump();

        std::string res = symbol_tb_stack.Get_count();
        if (op == '=')
        {
            ks.logic(res, src1, src2, "eq");
        }
        else
        {
            ks.logic(res, src1, src2, "ne");
        }
        return res;
    }
    std::cout << " }";
}

int RelExpAST::Get_value()
{
    if (tag == RelExpAST::ADD)
    {

        return add_exp->Get_value();
    }
    else if (tag == RelExpAST::REL_ADD)
    {
        if (!strcmp(op, "<"))
        {
            return rel_exp2->Get_value() < add_exp2->Get_value();
        }
        else if (!strcmp(op, ">"))
        {
            return rel_exp2->Get_value() > add_exp2->Get_value();
        }
        else if (!strcmp(op, "<="))
        {
            return rel_exp2->Get_value() <= add_exp2->Get_value();
        }
        else if (!strcmp(op, ">="))
        {
            return rel_exp2->Get_value() >= add_exp2->Get_value();
        }
    }
}
std::string RelExpAST::Dump() const
{
    std::cout << "RelExpAST { ";
    // func_def->Dump();
    // l_or_exp->Dump();
    if (tag == RelExpAST::ADD)
    {
        return add_exp->Dump();
    }
    else if (tag == RelExpAST::REL_ADD)
    {
        std::string src1 = rel_exp2->Dump();
        std::string src2 = add_exp2->Dump();

        std::string res = symbol_tb_stack.Get_count();
        if (!strcmp(op, "<"))
        {
            ks.logic(res, src1, src2, "lt");
        }
        else if (!strcmp(op, ">"))
        {
            ks.logic(res, src1, src2, "gt");
        }
        else if (!strcmp(op, "<="))
        {
            ks.logic(res, src1, src2, "le");
        }
        else if (!strcmp(op, ">="))
        {
            ks.logic(res, src1, src2, "ge");
        }
        return res;
    }
    std::cout << " }";
}

int AddExpAST::Get_value()
{
    if (tag == AddExpAST::MUL)
    {

        return mul_exp->Get_value();
    }
    else if (tag == AddExpAST::ADD_MUL)
    {
        if (op == '+')
        {
            return add_exp2->Get_value() + mul_exp2->Get_value();
        }
        else if (op == '-')
        {
            return add_exp2->Get_value() - mul_exp2->Get_value();
        }
    }
}

std::string AddExpAST::Dump() const
{
    std::cout << "AddExpAST { ";
    // func_def->Dump();
    // l_or_exp->Dump();
    if (tag == AddExpAST::MUL)
    {
        return mul_exp->Dump();
    }
    else if (tag == AddExpAST::ADD_MUL)
    {
        std::string src1 = add_exp2->Dump();
        std::string src2 = mul_exp2->Dump();

        std::string res = symbol_tb_stack.Get_count();
        if (op == '+')
        {
            ks.logic(res, src1, src2, "add");
        }
        else if (op == '-')
        {
            ks.logic(res, src1, src2, "sub");
        }
        return res;
    }
    std::cout << " }";
}
int MulExpAST::Get_value()
{
    if (tag == MulExpAST::UNARY)
    {

        return unary_exp->Get_value();
    }
    else if (tag == MulExpAST::MUL_UNARY)
    {
        if (op == '*')
        {
            return mul_exp2->Get_value() * unary_exp2->Get_value();
        }
        else if (op == '/')
        {
            return mul_exp2->Get_value() / unary_exp2->Get_value();
        }
        else if (op == '%')
        {
            return mul_exp2->Get_value() % unary_exp2->Get_value();
        }
    }
}
std::string MulExpAST::Dump() const
{
    std::cout << "MulExpAST { ";
    // func_def->Dump();
    // l_or_exp->Dump();
    if (tag == MulExpAST::UNARY)
    {
        return unary_exp->Dump();
    }
    else if (tag == MulExpAST::MUL_UNARY)
    {
        std::string src1 = mul_exp2->Dump();
        std::string src2 = unary_exp2->Dump();

        std::string res = symbol_tb_stack.Get_count();

        if (op == '*')
        {
            ks.logic(res, src1, src2, "mul");
        }
        else if (op == '/')
        {
            ks.logic(res, src1, src2, "div");
        }
        else if (op == '%')
        {
            ks.logic(res, src1, src2, "mod");
        }
        return res;
    }
}
int UnaryExpAST::Get_value()
{
    if (tag == UnaryExpAST::PRIMARY)
    {
        return primary_exp->Get_value();
    }
    else if (tag == UnaryExpAST::UNARY)
    {
        if (op == '+')
        {
            return unary_exp->Get_value();
        }
        else if (op == '-')
        {
            return 0 - unary_exp->Get_value();
        }
        else if (op == '!')
        {
            return (!unary_exp->Get_value());
        }
    }
}
std::string UnaryExpAST::Dump() const
{
    std::cout << "UnaryExpAST { ";
    // func_def->Dump();
    // l_or_exp->Dump();
    if (tag == UnaryExpAST::PRIMARY)
    {
        return primary_exp->Dump();
    }
    else if (tag == UnaryExpAST::UNARY)
    {
        std::string src1 = unary_exp->Dump();
        if (op == '+')
        {
            return src1;
        }
        else if (op == '-')
        {
            std::string res = symbol_tb_stack.Get_count();
            ks.logic(res, "0", src1, "sub");
            return res;
        }
        else if (op == '!')
        {
            std::string res = symbol_tb_stack.Get_count();
            ks.logic(res, "0", src1, "eq");
            return res;
        }
    }
    else if (tag == UnaryExpAST::FUN)
    {
        if (symbol_tb_stack.Get_fun_type(ident) == FunctionTable::INT)
        {
            std::string para = "";
            if (func_params)
            {
                para = func_params->Dump();
            }
            std::string res = symbol_tb_stack.Get_count();
            ks.appendaddtab(res + " = call @" + ident + "(" + para + ")\n");
            return res;
        }
        else if (symbol_tb_stack.Get_fun_type(ident) == FunctionTable::VOID)
        {
            std::string para = "";
            if (func_params)
            {
                para = func_params->Dump();
            }
            ks.appendaddtab("call @" + ident + "(" + para + ")\n");
            return "";
        }
    }
}

std::string FuncRParamsAST::Dump() const
{
    std::string res = "";
    int flag = 0;
    for (auto &i : exps)
    {
        if (flag == 0)
        {
            res += i->Dump();
            flag = 1;
        }
        else
        {
            res += "," + i->Dump();
        }
    }
    return res;
}

std::string PrimaryExpAST::Dump() const
{
    std::cout << "PrimaryExpAST { ";
    if (tag == PrimaryExpAST::EXP)
    {
        return exp->Dump();
    }
    else if (tag == PrimaryExpAST::NUMBER)
    {
        std::cout << "NUMBER_" << number;
        return std::to_string(number);
    }
    else if (tag == PrimaryExpAST::LVAL)
    {
        if (symbol_tb_stack.Get_type(lval->ident) == Symbol::INT)
        {
            std::string temp = symbol_tb_stack.Get_ir_name(lval->ident);
            std::string temp2 = symbol_tb_stack.Get_count();
            ks.appendaddtab(temp2 + " = " + "load " + temp + '\n');
            return temp2;
        }
        else if (symbol_tb_stack.Get_type(lval->ident) == Symbol::CONST)
        {
            return std::to_string(symbol_tb_stack.Get_value(lval->ident));
        }
        else if (symbol_tb_stack.Get_type(lval->ident) == Symbol::CONST_ARRAY ||
                 symbol_tb_stack.Get_type(lval->ident) == Symbol::ARRAY){
            std::string tmp = DumpLval();
            string tmp2 = symbol_tb_stack.Get_count();
            ks.appendaddtab(tmp2 + " = load " + tmp + '\n');
            return tmp2;
        }
    }

    std::cout << "}";
}

/**
 * 返回数组中某个元素的指针
 * 其中，name是数组在Koopa IR中的名字，index是元素在数组中的下标
*/
std::string getElemPtr(const std::string &name, const std::vector<std::string>& index){
    if(index.size() == 1){
        string tmp = symbol_tb_stack.Get_count();
        ks.getelemptr(tmp, name, index[0]);
        return tmp;
    } 

    string tmp = symbol_tb_stack.Get_count();
    ks.getelemptr(tmp, name, index[0]);
    vector<string> tmp2 = vector<string>(index.begin() + 1, index.end());
    return getElemPtr(tmp, tmp2);
    
}

std::string PrimaryExpAST::DumpLval() const{
    return lval->Dump();
}

std::string LValAST::Dump() const {
    std::vector<std::string> idx;
    std::vector<int> arr_size;

    for (auto& exp : exp_list) {
        idx.push_back(exp->Dump());
    }

    symbol_tb_stack.Get_array_size(ident, arr_size);

    // hint: array_size可以是-1开头的，说明这个数组是函数中使用的参数
    // 如 a[-1][3][2],表明a是参数 a[][3][2], 即 *[3][2].
    // 此时第一步不能用getelemptr，而应该getptr
    std::string ir_name = symbol_tb_stack.Get_ir_name(ident);
    std::string result;

    if (arr_size.size() != 0 && arr_size[0] == -1) {
        std::vector<int> sub_len(arr_size.begin() + 1, arr_size.end());
        std::string tmp_val = symbol_tb_stack.Get_count();
        ks.appendaddtab(tmp_val + " = load " + ir_name + '\n');
        std::string first_idxed = symbol_tb_stack.Get_count();
        ks.appendaddtab(first_idxed + " = getptr " + tmp_val + ", " + idx[0] + "\n");

        if (idx.size() > 1) {
            result = getElemPtr(
                first_idxed,
                std::vector<std::string>(
                    idx.begin() + 1, idx.end()
                )
            );
        } else {
            result = first_idxed;
        }

    } else {
        result = getElemPtr(ir_name, idx);
    }

    if (idx.size() < arr_size.size()) {
        // 一定是作为函数参数即实参使用，因为下标不完整
        std::string real_param = symbol_tb_stack.Get_count();
        ks.getelemptr(real_param, result, "0");
        return real_param;
    }

    return result;
}


int PrimaryExpAST::Get_value()
{
    if (tag == PrimaryExpAST::EXP)
    {
        return exp->Get_value();
    }
    else if (tag == PrimaryExpAST::NUMBER)
    {
        return number;
    }
    else if (tag == PrimaryExpAST::LVAL)
    {
        return symbol_tb_stack.Get_value(lval->ident);
    }
}

int ConstExpAST::Get_value()
{
    return exp->Get_value();
}