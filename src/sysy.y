%code requires {
  #include <memory>
  #include <string>
  #include "AST.h"
  #include <cassert>
  #include <string.h>
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "AST.h"
#include <cassert>
#include <string.h>
// 声明 lexer 函数和错误处理函数

int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况

%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
  char char_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val

%token INT RETURN EQ NE LEQ BGE AND OR CONST IF ELSE WHILE BREAK CONTINUE VOID
%token <str_val> IDENT
%token <int_val> INT_CONST 
%type <ast_val> FuncDef FuncType Block Stmt CompUnit Exp PrimaryExp UnaryExp  MulExp AddExp RelExp EqExp LAndExp LOrExp Decl ConstDecl VarDecl BType ConstDef VarDef ConstInitVal InitVal  BlockItem LVal ConstExp VarDefAtom ConstDefAtom BlockItemAtom CompUnitList CompUnitAtom FuncFParam FuncFParams FuncRParams ArrayIndexList LvalArrayIndexList ConstInitValList InitValList
%type <int_val> Number
%type <char_val> UnaryOp MULOp AddOp
%%


// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit:
  CompUnitList {
    auto comp_unit = unique_ptr<CompUnitAST>((CompUnitAST *)$1);
    ast = move(comp_unit);
  }
  ;
CompUnitList:
  CompUnitList CompUnitAtom{

    auto comp_unit = (CompUnitAST*)$1;
    auto rec = unique_ptr<CompUnitAST>((CompUnitAST*)$2);
    
    for (auto &i : rec->func_defs){
      comp_unit->func_defs.push_back(std::move(i));
    }
    for (auto &i : rec->decls){
      comp_unit->decls.push_back(std::move(i));
    }
    rec->func_defs.clear();
    rec->decls.clear();
    $$ = comp_unit;

  } | CompUnitAtom {

    $$ = $1;

  }
  ;

CompUnitAtom
  : FuncDef {

    auto comp_unit = new CompUnitAST();
    comp_unit->func_defs.push_back(unique_ptr<FuncDefAST>((FuncDefAST*)$1));
    // comp_unit->Dump();
    $$ = comp_unit;

  } | Decl {
    auto comp_unit = new CompUnitAST();
    comp_unit->decls.push_back(unique_ptr<DeclAST>((DeclAST*)$1));
    // comp_unit->Dump();
    $$ = comp_unit;
  }
  ;

Decl 
  : ConstDecl {

    auto ast = new DeclAST();
    ast->tag = DeclAST::CONST;
    ast->const_decl = unique_ptr<ConstDeclAST>((ConstDeclAST *)$1);
    std::cout<<"const decl"<<std::endl;
    $$ = ast;

  } | VarDecl {

    auto ast = new DeclAST();
    ast->tag = DeclAST::VAR;
    ast->var_decl = unique_ptr<VarDeclAST>((VarDeclAST *)$1);
    $$ = ast;

  }
  ;

ConstDecl
  : CONST BType ConstDef ';'{
    auto ast = (ConstDeclAST *)$3;
    ast->btype = unique_ptr<BTypeAST>((BTypeAST *)$2);
    std::cout<<"CONST BType ConstDef"<<std::endl;

    $$ = ast;
  }
  ;
BType
  : INT {
    auto ast = new BTypeAST();
    ast->tag = BTypeAST::INT;
    std::cout<<"Btype INT";
    $$ = ast;
  } | VOID {
    auto ast = new BTypeAST();
    ast->tag = BTypeAST::VOID;
    std::cout<<"Btype VOID";
    $$ = ast;
  }
  ;
ConstDef
  : ConstDefAtom ',' ConstDef{
      auto ast = new ConstDeclAST();
      auto rec = unique_ptr<ConstDeclAST>((ConstDeclAST*)$3);

      ast->const_defs.push_back(unique_ptr<ConstDefAST>((ConstDefAST*)$1));
      for (auto &i : rec->const_defs){
        ast->const_defs.push_back(std::move(i));
      }
      rec->const_defs.clear();

      $$ = ast;

  } | ConstDefAtom{
      auto ast = new ConstDeclAST();
      ast->const_defs.push_back(unique_ptr<ConstDefAST>((ConstDefAST*)$1));
      $$ = ast;
  }
  ;
ConstDefAtom
  : IDENT '=' ConstInitVal{
      auto ast = new ConstDefAST();
      ast->ident = *unique_ptr<std::string>($1);
      ast->const_init_val = unique_ptr<ConstInitValAST>((ConstInitValAST*)$3);
      $$ = ast;
  } | IDENT ArrayIndexList '=' ConstInitVal{
      auto ast = new ConstDefAST();
      ast->ident = *unique_ptr<std::string>($1);
      ast->tag = ConstDefAST::ARRAY;
      unique_ptr<ArrayIndexListAST> p((ArrayIndexListAST *)$2);
      for (auto &i : p->const_exp_list){
        ast->const_exp_list.emplace_back(i.release());
      }
      ast->const_init_val = unique_ptr<ConstInitValAST>((ConstInitValAST *)$4);
      $$ = ast;
  }
  ;

ArrayIndexList
  : '[' ConstExp ']' {
    auto ast = new ArrayIndexListAST();
    ast->const_exp_list.emplace_back((ConstExpAST *)$2);
    $$ = ast;
  } | ArrayIndexList '[' ConstExp ']'{
    auto ast = (ArrayIndexListAST*)$1;
    ast->const_exp_list.emplace_back((ConstExpAST *)$3);
    $$ = ast;
  }

ConstInitVal
  : ConstExp {
    auto ast = new ConstInitValAST();
    ast->tag = ConstInitValAST::SINGLE;
    ast->const_exp = unique_ptr<ConstExpAST>((ConstExpAST *)$1);
    $$ = ast;
  } | '{' '}'{
    auto ast = new ConstInitValAST();
    ast->tag = ConstInitValAST::ARRAY;
    $$ = ast;
  } | '{' ConstInitValList '}' {
    auto ast = $2;
    $$ = ast;
  }
  ;

ConstInitValList
  : ConstInitVal {
    auto ast = new ConstInitValAST();
    ast->tag = ConstInitValAST::ARRAY;
    ast->const_exp_list.emplace_back((ConstInitValAST *)$1);
    $$ = ast;
  } | ConstInitValList ',' ConstInitVal {
    auto ast = (ConstInitValAST *)$1;
    ast->const_exp_list.emplace_back((ConstInitValAST *)$3);
    $$ = ast;
  }
  ;


VarDecl
  : BType VarDef ';' {
    auto ast = (VarDeclAST *)$2;
    ast->btype = unique_ptr<BTypeAST>((BTypeAST *) $1);
    $$ = ast;
  }
  ;
VarDef
  : VarDefAtom ',' VarDef {
    auto ast = new VarDeclAST();
    auto rec = unique_ptr<VarDeclAST>((VarDeclAST *)$3);
    ast->var_defs.push_back(unique_ptr<VarDefAST>((VarDefAST*)$1));
    
    for (auto &i : rec->var_defs){
        ast->var_defs.push_back(std::move(i));
      }
    rec->var_defs.clear();
    $$ = ast;

  } | VarDefAtom {

    auto ast = new VarDeclAST();
    ast->var_defs.push_back(unique_ptr<VarDefAST>((VarDefAST*)$1));
    $$ = ast;

  }
  ;

VarDefAtom
  : IDENT{
    auto ast = new VarDefAST();
    ast->tag = VarDefAST::SINGLE;
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  }  | IDENT '=' InitVal {
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->tag = VarDefAST::SINGLE;
    ast->init_val = unique_ptr<InitValAST>((InitValAST *)$3);
    $$ = ast;
  } | IDENT ArrayIndexList '=' InitVal{
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->tag = VarDefAST::ARRAY;
    unique_ptr<ArrayIndexListAST> p((ArrayIndexListAST *)$2);
    for (auto &i : p->const_exp_list){
        ast->const_exp_list.emplace_back(i.release());
      }
    ast->init_val = unique_ptr<InitValAST>((InitValAST *)$4);
    $$ = ast;
  } | IDENT ArrayIndexList{
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->tag = VarDefAST::ARRAY;
    unique_ptr<ArrayIndexListAST> p((ArrayIndexListAST *)$2);
    for (auto &i : p->const_exp_list){
        ast->const_exp_list.emplace_back(i.release());
    }
    $$ = ast;
  }
  ;

InitVal
  : Exp{

    auto ast = new InitValAST();
    ast->exp  = unique_ptr<ExpAST>((ExpAST *)$1);
    $$ = ast;
  } | '{' '}' {
    auto ast = new InitValAST();
    ast->tag = InitValAST::ARRAY;
    $$ = ast;
  } | '{' InitValList '}' {
    $$ = $2;
  }
  ;

InitValList
  : InitVal {
    auto ast = new InitValAST();
    ast->tag = InitValAST::ARRAY;
    ast->exp_list.emplace_back((InitValAST *)$1);
    $$ = ast;
  } | InitValList ',' InitVal {
    auto ast = (InitValAST *)$1;
    ast->exp_list.emplace_back((InitValAST *)$3);
    $$ = ast;
  }
  ;

FuncDef
  : BType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BTypeAST>((BTypeAST*)$1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BlockAST>((BlockAST*)$5);
    $$ = ast;
  } | BType IDENT '(' FuncFParams ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BTypeAST>((BTypeAST*)$1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BlockAST>((BlockAST*)$6);
    ast->params = unique_ptr<FuncFParamsAST>((FuncFParamsAST*)$4);
    $$ = ast;
  }
  ;
FuncFParams
  : FuncFParam {
    auto ast = new FuncFParamsAST();
    ast->params.push_back(unique_ptr<FuncFParamAST>((FuncFParamAST*)$1));
    $$ = ast;
  } | FuncFParam ',' FuncFParams {
    auto ast = new FuncFParamsAST();
    ast->params.push_back(unique_ptr<FuncFParamAST>((FuncFParamAST*)$1));
    
    auto rec = unique_ptr<FuncFParamsAST>((FuncFParamsAST *)$3);
    for(auto &i : rec->params){
      ast->params.push_back(std::move(i));
    }
    rec->params.clear();
    $$ = ast;
  }
  ;

FuncFParam
  : BType IDENT {
    auto ast = new FuncFParamAST();
    ast->tag = FuncFParamAST::SINGLE;
    ast->btype = unique_ptr<BTypeAST>((BTypeAST *)$1);
    ast->ident = *unique_ptr<string>($2);
    $$ = ast;
  } | BType IDENT '[' ']' {
    auto ast = new FuncFParamAST();
    ast->tag = FuncFParamAST::ARRAY;
    ast->btype = unique_ptr<BTypeAST>((BTypeAST *)$1);
    ast->ident = *unique_ptr<string>($2);
    $$ = ast;
  } | BType IDENT '[' ']' ArrayIndexList {
    auto ast = new FuncFParamAST();
    ast->tag = FuncFParamAST::ARRAY;
    ast->btype = unique_ptr<BTypeAST>((BTypeAST *)$1);
    ast->ident = *unique_ptr<string>($2);
    std::unique_ptr<ArrayIndexListAST> ptr((ArrayIndexListAST*)$5);
    for (auto &i : ptr->const_exp_list){
      ast->const_exp_list.emplace_back(i.release());
    }
    $$ = ast;
  }
  ;

// 同上, 不再解释
/* FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->tag = FuncTypeAST::INT;
    $$ = ast;
  } | VOID {
    auto ast = new FuncTypeAST();
    ast->tag = FuncTypeAST::VOID;
    $$ = ast;
  }
  ; */

Block
  : '{' BlockItem '}' {
    // auto ast = new BlockAST();
    // ast->blockitem = unique_ptr<BlockItemAST>((BlockItemAST*)$2);
    // std::cout<<"3";
    $$ = $2;
  } | '{' '}' {
    auto ast = new BlockAST();
    $$ = ast;
  }
  ;
BlockItem
  : BlockItemAtom BlockItem{
    
    auto ast = new BlockAST();
    ast->blockitem.push_back(unique_ptr<BlockItemAST>((BlockItemAST*)$1));
    auto rec = unique_ptr<BlockAST>((BlockAST*)$2);

    for(auto &i : rec->blockitem){
      ast->blockitem.push_back(std::move(i));
    }

    rec->blockitem.clear();
    $$ = ast;

  } | BlockItemAtom{
      auto ast = new BlockAST();
      ast->blockitem.push_back(unique_ptr<BlockItemAST>((BlockItemAST*)$1));
      $$ = ast;
  }
  ;

BlockItemAtom
  :Decl{
    
    auto ast = new BlockItemAST();
    ast->tag = BlockItemAST::DECL;
    ast->decl = unique_ptr<DeclAST>((DeclAST*)$1);

    $$ = ast;

  } | Stmt{

    auto ast = new BlockItemAST();
    ast->tag = BlockItemAST::STMT;
    ast->stmt = unique_ptr<StmtAST>((StmtAST*)$1);
    $$ = ast;
  }
  ;

Stmt
  : RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->tag = StmtAST::RETURN;
    ast->exp = unique_ptr<ExpAST>((ExpAST*)$2);
    // std::cout<<"2";
    $$ = ast;
  } | RETURN ';'{
    auto ast = new StmtAST();
    ast->tag = StmtAST::RETURN;
  } | LVal '=' Exp ';' {
    auto ast = new StmtAST();
    ast->tag = StmtAST::ASSIGN;
    ast->exp = unique_ptr<ExpAST>((ExpAST*)$3);
    ast->lval = unique_ptr<LValAST>((LValAST*)$1);
    std::cout<<ast->lval->ident<<"= EXP"<<std::endl;
    // std::cout<<"2";
    $$ = ast;
  } | Exp ';'{
    auto ast = new StmtAST();
    ast->tag = StmtAST::EXP;
    ast->exp = unique_ptr<ExpAST>((ExpAST*)$1);
    $$ = ast;
  } | ';'{
    auto ast = new StmtAST();
    ast->tag = StmtAST::EXP;
    $$ = ast;
  } | Block{
    auto ast = new StmtAST();
    ast->tag = StmtAST::BLOCK;
    ast->block = unique_ptr<BlockAST>((BlockAST*)$1);
    $$ = ast;
  } | IF '(' Exp ')' Stmt {
    auto ast = new StmtAST();
    ast->tag = StmtAST::IF;
    ast->exp = unique_ptr<ExpAST>((ExpAST*)$3);
    ast->if_stmt = unique_ptr<StmtAST>((StmtAST*)$5);
    $$ = ast;
  } | IF '(' Exp ')' Stmt ELSE Stmt {
    auto ast = new StmtAST();
    ast->tag = StmtAST::IF;
    ast->exp = unique_ptr<ExpAST>((ExpAST*)$3);
    ast->if_stmt = unique_ptr<StmtAST>((StmtAST*)$5);
    ast->else_stmt = unique_ptr<StmtAST>((StmtAST*)$7);
    $$ = ast;
  } | WHILE '(' Exp ')' Stmt {
    auto ast = new StmtAST();
    ast->tag = StmtAST::WHILE;
    ast->exp = unique_ptr<ExpAST>((ExpAST*)$3);
    ast->while_stmt = unique_ptr<StmtAST>((StmtAST*)$5);
    $$ = ast;
  }
  ;

