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
/*
Stmt        ::= "return" Exp ";";


Exp         ::= LOrExp ;
PrimaryExp  ::= "(" Exp ")" | Number;
Number      ::= INT_CONST;
UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
UnaryOp     ::= "+" | "-" | "!";
MulExp      ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
AddExp      ::= MulExp | AddExp ("+" | "-") MulExp;
RelExp      ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
EqExp       ::= RelExp | EqExp ("==" | "!=") RelExp;
LAndExp     ::= EqExp | LAndExp "&&" EqExp;
LOrExp      ::= LAndExp | LOrExp "||" LAndExp;
Stmt CompUnit Exp PrimaryExp UnaryExp  MulExp 
AddExp RelExp EqExp LAndExp LOrExp
*/
class ExpAST;
class PrimaryExpAST;
class UnaryExpAST;
class MulExpAST;
class AddExpAST;
class RelExpAST;
class EqExpAST;
class LAndExpAST;
class LOrExpAST;

class BaseAST {
 public:
  virtual ~BaseAST() = default;
  // virtual std::string Dump()const = 0;
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
    std::string Dump() const;
};

class StmtAST : public BaseAST {
 public:
    // int number;
    std::unique_ptr<ExpAST> exp;
    std::string Dump() const;
};

class ExpAST : public BaseAST {
public:
    std::unique_ptr<LOrExpAST> l_or_exp;
    std::string Dump() const;
};

class LOrExpAST : public BaseAST {
public:
    enum TYPE {AND,OR_AND};
    TYPE tag;
    //or -> and
    std::unique_ptr<LAndExpAST> l_and_exp;
    //or -> or || and
    std::unique_ptr<LOrExpAST> l_or_exp2;
    std::unique_ptr<LAndExpAST> l_and_exp2;
    std::string Dump() const;
};

class LAndExpAST : public BaseAST {
public:
    enum TYPE {EQ, EQ_AND};
    TYPE tag;
    //and->eq
    std::unique_ptr<EqExpAST> eq_exp;
    //and -> eq && and
    std::unique_ptr<LAndExpAST> l_and_exp2;
    std::unique_ptr<EqExpAST> eq_exp2;
    std::string Dump() const;
};

class EqExpAST : public BaseAST {
public:
    enum TYPE {REL,EQ_REL};
    TYPE tag;
    //eq->rel
    std::unique_ptr<RelExpAST> rel_exp;
    //rq->rel eq
    std::unique_ptr<EqExpAST> eq_exp2;
    std::unique_ptr<RelExpAST> rel_exp2;
    char op;
    std::string Dump() const;
};

class RelExpAST : public BaseAST {
public:
    enum TYPE {ADD, REL_ADD};
    TYPE tag;
    //rel->add
    std::unique_ptr<AddExpAST> add_exp;
    //rel->rel add
    std::unique_ptr<RelExpAST> rel_exp2;
    std::unique_ptr<AddExpAST> add_exp2;
    char op[3];     // <,>,<=,>=
    std::string Dump() const;
};

class AddExpAST : public BaseAST {
public:
    enum TYPE {MUL, ADD_MUL};
    TYPE tag;
    std::unique_ptr<MulExpAST> mul_exp;

    std::unique_ptr<AddExpAST> add_exp2;
    std::unique_ptr<MulExpAST> mul_exp2;
    char op;
    std::string Dump() const;
};

class MulExpAST : public BaseAST {
public:
    enum TYPE {UNARY, MUL_UNARY};
    TYPE tag;

    std::unique_ptr<UnaryExpAST> unary_exp;

    std::unique_ptr<MulExpAST> mul_exp2;
    std::unique_ptr<UnaryExpAST> unary_exp2;
    char op;
    std::string Dump() const;
};

class UnaryExpAST : public BaseAST {
public:
    enum TYPE { PRIMARY, UNARY};
    TYPE tag;

    std::unique_ptr<PrimaryExpAST> primary_exp;

    char op;
    std::unique_ptr<UnaryExpAST> unary_exp;

    std::string Dump() const;
};

class PrimaryExpAST : public BaseAST {
public:
    enum TYPE {EXP , NUMBER};
    TYPE tag;
    std::unique_ptr<ExpAST> exp;

    int number;
    std::string Dump() const ;
};
