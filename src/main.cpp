#include <cassert>
#include <fstream>
#include <unordered_map>
#include<iostream>
#include "AST.h"


#include "koopa.h"
#include "tools.h"

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);
extern KoopaString ks;

using namespace std;

int main(int argc, const char *argv[]) {
  assert(argc == 5);
    auto mode = argv[1];
    auto input = argv[2];
    auto output = argv[4];

    // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
    yyin = fopen(input, "r");
    assert(yyin);
    
    // 输出文件
    // ofstream fout(output);
  // // 获取测试用例
    

    unique_ptr<BaseAST> base_ast;
    unique_ptr<CompUnitAST> ast;

    auto ret = yyparse(base_ast);
    assert(!ret);

    ast.reset((CompUnitAST *)base_ast.release());
    ast->Dump();
    
    const char *str = ks.c_str();
    
    // ifstream ihaha(input);
    // ofstream fhaha("./testcase.txt", ios::app);
    // string tmp;
    // fhaha <<"filename: " << input << endl;
    // while(getline(ihaha, tmp)){
    //     fhaha << tmp + "\n";
    // }
    // fhaha.close();ihaha.close();

    ofstream ofs(output);
    ofstream fout2("./myout.txt",ios::app);
    fout2 << str<<endl; 
    fout2.close();
    ofs << str;
    ofs.close();
    return 0;
}