Stmt
  : BREAK {
    auto ast = new StmtAST();
    ast->tag = StmtAST::BREAK;
    $$ = ast;
  } | CONTINUE {
    auto ast = new StmtAST();
    ast->tag = StmtAST::CONTINUE;
    $$ = ast;
  }
  ;


Exp
  : LOrExp {
    // std::cout<<"LOrExp"<<std::endl;
    auto ast = new ExpAST();
    ast->l_or_exp = unique_ptr<LOrExpAST>((LOrExpAST *)$1);
    $$ = ast;
  }
  ;
LVal
  : IDENT {

    auto ast = new LValAST();
    ast->ident = *unique_ptr<std::string>($1);
    std::cout<<ast->ident<<std::endl;

    $$ = ast;
  } | IDENT LvalArrayIndexList {
    auto ast = new LValAST();
    unique_ptr<LvalArrayIndexListAST> p((LvalArrayIndexListAST *)$2);
    ast->tag = LValAST::ARRAY;
    ast->ident = *unique_ptr<string>($1);
    for(auto &i : p->exp_list){
        ast->exp_list.emplace_back(i.release());
    }
    $$ = ast;
  }
  ;

LvalArrayIndexList
  : '[' Exp ']' {
      auto ast = new LvalArrayIndexListAST();
      ast->exp_list.emplace_back((ExpAST *)$2);
      $$ = ast;
    } | LvalArrayIndexList '[' Exp ']' {
      auto ast = (LvalArrayIndexListAST *)$1;
      ast->exp_list.emplace_back((ExpAST *)$3);
      $$ = ast;
    }
    ;
    
