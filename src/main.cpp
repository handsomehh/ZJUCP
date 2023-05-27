#include <cassert>
#include <fstream>
#include <unordered_map>
#include <iostream>
#include "AST.h"

#include "koopa.h"
#include "tools.h"
#include "Visit.h"

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);
extern KoopaString ks;
extern string RiscvString;

using namespace std;

int main(int argc, const char *argv[])
{
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
  yyin = fopen(input, "r");
  assert(yyin);

  // //输出文件
  // ofstream fout(output);
  // // 获取测试用例
  // ifstream ihaha(input);
  // ofstream fhaha("./testcase.txt", ios::app);
  // string tmp;
  // fhaha << "filename: " << input << endl;
  // while (getline(ihaha, tmp))
  // {
  //   fhaha << tmp + "\n";
  // }
  // fhaha.close();
  // ihaha.close();
  // return 0;

  unique_ptr<BaseAST> base_ast;
  unique_ptr<CompUnitAST> ast;

  std::cout << "parsing..." << std::endl;
  auto ret = yyparse(base_ast);
  assert(!ret);
  assert(base_ast);
  std::cout << "parse success!!" << std::endl;
  std::cout << "111" << endl;
  ast.reset((CompUnitAST *)base_ast.release());
  std::cout << "222" << endl;
  ast->Dump();

  const char *str = ks.c_str();

  ofstream ofs(output);

  if (std::string(mode) == "-koopa")
  {
    ofs << str;
    ofs.close();
    return 0;
  }

  koopa_program_t program;
  koopa_error_code_t err_c = koopa_parse_from_string(str, &program);
  assert(err_c == KOOPA_EC_SUCCESS); // 确保解析时没有出错
  // 创建一个 raw program builder, 用来构建 raw program
  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  // 将 Koopa IR 程序转换为 raw program
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  // 释放 Koopa IR 程序占用的内存
  koopa_delete_program(program);
  // 处理 raw program
  // TODO: Implement Code here:
  Visit(raw);

  if (std::string(mode) == "-riscv")
  {
    ofs << RiscvString;
  }

  ofs.close();
  // 处理完成, 释放 raw program builder 占用的内存
  // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
  // 所以不要在 raw program 处理完毕之前释放 builder
  koopa_delete_raw_program_builder(builder);

  ofstream fout2("./myout_ir.txt", ios::app);
  fout2 << str << endl;
  fout2.close();
  ofstream fout3("./myout_riscv.txt", ios::app);
  fout2 << RiscvString << endl;
  fout2.close();
  return 0;
}
