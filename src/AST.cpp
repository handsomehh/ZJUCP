#include "AST.h"
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

class Counter{
  public:
    int count = 0;
    std::string Get_count(){
        int temp = count++;
        std::string name = "%"+std::to_string(temp);
        return name;
    }
};

Counter* mycount = new Counter();

void CompUnitAST::Dump()const {
    std::cout << "CompUnitAST { ";
    func_def->Dump();
    std::cout << " }";
    // ks.declLibFunc();
}

void FuncDefAST::Dump()const {
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
    std::string res = stmt->Dump();
    std::cout << " }";
    return res;
}

std::string StmtAST::Dump()const {
    std::cout << "StmtAST { ";
    // func_def->Dump();
    std::string res = exp->Dump();
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

        std::string res1 = mycount->Get_count();
        ks.logic(res1,src1,"0","ne");
        std::string res2 = mycount->Get_count();
        ks.logic(res2,src2,"0","ne");
        std::string res3 = mycount->Get_count();
        ks.logic(res3,res1,res2,"or");
        return res3;
    }
    std::cout << " }";
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

        std::string res1 = mycount->Get_count();
        ks.logic(res1,src1,"0","ne");
        std::string res2 = mycount->Get_count();
        ks.logic(res2,src2,"0","ne");
        std::string res3 = mycount->Get_count();
        ks.logic(res3,res1,res2,"and");
        return res3;
    }
    std::cout << " }";
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

        std::string res = mycount->Get_count();
        if(op == '='){
            ks.logic(res,src1,src2,"eq");
        }else{
            ks.logic(res,src1,src2,"ne");
        }
        return res;
    }
    std::cout << " }";
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

        std::string res = mycount->Get_count();
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

        std::string res = mycount->Get_count();
        if(op == '+'){
            ks.logic(res,src1,src2,"add");
        }else if(op == '-'){
            ks.logic(res,src1,src2,"sub");
        }
        return res;
    }
    std::cout << " }";
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

        std::string res = mycount->Get_count();

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
            std::string res = mycount->Get_count();
            ks.logic(res,"0",src1,"sub");
            return res;
        }else if(op == '!'){
            std::string res = mycount->Get_count();
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
        return std::to_string(number);
    }
}