PrimaryExp
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST();
    ast->tag = PrimaryExpAST::EXP;
    
    ast->exp =  unique_ptr<ExpAST>((ExpAST *)$2);
    $$ = ast;
  } | Number {
    auto ast = new PrimaryExpAST();
    ast->tag = PrimaryExpAST::NUMBER;
    ast->number = $1;
    $$ = ast;
  } | LVal {
    // std::cout<<"LVal"<<std::endl;
    auto ast = new PrimaryExpAST();
    ast->tag = PrimaryExpAST::LVAL;
    ast->lval = unique_ptr<LValAST>((LValAST *)$1);
    $$ = ast;
  
  }
  ;

Number
  : INT_CONST {
    std::cout<<$1;
    $$ = $1;
  }
  ;

UnaryExp
  : PrimaryExp {

    auto ast = new UnaryExpAST();
    ast->tag = UnaryExpAST::PRIMARY;
    ast->primary_exp = unique_ptr<PrimaryExpAST>((PrimaryExpAST *)$1);
    $$ = ast;

  } | UnaryOp UnaryExp{

    auto ast = new UnaryExpAST();
    ast->tag = UnaryExpAST::UNARY;
    ast->op = $1;
    ast->unary_exp = unique_ptr<UnaryExpAST>((UnaryExpAST *)$2);
    $$ = ast;

  }  | IDENT '(' ')' {

    auto ast = new UnaryExpAST();
    ast->tag = UnaryExpAST::FUN;
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;

  } | IDENT '(' FuncRParams ')' {

    auto ast = new UnaryExpAST();
    ast->tag = UnaryExpAST::FUN;
    ast->ident = *unique_ptr<string>($1);
    ast->func_params = unique_ptr<FuncRParamsAST>((FuncRParamsAST *)$3);
    $$ = ast;

  }
  ;
