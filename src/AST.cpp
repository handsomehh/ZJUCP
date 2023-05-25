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

void CompUnitAST::Dump() const
{
    std::cout << "CompUnitAST { ";
    ks.func_decl();
    symbol_tb_stack.func_decl_init();
    for (auto &i : decls)
    {
        i->Dump_Global();
    }

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
void ConstDefAST::Dump() const
{
    std::cout << "ConstDefAST { def:" << ident;
    // func_def->Dump();
    // 这个判断条件是为了保证在当前作用域，该常量只能被定义一次
    if (symbol_tb_stack.is_exist(ident) == symbol_tb_stack.size())
    {
        std::cout << "redefined const val: " << ident;
        return;
    }

    int value = const_init_val->Dump();
    std::string ir_name = symbol_tb_stack.Get_var_name(ident);
    symbol_tb_stack.insert(ident, ir_name, value, SymbolTable::CONST);
    std::cout << " }";
    // ks.declLibFunc();
}

void ConstDefAST::Dump_Global() const
{
    std::cout << "ConstDefAST { def:" << ident;
    // func_def->Dump();
    // 这个判断条件是为了保证在当前作用域，该常量只能被定义一次
    if (symbol_tb_stack.is_exist_global(ident))
    {
        std::cout << "Global redefined const val: " << ident;
        return;
    }

    int value = const_init_val->Dump();
    std::string ir_name = symbol_tb_stack.Get_var_name(ident);
    symbol_tb_stack.insert_global(ident, ir_name, value, SymbolTable::CONST);
    std::cout << " }";
    // ks.declLibFunc();
}

int ConstInitValAST::Dump() const
{
    return const_exp->Get_value();
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
    cout << "VaeDefAST { def:" << ident;

    // 这个判断条件是为了保证在当前作用域，该变量只能被定义一次
    if (symbol_tb_stack.is_exist(ident) == symbol_tb_stack.size())
    {
        cout << "Var " << ident << " has been defined" << endl;
        return;
    }

    std::string ir_name = symbol_tb_stack.Get_var_name(ident);
    ks.appendaddtab(ir_name + ks.alloc32i);
    symbol_tb_stack.insert(ident, ir_name, SymbolTable::INT);
    if (init_val)
    {
        std::string res = init_val->exp->Dump();
        ks.appendaddtab("store " + res + ", " + ir_name + '\n');
    }

    cout << " }";
}
void VarDefAST::Dump_Global() const
{
    cout << "VaeDefAST { def:" << ident;

    // 这个判断条件是为了保证在当前作用域，该变量只能被定义一次
    if (symbol_tb_stack.is_exist_global(ident))
    {
        cout << "Global Var " << ident << " has been defined" << endl;
        return;
    }
    std::string ir_name = symbol_tb_stack.Get_var_name(ident);
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
        if (i->tag == BlockItemAST::STMT)
        {
            if (i->stmt->tag == StmtAST::RETURN)
            {
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
        cout << "RETURN ";
        res = exp->Dump();
        ks.ret(res);

        ctrl = false;
    }
    else if (tag == StmtAST::ASSIGN)
    {
        std::string res = exp->Dump(); // 右表达式结果
        std::string to = lval->ident;  // 目标变量
        cout << "ASSIGN";
        int tb_id = symbol_tb_stack.is_exist(to);
        if (tb_id > 0)
        {
            std::string ir_name;
            ks.appendaddtab("store " + res + ", " + symbol_tb_stack.Get_ir_name(to) + '\n');
        }
        else
        {
            cout << "assgin to a undeclear var: " << to;
        }
        // res = lval->Dump();
    }
    else if (tag == StmtAST::EXP)
    {
        if (exp)
        {
            cout << "Expression_NULL";
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
        std::cout << "}";
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
        cout << "NUMBER_" << number;
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
    }
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