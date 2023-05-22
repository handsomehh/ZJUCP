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
KoopaString ks;
SymbolTableStack symbol_tb_stack;

void CompUnitAST::Dump()const {
    std::cout << "CompUnitAST { ";
    func_def->Dump();
    std::cout << " }";
    // ks.declLibFunc();
}
void DeclAST::Dump()const {
    std::cout << "DeclAST { ";
    // func_def->Dump();
    if (tag == DeclAST::CONST){
        const_decl->Dump();
    }else if(tag == DeclAST::VAR){
        var_decl->Dump();
    }
    std::cout << " }";
    // ks.declLibFunc();
}

void ConstDeclAST::Dump()const {
    std::cout << "ConstDeclAST { ";
    // func_def->Dump();
    for (auto &i : const_defs){
        i->Dump();
    }
    std::cout << " }";
    // ks.declLibFunc();
}

void ConstDefAST::Dump()const {
    std::cout << "ConstDefAST { def:" << ident;
    // func_def->Dump();
    int value = const_init_val->Dump();

    // 这个判断条件是为了保证在当前作用域，该常量只能被定义一次
    if(symbol_tb_stack.is_exist(ident) == symbol_tb_stack.size()){
        std::cout<<"redefined const val: "<<ident;
        return;
    }
    symbol_tb_stack.insert(ident,value,SymbolTable::CONST);
    std::cout << " }";
    // ks.declLibFunc();
}

int ConstInitValAST::Dump()const {
    return const_exp->Get_value();   
}

void VarDeclAST::Dump()const {
    std::cout << "VarDeclAST {";
    for (auto &i : var_defs){
        i->Dump();
    }
    std::cout << "}";
    // return const_exp-Dump();   
}

void VarDefAST::Dump()const {
    cout << "VaeDefAST { def:" << ident;

    // 这个判断条件是为了保证在当前作用域，该变量只能被定义一次
    if (symbol_tb_stack.is_exist(ident) == symbol_tb_stack.size()){
        cout << "Var " << ident << " has been defined" << endl;
        return ;
    }

    std::string name = "@" + ident + std::to_string(symbol_tb_stack.size());
    ks.appendaddtab(name + ks.alloc32i);
    symbol_tb_stack.insert(ident,SymbolTable::INT);
    if(init_val){
        int value = init_val->Get_value();
        ks.appendaddtab("store " + std::to_string(value) + ", " + name + '\n');
        symbol_tb_stack.Update(ident,value);  
    }

    cout << " }";
}

int InitValAST::Get_value() {
    return  exp->Get_value();
    // return const_exp-Dump();
}
void FuncDefAST::Dump()const {
    symbol_tb_stack.Reset_count();

    std::cout << "FuncDefAST { ";
    
    ks.append("fun @" + ident + "(");
    ks.append(")");
    if (func_type->tag == FuncTypeAST::INT)
    {
        ks.append(": i32");
    }
    ks.append(" {\n");
    ks.label("%entry");
    
    func_type->Dump();
    std::cout << ", " << ident << ", ";
    std::string res = block->Dump();
    std::cout << " }";
    
    ks.ret(res);
    ks.append("}\n\n");

}

void FuncTypeAST::Dump()const {
    std::cout << "FuncTypeAST { ";
    // func_def->Dump();
    if(tag == FuncTypeAST::INT){
        std::cout<<"int";
    }
    std::cout << " }";
}

std::string BlockAST::Dump()const {
    std::cout << "BlockAST { ";
    // func_def->Dump();

    symbol_tb_stack.alloc(); // 每进入一个新的block，新建一张symbol table

    std::string res;
    for(auto &i : blockitem){
        res = i->Dump();
    }
    std::cout << " }";

    symbol_tb_stack.quit(); // 每离开一个block，删除symbol table
    return res;
}

std::string BlockItemAST::Dump()const {
    std::cout << "BlockItemAST { ";
    // func_def->Dump();
    std::string res = "-1";
    if(tag == BlockItemAST::DECL){
        decl->Dump();
    }else if(tag == BlockItemAST::STMT){
        res = stmt->Dump();
    }
    std::cout << " }; ";
    return res;
}

std::string StmtAST::Dump()const {
    std::cout << "StmtAST { ";
    // func_def->Dump();
    std::string res = "-1";
    if (tag == StmtAST::RETURN){
        cout << "RETURN ";
        res = exp->Dump();
    }
    else if (tag == StmtAST::ASSIGN){
        int res_int = exp->Get_value();
        string to = lval->ident;
        cout << "ASSIGN "<< res_int << " to " << to;
        int tb_id = symbol_tb_stack.is_exist(to);
        if(tb_id > 0){
            ks.appendaddtab("store " + std::to_string(res_int) + ", @" + to + std::to_string(tb_id) + '\n');
            symbol_tb_stack.Update(to,res_int);
            res = std::to_string(res_int);
        }else{
            cout<< "assgin to a undeclear var: "<<to;
        }
        // res = lval->Dump();
    }
    else if (tag == StmtAST::EXP){
        if (exp){
            cout << "EXP_NULL" ;
            res = exp->Dump();
        }
    }
    else if (tag == StmtAST::BLOCK){
        res = block->Dump();
    }
    // exp->Dump();
    std::cout << " }";
    return res;
}