UnaryOp 
  : '+' {
    $$ = '+';
  } | '-' {
    $$ = '-';
  } | '!' {
    $$ = '!';
  }
  ;
FuncRParams
  : Exp {

    auto ast = new FuncRParamsAST();
    ast->exps.push_back(unique_ptr<ExpAST>((ExpAST *)$1));
    $$ = ast;

  } | Exp ',' FuncRParams {
    
    auto ast = new FuncRParamsAST();
    ast->exps.push_back(unique_ptr<ExpAST>((ExpAST *)$1));
    auto rec = unique_ptr<FuncRParamsAST>((FuncRParamsAST *)$3);

    for(auto &i : rec->exps){

      ast->exps.push_back(std::move(i));

    }
    rec->exps.clear();
    
    $$ = ast;

  }
  ;
MulExp
  : UnaryExp{

    auto ast = new MulExpAST();
    ast->tag = MulExpAST::UNARY;
    ast->unary_exp = unique_ptr<UnaryExpAST>((UnaryExpAST *)$1);
    $$ = ast;

  } | MulExp MULOp UnaryExp{

    auto ast = new MulExpAST();
    ast->tag = MulExpAST::MUL_UNARY;

    ast->mul_exp2 = unique_ptr<MulExpAST>((MulExpAST *)$1);
    ast->unary_exp2 = unique_ptr<UnaryExpAST>((UnaryExpAST *)$3);
    
    ast->op = $2;

    $$ = ast;
  } 
  ;
