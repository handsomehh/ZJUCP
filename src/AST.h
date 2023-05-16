#pragma once
#include <string>
#include <memory>
#include <vector>
using namespace std;
// 所有类的声明
class BaseAST; 
class CompUnitAST;
class FuncDefAST;
class FuncTypeAST;
class BlockAST;
class StmtAST;


class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Dump()const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<FuncDefAST> func_def;
  void Dump() const;
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<FuncTypeAST> func_type;
  std::string ident;
  std::unique_ptr<BlockAST> block;
  void Dump() const;
};

class FuncTypeAST : public BaseAST {
 public:
    enum TAG {INT};
    TAG tag;
    void Dump() const;
};

class BlockAST : public BaseAST {
 public:
    std::unique_ptr<StmtAST> stmt;
    void Dump() const;
};

class StmtAST : public BaseAST {
 public:
    int number;
    void Dump() const;
};