std::string ExpAST::Dump()const {
    std::cout << "ExpAST { ";
    // func_def->Dump();
    std::string res = l_or_exp->Dump();
    std::cout << " }";
    return res;
}

int ExpAST::Get_value(){

    return l_or_exp->Get_value();

}
int LOrExpAST::Get_value(){
    if (tag == LOrExpAST::AND){

        return l_and_exp->Get_value();

    }
    else if(tag == LOrExpAST::OR_AND){

        return l_or_exp2->Get_value()||l_and_exp2->Get_value();
    }
}
std::string LOrExpAST::Dump()const {
    std::cout << "LOrExpAST { ";
    // func_def->Dump();
    // l_or_exp->Dump();
    if (tag == LOrExpAST::AND){
        return l_and_exp->Dump();
    }
    else if(tag == LOrExpAST::OR_AND){
        std::string src1 = l_or_exp2->Dump();
        std::string src2 = l_and_exp2->Dump();

        std::string res1 = symbol_tb_stack.Get_count();
        ks.logic(res1,src1,"0","ne");
        std::string res2 = symbol_tb_stack.Get_count();
        ks.logic(res2,src2,"0","ne");
        std::string res3 = symbol_tb_stack.Get_count();
        ks.logic(res3,res1,res2,"or");
        return res3;
    }
    std::cout << " }";
}

int LAndExpAST::Get_value(){
    if (tag == LAndExpAST::EQ){

        return eq_exp->Get_value();
        
    }
    else if(tag == LAndExpAST::EQ_AND){

        return l_and_exp2->Get_value() && eq_exp2->Get_value();
    }
}

std::string LAndExpAST::Dump()const {
    std::cout << "LAndExpAST { ";
    // func_def->Dump();
    // l_or_exp->Dump();
    if (tag == LAndExpAST::EQ){
        return eq_exp->Dump();
    }
    else if(tag == LAndExpAST::EQ_AND){
        std::string src1 = l_and_exp2->Dump();
        std::string src2 = eq_exp2->Dump();

        std::string res1 = symbol_tb_stack.Get_count();
        ks.logic(res1,src1,"0","ne");
        std::string res2 = symbol_tb_stack.Get_count();
        ks.logic(res2,src2,"0","ne");
        std::string res3 = symbol_tb_stack.Get_count();
        ks.logic(res3,res1,res2,"and");
        return res3;
    }
    std::cout << " }";
}

int EqExpAST::Get_value(){
    if (tag == EqExpAST::REL){

        return rel_exp->Get_value();
        
    }
    else if(tag == EqExpAST::EQ_REL){
        if(op=='='){
            return eq_exp2->Get_value() == rel_exp2->Get_value();
        }else if(op == '!'){
            return eq_exp2->Get_value()!=rel_exp2->Get_value();
        }
    }
}

std::string EqExpAST::Dump()const {
    std::cout << "EqExpAST { ";
    // func_def->Dump();
    // l_or_exp->Dump();
    if (tag == EqExpAST::REL){
        return rel_exp->Dump();
    }
    else if(tag == EqExpAST::EQ_REL){
        std::string src1 = eq_exp2->Dump();
        std::string src2 = rel_exp2->Dump();

        std::string res = symbol_tb_stack.Get_count();
        if(op == '='){
            ks.logic(res,src1,src2,"eq");
        }else{
            ks.logic(res,src1,src2,"ne");
        }
        return res;
    }
    std::cout << " }";
}

int RelExpAST::Get_value(){
    if (tag == RelExpAST::ADD){

        return add_exp->Get_value();
        
    }
    else if(tag == RelExpAST::REL_ADD){
         if(!strcmp(op,"<")){
            return rel_exp2->Get_value() < add_exp2->Get_value();
        }else if(!strcmp(op,">")){
            return rel_exp2->Get_value() > add_exp2->Get_value();
        }else if(!strcmp(op,"<=")){
            return rel_exp2->Get_value() <= add_exp2->Get_value();
        }else if(!strcmp(op,">=")){
            return rel_exp2->Get_value() >= add_exp2->Get_value();
        }
    }
}
std::string RelExpAST::Dump()const {
    std::cout << "RelExpAST { ";
    // func_def->Dump();
    // l_or_exp->Dump();
    if (tag == RelExpAST::ADD){
        return add_exp->Dump();
    }
    else if(tag == RelExpAST::REL_ADD){
        std::string src1 = rel_exp2->Dump();
        std::string src2 = add_exp2->Dump();

        std::string res = symbol_tb_stack.Get_count();
        if(!strcmp(op,"<")){
            ks.logic(res,src1,src2,"lt");
        }else if(!strcmp(op,">")){
            ks.logic(res,src1,src2,"gt");
        }else if(!strcmp(op,"<=")){
            ks.logic(res,src1,src2,"le");
        }else if(!strcmp(op,">=")){
            ks.logic(res,src1,src2,"ge");
        }
        return res;
    }
    std::cout << " }";
}