MULOp 
  : '*' {
    $$ = '*';
  } | '/' {
    $$ = '/';
  } | '%' {
    $$ = '%';
  }
  ;
AddExp 
  : MulExp {
    auto ast = new AddExpAST();
    ast->tag = AddExpAST::MUL;
    ast->mul_exp = unique_ptr<MulExpAST>((MulExpAST *)$1);
    $$ = ast;
  } | AddExp AddOp MulExp {
    auto ast = new AddExpAST();

    ast->tag = AddExpAST::ADD_MUL;
    ast->add_exp2 = unique_ptr<AddExpAST>((AddExpAST *)$1);
    ast->mul_exp2 = unique_ptr<MulExpAST>((MulExpAST *)$3);
    ast->op = $2;

    $$ = ast;
  }
  ;
AddOp 
  : '+' {
    $$ = '+';
  } | '-' {
    $$ = '-';
  }
  ;

RelExp 
  : AddExp{
    auto ast = new RelExpAST();
    ast->tag = RelExpAST::ADD;
    ast->add_exp = unique_ptr<AddExpAST>((AddExpAST *)$1);
    $$ = ast;
  } |  RelExp '<' AddExp{

    auto ast = new RelExpAST();
    ast->tag = RelExpAST::REL_ADD;
    ast->rel_exp2 = unique_ptr<RelExpAST>((RelExpAST *)$1);
    ast->add_exp2 = unique_ptr<AddExpAST>((AddExpAST *)$3);
    // ast->op= "<"
    strcpy(ast->op,"<");
    $$ = ast;

  } |  RelExp '>' AddExp{

    auto ast = new RelExpAST();
    ast->tag = RelExpAST::REL_ADD;
    ast->rel_exp2 = unique_ptr<RelExpAST>((RelExpAST *)$1);
    ast->add_exp2 = unique_ptr<AddExpAST>((AddExpAST *)$3);
    strcpy(ast->op,">");
    $$ = ast;

  } |  RelExp LEQ AddExp{

    auto ast = new RelExpAST();
    ast->tag = RelExpAST::REL_ADD;
    ast->rel_exp2 = unique_ptr<RelExpAST>((RelExpAST *)$1);
    ast->add_exp2 = unique_ptr<AddExpAST>((AddExpAST *)$3);
    strcpy(ast->op,"<=");
    $$ = ast;

  } | RelExp BGE AddExp{

    auto ast = new RelExpAST();
    ast->tag = RelExpAST::REL_ADD;
    ast->rel_exp2 = unique_ptr<RelExpAST>((RelExpAST *)$1);
    ast->add_exp2 = unique_ptr<AddExpAST>((AddExpAST *)$3);
    strcpy(ast->op,">=");
    $$ = ast;

  }
  ;

