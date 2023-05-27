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
class FuncFParamsAST;
class FuncFParamAST;
class FuncRParamsAST;

class ExpAST;
class PrimaryExpAST;
class UnaryExpAST;
class MulExpAST;
class AddExpAST;
class RelExpAST;
class EqExpAST;
class LAndExpAST;
class LOrExpAST;

class DeclAST;
class ConstDeclAST;
class BTypeAST;
class ConstDefAST;
class ConstInitValAST;
class VarDeclAST;
class VarDefAST;
class InitValAST;
class BlockItemAST;

class LValAST;
class ConstExpAST;

// 数组相关
class ArrayIndexListAST;
class LvalArrayIndexListAST;

class BaseAST
{
public:
    virtual ~BaseAST() = default;
    // virtual std::string Dump()const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST
{
public:
    // 用智能指针管理对象
    std::vector<std::unique_ptr<FuncDefAST>> func_defs;
    std::vector<std::unique_ptr<DeclAST>> decls;
    void Dump() const;
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST
{
public:
    // std::unique_ptr<BTypeAST> btype;
    std::unique_ptr<FuncFParamsAST> params;

    std::unique_ptr<BTypeAST> func_type;
    std::string ident;
    std::unique_ptr<BlockAST> block;
    void Dump() const;
};

class FuncFParamsAST : public BaseAST
{
public:
    std::vector<std::unique_ptr<FuncFParamAST>> params;
    void Dump() const;
};

class FuncFParamAST : public BaseAST
{
public:
    std::unique_ptr<BTypeAST> btype;
    std::string ident;
    std::string Dump() const;
};

class FuncTypeAST : public BaseAST
{
public:
    enum TAG
    {
        INT,
        VOID
    };
    TAG tag;
    void Dump() const;
};

class BlockAST : public BaseAST
{
public:
    std::vector<std::unique_ptr<BlockItemAST>> blockitem;
    std::string Dump() const;
};
class BlockItemAST : public BaseAST
{
public:
    enum TYPE
    {
        DECL,
        STMT
    };
    TYPE tag;
    std::unique_ptr<DeclAST> decl;
    std::unique_ptr<StmtAST> stmt;
    std::string Dump() const;
};
class StmtAST : public BaseAST
{
public:
    // int number;
    enum TYPE {RETURN, ASSIGN, EXP, BLOCK, IF, WHILE, BREAK, CONTINUE};
    TYPE tag;
    std::unique_ptr<ExpAST> exp;
    std::unique_ptr<LValAST> lval;
    std::unique_ptr<BlockAST> block;
    std::unique_ptr<StmtAST> if_stmt;
    std::unique_ptr<StmtAST> else_stmt;
    std::unique_ptr<StmtAST> while_stmt;
    std::string Dump() const;
};

class ExpAST : public BaseAST
{
public:
    std::unique_ptr<LOrExpAST> l_or_exp;
    std::string Dump() const;
    int Get_value();
};

class LOrExpAST : public BaseAST
{
public:
    enum TYPE
    {
        AND,
        OR_AND
    };
    TYPE tag;
    // or -> and
    std::unique_ptr<LAndExpAST> l_and_exp;
    // or -> or || and
    std::unique_ptr<LOrExpAST> l_or_exp2;
    std::unique_ptr<LAndExpAST> l_and_exp2;
    std::string Dump() const;
    int Get_value();
};

class LAndExpAST : public BaseAST
{
public:
    enum TYPE
    {
        EQ,
        EQ_AND
    };
    TYPE tag;
    // and->eq
    std::unique_ptr<EqExpAST> eq_exp;
    // and -> eq && and
    std::unique_ptr<LAndExpAST> l_and_exp2;
    std::unique_ptr<EqExpAST> eq_exp2;
    std::string Dump() const;
    int Get_value();
};

class EqExpAST : public BaseAST
{
public:
    enum TYPE
    {
        REL,
        EQ_REL
    };
    TYPE tag;
    // eq->rel
    std::unique_ptr<RelExpAST> rel_exp;
    // rq->rel eq
    std::unique_ptr<EqExpAST> eq_exp2;
    std::unique_ptr<RelExpAST> rel_exp2;
    char op;
    std::string Dump() const;
    int Get_value();
};

class RelExpAST : public BaseAST
{
public:
    enum TYPE
    {
        ADD,
        REL_ADD
    };
    TYPE tag;
    // rel->add
    std::unique_ptr<AddExpAST> add_exp;
    // rel->rel add
    std::unique_ptr<RelExpAST> rel_exp2;
    std::unique_ptr<AddExpAST> add_exp2;
    char op[3]; // <,>,<=,>=
    std::string Dump() const;
    int Get_value();
};

class AddExpAST : public BaseAST
{
public:
    enum TYPE
    {
        MUL,
        ADD_MUL
    };
    TYPE tag;
    std::unique_ptr<MulExpAST> mul_exp;

    std::unique_ptr<AddExpAST> add_exp2;
    std::unique_ptr<MulExpAST> mul_exp2;
    char op;
    std::string Dump() const;
    int Get_value();
};

class MulExpAST : public BaseAST
{
public:
    enum TYPE
    {
        UNARY,
        MUL_UNARY
    };
    TYPE tag;

    std::unique_ptr<UnaryExpAST> unary_exp;

    std::unique_ptr<MulExpAST> mul_exp2;
    std::unique_ptr<UnaryExpAST> unary_exp2;
    char op;
    std::string Dump() const;
    int Get_value();
};

class UnaryExpAST : public BaseAST
{
public:
    enum TYPE
    {
        PRIMARY,
        UNARY,
        FUN
    };
    TYPE tag;

    std::unique_ptr<PrimaryExpAST> primary_exp;

    char op;
    std::unique_ptr<UnaryExpAST> unary_exp;
    std::unique_ptr<FuncRParamsAST> func_params;
    std::string ident;

    std::string Dump() const;
    int Get_value();
};

class PrimaryExpAST : public BaseAST
{
public:
    enum TYPE
    {
        EXP,
        NUMBER,
        LVAL
    };
    TYPE tag;
    std::unique_ptr<ExpAST> exp;
    std::unique_ptr<LValAST> lval;
    int number;
    std::string Dump() const;
    int Get_value();
    std::string DumpLval() const;
};

class DeclAST : public BaseAST {
public:
    enum TYPE
    {
        CONST,
        VAR
    };
    TYPE tag;
    std::unique_ptr<ConstDeclAST> const_decl;
    std::unique_ptr<VarDeclAST> var_decl;
    void Dump() const;
    void Dump_Global() const;
};

class ConstDeclAST : public BaseAST
{
public:
    std::unique_ptr<BTypeAST> btype;
    std::vector<std::unique_ptr<ConstDefAST>> const_defs;
    void Dump() const;
    void Dump_Global() const;
};

class VarDeclAST : public BaseAST
{
public:
    std::unique_ptr<BTypeAST> btype;
    std::vector<std::unique_ptr<VarDefAST>> var_defs;
    void Dump() const;
    void Dump_Global() const;
};

class BTypeAST : public BaseAST
{
public:
    enum TAG
    {
        INT,
        VOID
    };
    TAG tag;
    void Dump() const;
};

class ConstDefAST : public BaseAST
{
public:
    enum TYPE {SINGLE, ARRAY};
    TYPE tag;
    std::string ident;
    std::vector<std::unique_ptr<ConstExpAST>> const_exp_list; // 一组常数值，表述数组的维度，例如vec = {5,5} <=> a[5][5]
    std::unique_ptr<ConstInitValAST> const_init_val;
    void Dump() const;
    void Dump_Global() const;
};

class ArrayIndexListAST : public BaseAST{
public:
    std::vector<std::unique_ptr<ConstExpAST>> const_exp_list;
};

class VarDefAST : public BaseAST
{
public:
    enum TYPE {SINGLE, ARRAY};
    TYPE tag;
    std::string ident;
    std::vector<std::unique_ptr<ConstExpAST>> const_exp_list; // 一组常数值，表述数组的维度，例如vec = {5,5} <=> a[5][5]
    std::unique_ptr<InitValAST> init_val;
    void Dump() const;
    void Dump_Global() const;
};

class InitValAST : public BaseAST
{
public:
    enum TYPE {SINGLE, ARRAY};
    TYPE tag;
    std::unique_ptr<ExpAST> exp;
    std::vector<std::unique_ptr<InitValAST>> exp_list; // 数组初始值
    int Get_value();
    void getInitVal(std::string *ptr, const std::vector<int> &len, bool is_global = false) const;
};

class ConstInitValAST : public BaseAST
{
public:
    enum TYPE {SINGLE, ARRAY};
    TYPE tag;
    std::unique_ptr<ConstExpAST> const_exp;
    std::vector<std::unique_ptr<ConstInitValAST>> const_exp_list; // const array
    int Dump() const;
    void getInitVal(std::string *ptr, const std::vector<int> &len) const;
};

class LValAST : public BaseAST
{
public:
    enum TYPE {IDENT, ARRAY};
    TYPE tag;
    std::string ident;
    std::vector<std::unique_ptr<ExpAST>> exp_list;// 数组的情况，记录数组下标
    std::string Dump() const;
};

class LvalArrayIndexListAST : public BaseAST {
public:
    std::vector<std::unique_ptr<ExpAST>> exp_list;
};


class ConstExpAST : public BaseAST
{
public:
    std::unique_ptr<ExpAST> exp;
    int Get_value();
};
class FuncRParamsAST : public BaseAST
{
public:
    std::vector<std::unique_ptr<ExpAST>> exps;
    std::string Dump() const;
};