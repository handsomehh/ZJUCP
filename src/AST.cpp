#include "AST.h"
#include <memory>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include "tools.h"

using namespace std;
KoopaString ks;

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
    block->Dump();
    std::cout << " }";
    
    ks.ret(to_string(block->stmt->number));
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

void BlockAST::Dump()const {
    std::cout << "BlockAST { ";
    // func_def->Dump();
    stmt->Dump();
    std::cout << " }";
}

void StmtAST::Dump()const {
    std::cout << "StmtAST { ";
    // func_def->Dump();
    std::cout<<number;
    std::cout << " }";
}