EqExp 
  : RelExp{

    auto ast = new EqExpAST();
    ast->tag = EqExpAST::REL;
    ast->rel_exp = unique_ptr<RelExpAST>((RelExpAST *)$1);
    $$ = ast;

  } | EqExp EQ RelExp{

    auto ast = new EqExpAST();
    ast->tag = EqExpAST::EQ_REL;
    ast->eq_exp2 = unique_ptr<EqExpAST>((EqExpAST *)$1);
    ast->rel_exp2 = unique_ptr<RelExpAST>((RelExpAST *)$3);
    ast->op = '=';
    $$ = ast;

  } | EqExp NE RelExp{

    auto ast = new EqExpAST();
    ast->tag = EqExpAST::EQ_REL;
    ast->eq_exp2 = unique_ptr<EqExpAST>((EqExpAST *)$1);
    ast->rel_exp2 = unique_ptr<RelExpAST>((RelExpAST *)$3);
    ast->op = '!';
    $$ = ast;

  }
  ;

LAndExp
  : EqExp {

    auto ast = new LAndExpAST();
    ast->tag = LAndExpAST::EQ;
    ast->eq_exp = unique_ptr<EqExpAST>((EqExpAST *)$1);
    $$ = ast;

  } | LAndExp AND EqExp{

    auto ast = new LAndExpAST();
    ast->tag = LAndExpAST::EQ_AND;
    ast->l_and_exp2= unique_ptr<LAndExpAST>((LAndExpAST *)$1);
    ast->eq_exp2 = unique_ptr<EqExpAST>((EqExpAST *)$3);
    $$ = ast;

  }

LOrExp
  : LAndExp {

    auto ast = new LOrExpAST();
    ast->tag = LOrExpAST::AND;
    ast->l_and_exp = unique_ptr<LAndExpAST>((LAndExpAST *)$1);
    $$ = ast;

  } |  LOrExp OR LAndExp {

    auto ast = new LOrExpAST();
    ast->tag = LOrExpAST::OR_AND;
    ast->l_or_exp2= unique_ptr<LOrExpAST>((LOrExpAST *)$1);
    ast->l_and_exp2 = unique_ptr<LAndExpAST>((LAndExpAST *)$3);
    $$ = ast;

  }
ConstExp
  : Exp {
    std::cout<<"Exp"<<std::endl;
    auto ast = new ConstExpAST();
    ast->exp = unique_ptr<ExpAST>((ExpAST *)$1);
    $$ = ast;
  }
  ;
%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "yacc error: " << s << endl;
  // ast->Dump();
}