int AddExpAST::Get_value(){
    if (tag == AddExpAST::MUL){

        return mul_exp->Get_value();
        
    }
    else if(tag == AddExpAST::ADD_MUL){
         if(op == '+'){
            return add_exp2->Get_value() + mul_exp2->Get_value();
        }else if(op == '-'){
            return add_exp2->Get_value() - mul_exp2->Get_value();
        }
    }
}

std::string AddExpAST::Dump()const {
    std::cout << "AddExpAST { ";
    // func_def->Dump();
    // l_or_exp->Dump();
    if (tag == AddExpAST::MUL){
        return mul_exp->Dump();
    }
    else if(tag == AddExpAST::ADD_MUL){
        std::string src1 = add_exp2->Dump();
        std::string src2 = mul_exp2->Dump();

        std::string res = symbol_tb_stack.Get_count();
        if(op == '+'){
            ks.logic(res,src1,src2,"add");
        }else if(op == '-'){
            ks.logic(res,src1,src2,"sub");
        }
        return res;
    }
    std::cout << " }";
}
int MulExpAST::Get_value(){
    if (tag == MulExpAST::UNARY){

        return unary_exp->Get_value();
        
    }
    else if(tag == MulExpAST::MUL_UNARY){
         if(op == '*'){
            return mul_exp2->Get_value() * unary_exp2->Get_value();
        }else if(op == '/'){
            return mul_exp2->Get_value() / unary_exp2->Get_value();
        }else if(op == '%'){
            return mul_exp2->Get_value() % unary_exp2->Get_value();
        }
    }
}
std::string MulExpAST::Dump()const {
    std::cout << "MulExpAST { ";
    // func_def->Dump();
    // l_or_exp->Dump();
    if (tag == MulExpAST::UNARY){
        return unary_exp->Dump();
    }
    else if(tag == MulExpAST::MUL_UNARY){
        std::string src1 = mul_exp2->Dump();
        std::string src2 = unary_exp2->Dump();

        std::string res = symbol_tb_stack.Get_count();

        if(op == '*'){
            ks.logic(res,src1,src2,"mul");
        }else if(op == '/'){
            ks.logic(res,src1,src2,"div");
        }else if(op == '%'){
            ks.logic(res,src1,src2,"mod");
        }
        return res;
    }
}
int UnaryExpAST::Get_value(){
    if (tag == UnaryExpAST::PRIMARY){
        return primary_exp->Get_value();
    }
    else if(tag == UnaryExpAST::UNARY){
         if(op == '+'){
            return unary_exp->Get_value();
        }else if(op == '-'){
            return 0 - unary_exp->Get_value() ;
        }else if(op == '!'){
            return (!unary_exp->Get_value());
        }
    }
}
std::string UnaryExpAST::Dump()const {
    std::cout << "UnaryExpAST { ";
    // func_def->Dump();
    // l_or_exp->Dump();
    if (tag == UnaryExpAST::PRIMARY){
        return primary_exp->Dump();
    }
    else if(tag == UnaryExpAST::UNARY){
        std::string src1 = unary_exp->Dump();
        if(op == '+'){
            return src1;
        }else if(op == '-'){
            std::string res = symbol_tb_stack.Get_count();
            ks.logic(res,"0",src1,"sub");
            return res;
        }else if(op == '!'){
            std::string res = symbol_tb_stack.Get_count();
            ks.logic(res,"0",src1,"eq");
            return res;
        }
    }
}

std::string PrimaryExpAST::Dump()const {
    std::cout << "PrimaryExpAST { ";
    if (tag == PrimaryExpAST::EXP){
        return exp->Dump();
    }
    else if(tag == PrimaryExpAST::NUMBER){
        cout << "NUMBER_" << number;
        return std::to_string(number);
    }else if(tag == PrimaryExpAST::LVAL){
        
        return std::to_string(symbol_tb_stack.Get_value(lval->ident));
    }
}

int PrimaryExpAST::Get_value(){
    if (tag == PrimaryExpAST::EXP){
        return exp->Get_value();
    }
    else if(tag == PrimaryExpAST::NUMBER){
         return number;
    }else if(tag == PrimaryExpAST::LVAL){
        return symbol_tb_stack.Get_value(lval->ident);
    }
}

int ConstExpAST::Get_value(){
    return exp->Get_value();
}

