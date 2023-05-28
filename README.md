<div class="cover" style="page-break-after:always;font-family:方正公文仿宋;width:100%;height:100%;border:none;margin: 0 auto;text-align:center;">
    <div style="width:60%;margin: 0 auto;height:0;padding-bottom:10%;">
        </br>
        <img src="./pic/校名-黑色.svg" alt="校名" style="width:100%;"/>
    </div>
    </br></br></br></br></br>
    <div style="width:60%;margin: 0 auto;height:0;padding-bottom:40%;">
        <img src="./pic/校徽-黑色.svg" alt="校徽" style="width:100%;"/>
	</div>
    </br></br></br></br></br></br></br></br>
    <span style="font-family:华文黑体Bold;text-align:center;font-size:20pt;margin: 10pt auto;line-height:30pt;">编译原理</span>
    <p style="text-align:center;font-size:14pt;margin: 0 auto">课程实验报告 </p>
    </br>
    </br>
    <table style="border:none;text-align:center;width:72%;font-family:仿宋;font-size:14px; margin: 0 auto;">
    <tbody style="font-family:方正公文仿宋;font-size:12pt;">
        <tr style="font-weight:normal;"> 
    		<td style="width:20%;text-align:right;">题　　目</td>
    		<td style="width:2%">：</td> 
    		<td style="width:40%;font-weight:normal;border-bottom: 1px solid;text-align:center;font-family:华文仿宋"> 编译原理project</td>     </tr>
    	<tr style="font-weight:normal;"> 
    		<td style="width:20%;text-align:right;">授课教师</td>
    		<td style="width:2%">：</td> 
    		<td style="width:40%;font-weight:normal;border-bottom: 1px solid;text-align:center;font-family:华文仿宋">李莹 </td>     </tr>
    	<tr style="font-weight:normal;"> 
    		<td style="width:20%;text-align:right;">小组成员1</td>
    		<td style="width:%">：</td> 
    		<td style="width:40%;font-weight:normal;border-bottom: 1px solid;text-align:center;font-family:华文仿宋"> 潘韬-3200105354</td>     </tr>
    	<tr style="font-weight:normal;"> 
    		<td style="width:20%;text-align:right;">小组成员2</td>
    		<td style="width:%">：</td> 
    		<td style="width:40%;font-weight:normal;border-bottom: 1px solid;text-align:center;font-family:华文仿宋"> 周轶潇-3200103645</td>     </tr>
        <tr style="font-weight:normal;"> 
    		<td style="width:20%;text-align:right;">小组成员3</td>
    		<td style="width:%">：</td> 
    		<td style="width:40%;font-weight:normal;border-bottom: 1px solid;text-align:center;font-family:华文仿宋"> 韩恺荣-3200105385</td>     </tr>
    	<tr style="font-weight:normal;"> 
    		<td style="width:20%;text-align:right;">日　　期</td>
    		<td style="width:2%">：</td> 
    		<td style="width:40%;font-weight:normal;border-bottom: 1px solid;text-align:center;font-family:华文仿宋">2023/05/27</td>     </tr>
    </tbody>              
    </table>
</div>





<!-- 注释语句：导出PDF时会在这里分页 -->

# 一、编译器概述

我们实现了一个可以将 SysY 语言编译到 RISC-V 汇编的编译器。SysY 语言是C语言的子集，而编译器将生成`RV32IM`范围内的 RISC-V 汇编。该编译器使用的中间表示是 Koopa IR，它先将 SysY 源程序翻译成 Koopa IR，再将 Koopa IR 翻译成 RISC-V 汇编。

由于我们参考了[北大编译实践在线文档](https://pku-minic.github.io/online-doc/#/)，我们的编译器需要在docker环境下才能正常编译运行。

##### 获取镜像

在系统的命令行中执行:

```shell
docker pull maxxing/compiler-dev
```

如果你使用的是 Linux 系统, 则上述命令可能需要 `sudo` 才可正常执行.

##### 编译运行

在运行我们的编译器时，指定`-koopa`和`-riscv`选项可分别生成 Koopa IR 和 RISC-V 汇编。进入项目文件夹下，执行如下指令：

```shell
docker run -it --rm -v 项目目录:/root/compiler maxxing/compiler-dev make -C ./compiler
```

此时可以在工程文件夹中看到名为`build`的子文件夹生成。

```shell
./build/compiler -koopa SysY文件路径 -o KoopaIR文件路径
./build/compiler -riscv SysY文件路径 -o RISC-V文件路径
```

通过上面两行命令即可通过我们的编译器将输入的`.sysy程序`编译为`Koopa IR`或`RISC-V`汇编.

# 二、编译器设计与实现

## 2.1 词法分析模块

词法分析模块负责将SysY源代码分析为token流，去除对编译过程无用的注释、空行、空白等，只留下有意义的tokens.

### 2.1.1 设计思路

词法分析器是编译器的第一个阶段，用于将源代码分解为各个词法单元（tokens）。本设计分析旨在描述实现SysY语言的词法分析器所采用的设计思路。

本词法分析器使用了Flex工具（快速词法分析器生成器）来定义词法规则并生成词法分析器代码。同时，为了与语法分析器（Bison）协同工作，Flex代码包含了Bison生成的头文件（sysy.tab.hpp）。

我们对不同的需要匹配的情况书写了正则表达式，并在yacc文件中针对lex返回的匹配关键字进行分析，来达到交互的效果。

### 2.1.2 具体实现

#### 空白符和注释

- `WhiteSpace`：匹配空格、制表符、换行符和回车符。
- `LineComment`：匹配以"//"开头的行注释。
- `BlockComment`：匹配/* ... */格式的块注释。

#### 标识符

- `Identifier`：匹配标识符，以字母或下划线开头，后跟字母、数字或下划线。

#### 整数字面量

- `Decimal`：匹配十进制整数字面量，以非零数字开头，后跟数字。
- `Octal`：匹配八进制整数字面量，以0开头，后跟0-7的数字。
- `Hexadecimal`：匹配十六进制整数字面量，以0x或0X开头，后跟0-9或a-f或A-F的数字。

#### 关键字和运算符

- 关键字：匹配SysY语言中的关键字，如"int"、"return"、"const"、"if"等。
- 运算符：匹配SysY语言中的运算符，如"<="、">="、"=="、"!="、"&&"、"||"等。

#### 其他字符

- `.`：匹配任意单个字符。

#### 词法分析器的工作流程

词法分析器根据定义的词法规则对输入的SysY源代码进行扫描，识别和生成各个词法单元（tokens）。工作流程如下：

1. 初始时，词法分析器从输入源代码中读取字符流。
2. 通过正则表达式模式匹配，词法分析器将字符流划分为词法单元。
3. 对于每个匹配的词法单元，根据对应的词法规则执行相应的操作。
4. 如果没有匹配的词法规则，将报告词法错误。
5. 重复步骤2-4，直到扫描完整个输入源代码。
6. 词法分析器将生成的词法单元


## 2.2 语法分析模块

语法分析模块负责将token流分析为一棵抽象语法树。它能确定一个SysY源程序的语法结构，能够检测出SysY源程序中的语法错误。 语法分析模块从词法分析模块获得token流，并验证这个token流是否可以由文法生成。语法分析模块会构造一棵语法分析树，并把它传递给编译器的其他部分进一步处理，在构建语法分析树的过程中，就验证了这个token流是否符合文法。

### 2.2.1 设计思路

- 定义语法规则：根据给定的语法规则，使用 BNF 形式定义不同的语法规则和终结符。
- 定义语义动作：在语法规则中，添加相应的语义动作来构建抽象语法树（AST）和执行特定的操作。
- 集成词法分析器：通过调用词法分析器（lexer）获取输入的标记流（tokens）。
- 构建抽象语法树：在语义动作中，根据语法规则和输入的标记流，构建相应的抽象语法树。
- 处理语法错误：在解析过程中，对于不符合语法规则的输入，通过错误处理函数进行适当的处理。

### 2.2.2 具体实现

下面对不同的代码段和其对应的功能进行具体实现分析。

#### （1）程序解析入口与变量定义赋值模块

- CompUnit

  `CompUnit` 是起始符号，表示整个编译单元。它由一个或多个 `FuncDef` 组成。

- Decl

  `Decl` 表示声明语句，可以是 `ConstDecl` 或 `VarDecl`。

  - `ConstDecl` 表示常量声明，由关键字 "const"、`BType`（基本类型）和一个或多个 `ConstDef` 组成。
  - `VarDecl` 表示变量声明，由 `BType` 和一个或多个 `VarDef` 组成。

- BType

  `BType` 表示基本类型，可以是 "int" 或 "void"。

- ConstDef

  `ConstDef` 表示常量定义，由标识符（`IDENT`）和一个常量初始化值 `ConstInitVal` 组成。

- ConstInitVal

  `ConstInitVal` 表示常量的初始值，它是一个常量表达式 `ConstExp`。

- VarDef

  `VarDef` 表示变量定义，可以是标识符（`IDENT`）或标识符加初始化值（`IDENT '=' InitVal`）。

- InitVal

  `InitVal` 表示变量的初始值，它是一个表达式 `Exp`。

- FuncDef

  `FuncDef` 表示函数定义，由函数类型 `FuncType`、标识符（函数名）、函数参数列表 `FuncFParams` 和函数体 `Block` 组成。

#### （2）基本块和语句模块

- Block

  `Block` 表示代码块，可以包含一系列的 `BlockItem`。

- Stmt

  `Stmt` 表示语句，可以是赋值语句、表达式语句、代码块或返回语句。

  - 赋值语句（`Exp ASSIGN Exp`）表示将一个表达式的值赋给另一个表达式。
  - 表达式语句（`Exp SEMI`）表示单独的表达式语句。
  - 代码块（`Block`）表示一个代码块。
  - 返回语句（`RETURN Exp SEMI`）表示函数的返回语句。

- Exp

  `Exp` 表示表达式，可以是变量、常量、函数调用、运算表达式等。

  - 变量（`IDENT`）表示一个变量。
  - 常量（`ICONST`）表示一个整数常量。
  - 函数调用（`IDENT '(' Args ')'`）表示调用一个函数，其中 `Args` 是参数列表。
  - 运算表达式（例如加法、减法等）由操作符和两个操作数组成。

#### （3）运算模块

- LVal：

  解析标识符（IDENT），创建一个 `LValAST` 对象，并将标识符存储在 `ast->ident` 中。

- PrimaryExp

  解析基本表达式，可以是括号内的表达式、数字（Number）或标识符（LVal）。根据不同的情况，创建一个 `PrimaryExpAST` 对象，并将对应的表达式存储在 `ast->exp` 或数字存储在 `ast->number` 中。

- Number

  解析整数常量（INT_CONST），输出该整数并将其存储在 `$$` 中。

- UnaryExp

  解析一元表达式，可以是基本表达式（PrimaryExp）、一元操作符（UnaryOp）后跟一元表达式、函数调用或函数调用带参数。根据不同的情况，创建一个 `UnaryExpAST` 对象，并存储对应的信息。


- UnaryOp

  解析一元操作符，可以是正号（'+'）、负号（'-'）或逻辑非（'!'）。将对应的操作符存储在 `$$` 中。

- FuncRParams

  解析函数调用时的参数列表，可以是单个表达式（Exp）或逗号分隔的表达式列表。创建一个 `FuncRParamsAST` 对象，并将参数表达式存储在 `ast->exps` 中。

- MulExp

  解析乘法表达式，可以是一元表达式（UnaryExp）或乘法操作符（MULOp）后跟乘法表达式。根据不同的情况，创建一个 `MulExpAST` 对象，并存储对应的信息。

- MULOp

  解析乘法操作符，可以是乘号（'*'）、除号（'/'）或取模运算符（'%'）。将对应的操作符存储在 `$$` 中。

- AddExp

  解析加法表达式，可以是乘法表达式（MulExp）或加法操作符（AddOp）后跟加法表达式。根据不同的情况，创建一个 `AddExpAST` 对象，并存储对应的信息。

- AddOp

  解析加法操作符，可以是加号（'+'）或减号（'-'）。将对应的操作符存储在 `$$` 中。

- RelExp

  解析关系表达式，可以是加法表达式（AddExp）或关系操作符（'<', '>', '<=', '>='）后跟加法表达式。根据不同的情况...

#### （4）表达式模块

- EqExp 规则：

  - 第一个产生式：
    创建一个 EqExpAST 对象，将其标记设置为 REL，并将 RelExp 的解析结果存储在 `ast->rel_exp` 中。

  - 第二个产生式：
    创建一个 EqExpAST 对象，将其标记设置为 EQ_REL。将 EqExp 的解析结果存储在 `ast->eq_exp2` 中，将 RelExp 的解析结果存储在 `ast->rel_exp2` 中，并将操作符 '=' 存储在 `ast->op` 中。

  - 第三个产生式：
    创建一个 EqExpAST 对象，将其标记设置为 EQ_REL。将 EqExp 的解析结果存储在 `ast->eq_exp2` 中，将 RelExp 的解析结果存储在 `ast->rel_exp2` 中，并将操作符 '!' 存储在 `ast->op` 中。

- LAndExp 规则：
  - 第一个产生式：
    创建一个 LAndExpAST 对象，将其标记设置为 EQ，并将 EqExp 的解析结果存储在 `ast->eq_exp` 中。

  - 第二个产生式：
    创建一个 LAndExpAST 对象，将其标记设置为 EQ_AND。将 LAndExp 的解析结果存储在 `ast->l_and_exp2` 中，将 EqExp 的解析结果存储在 `ast->eq_exp2` 中。
- LOrExp 规则：

  - 第一个产生式：
    创建一个 LOrExpAST 对象，将其标记设置为 AND，并将 LAndExp 的解析结果存储在 `ast->l_and_exp` 中。

  - 第二个产生式：
    创建一个 LOrExpAST 对象，将其标记设置为 OR_AND。将 LOrExp 的解析结果存储在 `ast->l_or_exp2` 中，将 LAndExp 的解析结果存储在 `ast->l_and_exp2` 中。

- ConstExp 规则：

  - 产生式：
    创建一个 ConstExpAST 对象，将 Exp 的解析结果存储在 `ast->exp` 中，并输出 "Exp"。

### 2.2.3 语法范式描述

	CompUnit      ::= [CompUnit] (Decl | FuncDef);
	
	Decl          ::= ConstDecl | VarDecl;
	ConstDecl     ::= "const" BType ConstDef {"," ConstDef} ";";
	BType         ::= "int";
	ConstDef      ::= IDENT {"[" ConstExp "]"} "=" ConstInitVal;
	ConstInitVal  ::= ConstExp | "{" [ConstInitVal {"," ConstInitVal}] "}";
	VarDecl       ::= BType VarDef {"," VarDef} ";";
	VarDef        ::= IDENT {"[" ConstExp "]"}
					| IDENT {"[" ConstExp "]"} "=" InitVal;
	InitVal       ::= Exp | "{" [InitVal {"," InitVal}] "}";
	
	FuncDef       ::= FuncType IDENT "(" [FuncFParams] ")" Block;
	FuncType      ::= "void" | "int";
	FuncFParams   ::= FuncFParam {"," FuncFParam};
	FuncFParam    ::= BType IDENT ["[" "]" {"[" ConstExp "]"}];
	
	Block         ::= "{" {BlockItem} "}";
	BlockItem     ::= Decl | Stmt;
	Stmt          ::= LVal "=" Exp ";"
					| [Exp] ";"
					| Block
					| "if" "(" Exp ")" Stmt ["else" Stmt]
					| "while" "(" Exp ")" Stmt
					| "break" ";"
					| "continue" ";"
					| "return" [Exp] ";";
	
	Exp           ::= LOrExp;
	LVal          ::= IDENT {"[" Exp "]"};
	PrimaryExp    ::= "(" Exp ")" | LVal | Number;
	Number        ::= INT_CONST;
	UnaryExp      ::= PrimaryExp | IDENT "(" [FuncRParams] ")" | UnaryOp UnaryExp;
	UnaryOp       ::= "+" | "-" | "!";
	FuncRParams   ::= Exp {"," Exp};
	MulExp        ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
	AddExp        ::= MulExp | AddExp ("+" | "-") MulExp;
	RelExp        ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
	EqExp         ::= RelExp | EqExp ("==" | "!=") RelExp;
	LAndExp       ::= EqExp | LAndExp "&&" EqExp;
	LOrExp        ::= LAndExp | LOrExp "||" LAndExp;
	ConstExp      ::= Exp;


## 2.3 IR生成模块

我们的编译器在做完词法分析和语法分析的工作以后，下一步就是生成 IR(Intermediate Representation, 即中间表示)。IR是介于前端和后端中间的语言，我们的编译器通过这个模块将抽象语法树分析为Koopa这种中间表示，这样一来，后端只用获得Koopa，就可以生成目标代码。

### 2.3.1 设计思路

在词法分析阶段完成了抽象语法树（AST）的构建，可以参考代码文件 sysy.y。该文件定义了在每个产生式归约时执行的动作，即代码块。这些代码块大多数情况下返回一个指向 BaseAST 类型的指针，用于构建子树，并传递给上一层。通过连续的归约操作，最终得到了抽象语法树的根节点 CompUnitAST。

在 AST.cpp 文件中，定义了全局变量 KoopaString ks，用于管理 Koopa IR 字符串。从根节点 CompUnitAST 开始，遍历整个抽象语法树，并调用每个抽象语法树节点的 Dump 函数，将生成的 Koopa IR 添加到 KoopaString ks 中。

这样，通过以上的实现和过程，成功构建了抽象语法树，并将其转化为 Koopa IR 字符串形式。

### 2.3.2 具体实现

#### (1)类设计：

所有的函数都继承了纯虚基类函数。

```c++
class BaseAST
{
public:
    virtual ~BaseAST() = default;
    // virtual std::string Dump()const = 0;
};
```

#### (2)具体的类设计：

对于每一个类，我们的类都遵循类似的套路和模板：

```
class 类名 : public BaseAST
{
public:
	/* 类内枚举里量设计*/
    enum TYPE
    {
        /*枚举不同的类型情况 */
    };
    TYPE tag;
    
	/* 类内变量*/
    std::unique_ptr<AST子类>变量名;
	
	/* 类函数声明*/
    std::string Dump() const;
    int Get_value();
};
```

而下面则列举了一些会再类内出现的结构以及可能的函数名:

1. 类型控制：

```
enum TYPE{}
```

2. 变量包含

```
std::unique_ptr<AST子类>变量名1;
std::vector<std::unique_ptr<AST子类>> 变量名2;
```

std::unique_ptr< AST子类 >是再yacc中生成AST时，对应类型的结点可能指向的其他类型结点的类型枚举，而std::vector< std::unique_ptr< AST子类 >>则是对于EBNF文法中可重复模块的保存，如：

	Block         ::= "{" {BlockItem} "}";

3. 函数包含:

```
std::string Dump() const;
std::string Dump_Global() const;
int Get_value();
```

其中，Dump是再递归调用时，控制生成的IR的主要结构，而DumpGlobal则提供了生成全局变量的方法。
Get_value则是Exp的衍生类中特有的方法，用与直接得到一个表达式的对应数值，常用于常量生成过程。

#### (3)细节说明:


- `CompUnitAST` 类继承自 `BaseAST`，表示编译单元的抽象语法树节点。它包含以下成员：
  - `func_defs`：包含指向 `FuncDefAST` 的智能指针的向量，表示函数定义列表
  - `decls`：包含指向 `DeclAST` 的智能指针的向量，表示声明列表
  - `Dump()`：用于将节点信息转储为字符串的方法

- `FuncDefAST` 类继承自 `BaseAST`，表示函数定义的抽象语法树节点。它包含以下成员：
  - `params`：指向 `FuncFParamsAST` 的智能指针，表示函数参数列表
  - `func_type`：指向 `BTypeAST` 的智能指针，表示函数返回类型
  - `ident`：函数的标识符
  - `block`：指向 `BlockAST` 的智能指针，表示函数体
  - `Dump()`：用于将节点信息转储为字符串的方法

- `FuncFParamsAST` 类继承自 `BaseAST`，表示函数形参列表的抽象语法树节点。它包含以下成员：
  - `params`：包含指向 `FuncFParamAST` 的智能指针的向量，表示形参列表
  - `Dump()`：用于将节点信息转储为字符串的方法

- `FuncFParamAST` 类继承自 `BaseAST`，表示函数形参的抽象语法树节点。它包含以下成员：
  - `btype`：指向 `BTypeAST` 的智能指针，表示形参的类型
  - `ident`：形参的标识符
  - `Dump()`：用于将节点信息转储为字符串的方法

- `FuncTypeAST` 类继承自 `BaseAST`，表示函数类型的抽象语法树节点。它包含以下成员：
  - `tag`：枚举类型，表示函数类型的标签（INT 或 VOID）
  - `Dump()`：用于将节点信息转储为字符串的方法

- `BlockAST` 类继承自 `BaseAST`，表示代码块的抽象语法树节点。它包含以下成员：
  - `blockitem`：包含指向 `BlockItemAST` 的智能指针的向量，表示代码块的语句或声明列表
  - `Dump()`：用于将节点信息转储为字符串的方法

- `BlockItemAST` 类继承自 `BaseAST`，表示代码块中的语句或声明的抽象语法树节点。它包含以下成员：
  - `tag`：枚举类型，表示节点的类型（DECL 或 STMT）
  - `decl`：指向 `DeclAST` 的智能指针，表示声明
  - `stmt`：指向 `StmtAST` 的智能指针，表示语句
  - `Dump()`：用于将节点信息转储为字符串的方法

- `StmtAST` 类继承自 `BaseAST`，表示语句的抽象语法树节点。它包含以下成员：
  - `tag`：枚举类型，表示语句的类型（RETURN、ASSIGN、EXP、BLOCK、IF 或 WHILE）
  - `exp`：指向 `ExpAST` 的智能指针，表示表达式
  - `lval`：指向 `LValAST` 的智能指针，表示左值
  - `block`：指向 `BlockAST` 的智能指针，表示代码块
  - `if_stmt`：指向 `StmtAST` 的智能指针，表示 if 语句
  - `else_stmt`：指向 `StmtAST` 的智能指针，表示 else 语句
  - `while_stmt`：指向 `StmtAST` 的智能指针，表示 while 语句
  - `Dump()`：用于将节点信息转储为字符串的方法

- `ExpAST` 类继承自 `BaseAST`，表示表达式的抽象语法树节点。它包含以下成员：
  - `l_or_exp`：指向 `LOrExpAST` 的智能指针，表示逻辑或表达式
  - `Dump()`：用于将节点信息转储为字符串的方法
  - `Get_value()`：用于获取表达式的值的方法

- `LOrExpAST` 类继承自 `BaseAST`，表示逻辑或表达式的抽象语法树节点。它包含以下成员：
  - `tag`：枚举类型，表示节点的类型（AND 或 OR_AND）
  - `l_and_exp`：指向 `LAndExpAST` 的智能指针，表示逻辑与表达式
  - `l_or_exp2`：指向 `LOrExpAST` 的智能指针，表示逻辑或表达式（递归）
  - `l_and_exp2`：指向 `LAndExpAST` 的智能指针，表示逻辑与表达式
  - `Dump()`：用于将节点信息转储为字符串的方法
  - `Get_value()`：用于获取逻辑或表达式的值的方法

- `LAndExpAST` 类继承自 `BaseAST`，表示逻辑与表达式的抽象语法树节点。它包含以下成员：
  - `tag`：枚举类型，表示节点的类型（EQ 或 EQ_AND）
  - `eq_exp`：指向 `EqExpAST` 的智能指针，表示相等表达式
  - `l_and_exp2`：指向 `LAndExpAST` 的智能指针，表示逻辑与表达式（递归）
  - `eq_exp2`：指向 `EqExpAST` 的智能指针，表示相等表达式
  - `Dump()`：用于将节点信息转储为字符串的方法
  - `Get_value()`：用于获取逻辑与表达式的值的方法

- `EqExpAST` 类继承自 `BaseAST`，表示相等表达式的抽象语法树节点。它包含以下成员：
  - `tag`：枚举类型，表示节点的类型（REL 或 EQ_REL）
  - `rel_exp`：指向 `RelExpAST` 的智能指针，表示关系表达式
  - `eq_exp2`：指向 `EqExpAST` 的智能指针，表示相等表达式（递归）
  - `rel_exp2`：指向 `RelExpAST` 的智能指针，表示关系表达式
  - `op`：表示相等操作符（如"=="、"!="等）
  - `Dump()`：用于将节点信息转储为字符串的方法
  - `Get_value()`：用于获取相等表达式的值的方法

- `RelExpAST` 类继承自 `BaseAST`，表示关系表达式的抽象语法树节点。它包含以下成员：
  - `tag`：枚举类型，表示节点的类型（ADD 或 REL_ADD）
  - `add_exp`：指向 `AddExpAST` 的智能指针，表示加法表达式
  - `rel_exp2`：指向 `RelExpAST` 的智能指针，表示关系表达式（递归）
  - `add_exp2`：指向 `AddExpAST` 的智能指针，表示加法表达式
  - `op`：表示关系操作符（如"<"、">"、"<="、">="等）
  - `Dump()`：用于将节点信息转储为字符串的方法
  - `Get_value()`：用于获取关系表达式的值的方法

- `AddExpAST` 类继承自 `BaseAST`，表示加法表达式的抽象语法树节点。它包含以下成员：
  - `tag`：枚举类型，表示节点的类型（MUL 或 ADD_MUL）
  - `mul_exp`：指向 `MulExpAST` 的智能指针，表示乘法表达式
  - `add_exp2`：指向 `AddExpAST` 的智能指针，表示加法表达式（递归）
  - `mul_exp2`：指向 `MulExpAST` 的智能指针，表示乘法表达式
  - `op`：表示加法操作符（如"+"、"-"等）
  - `Dump()`：用于将节点信息转储为字符串的方法
  - `Get_value()`：用于获取加法表达式的值的方法

- `MulExpAST` 类继承自 `BaseAST`，表示乘法表达式的抽象语法树节点。它包含以下成员：
  - `tag`：枚举类型，表示节点的类型（UNARY 或 MUL_UNARY）
  - `unary_exp`：指向 `UnaryExpAST` 的智能指针，表示一元表达式
  - `mul_exp2`：指向 `MulExpAST` 的智能指针，表示乘法表达式（递归）
  - `unary_exp2`：指向 `UnaryExpAST` 的智能指针，表示一元表达式
  - `op`：表示乘法操作符（如"*"、"/"、"%"等）
  - `Dump()`：用于将节点信息转储为字符串的方法
  - `Get_value()`：用于获取乘法表达式的值的方法

- `UnaryExpAST` 类继承自 `BaseAST`，表示一元表达式的抽象语法树节点。它包含以下成员：
  - `tag`：枚举类型，表示节点的类型（FACTOR 或 UNARY_FACTOR）
  - `factor`：指向 `FactorAST` 的智能指针，表示因子
  - `op`：表示一元操作符（如"-"、"!"等）
  - `Dump()`：用于将节点信息转储为字符串的方法
  - `Get_value()`：用于获取一元表达式的值的方法

- `DeclAST` 类继承自 `BaseAST`，表示声明的抽象语法树节点。它包含以下成员：
  - `btype`：指向 `BTypeAST` 的智能指针，表示声明的类型
  - `ident`：标识符
  - `Dump()`：用于将节点信息转储为字符串的方法

- `BTypeAST` 类继承自 `BaseAST`，表示基本类型的抽象语法树节点。它包含以下成员：
  - `tag`：枚举类型，表示类型的标签（INT、FLOAT 或 CHAR）
  - `Dump()`：用于将节点信息转储为字符串的方法

- `ConstDefAST` 类继承自 `BaseAST`，表示常量定义的抽象语法树节点。它包含以下成员：
  - `ident`：常量的标识符
  - `const_init_val`：指向 `ConstInitValAST` 的智能指针，表示常量的初始值
  - `Dump()`：用于将节点信息转储为字符串的方法
  - `Dump_Global()`：用于将全局节点信息转储为字符串的方法
- `VarDefAST` 类继承自 `BaseAST`，表示变量定义的抽象语法树节点。它包含以下成员：
  - `ident`：变量的标识符
  - `init_val`：指向 `InitValAST` 的智能指针，表示变量的初始值
  - `Dump()`：用于将节点信息转储为字符串的方法
  - `Dump_Global()`：用于将全局节点信息转储为字符串的方法

- `InitValAST` 类继承自 `BaseAST`，表示初始化值的抽象语法树节点。它包含以下成员：
  - `exp`：指向 `ExpAST` 的智能指针，表示初始化表达式
  - `Get_value()`：用于获取初始化值的方法

- `ConstInitValAST` 类继承自 `BaseAST`，表示常量的初始化值的抽象语法树节点。它包含以下成员：
  - `const_exp`：指向 `ConstExpAST` 的智能指针，表示常量的初始化表达式
  - `Dump()`：用于将节点信息转储为字符串的方法

- `LValAST` 类继承自 `BaseAST`，表示左值（变量或常量）的抽象语法树节点。它包含以下成员：
  - `ident`：左值的标识符

- `ConstExpAST` 类继承自 `BaseAST`，表示常量表达式的抽象语法树节点。它包含以下成员：
  - `exp`：指向 `ExpAST` 的智能指针，表示常量表达式
  - `Get_value()`：用于获取常量表达式的值的方法

- `FuncRParamsAST` 类继承自 `BaseAST`，表示函数调用的实参列表的抽象语法树节点。它包含以下成员：
  - `exps`：包含指向 `ExpAST` 的智能指针的向量，表示实参列表
  - `Dump()`：用于将节点信息转储为字符串的方法

这些类用于构建抽象语法树，每个类都包含了相应节点的成员变量和方法，用于表示语法结构和操作节点。可以根据需要调用它们的方法来操作和处理抽象语法

## 2.4 代码生成模块

代码生成模块负责将上一步生成的KoopaIR转化为最终的RISC-V汇编指令，通过这个模块，程序进一步贴近机器，也达成了我们的终极目标。

### 2.4.1 设计思路

通过调用北大编译实践提供的`Koopa.h`文件，我们可以将前面几个模块生成的`KoopaIR`从`string`形式转化为内存形式，以便调用和解析。对于内存形式的`KoopaIR`，该模块将进行一个类`dfs`形式的遍历，从最外层的全局变量和函数层层深入，遍历的同时将每一步操作对应的`RISC-V`汇编指令写入`string`类型的变量`RiscvString`中，当遍历完成时，`RISC-V`汇编指令也生成完毕了。

### 2.4.2 具体实现

##### 2.4.2.1 从`KoopaIR`到`raw program`

由于之前的模块生成的`KoopaIR`是字符串形式的，这不利于我们后续对其进行解析。好在参考文档中提供了`Koopa.h`头文件，其中拥有现成的封装好的解析函数来帮助我们把字符串形式的`KoopaIR`转化为内存形式。我们只需要在`main`函数中调用这些函数，便可得到更方便后续处理的`raw program`。

具体实现如下：

```cpp
// in main.cpp
#include "koopa.h"
#include "Visit.h"

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
Visit(raw);

// 处理完成, 释放 raw program builder 占用的内存
// 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
// 所以不要在 raw program 处理完毕之前释放 builder
koopa_delete_raw_program_builder(builder);
```

其中, `raw program` 的结构层次如下：

- 最上层是 `koopa_raw_program_t`, 也就是 `Program`.
- 之下是全局变量定义列表和函数定义列表.
  - 在 `raw program` 中, 列表的类型是 `koopa_raw_slice_t`.
  - 本质上这是一个指针数组, 其中的 buffer 字段记录了指针数组的地址 (类型是 `const void **`), `len` 字段记录了指针数组的长度, `kind` 字段记录了数组元素是何种类型的指针
  - 在访问时, 可以通过 `slice.buffer[i]` 拿到列表元素的指针, 然后通过判断 `kind` 来决定把这个指针转换成什么类型.
- `koopa_raw_function_t `代表函数, 其中是基本块列表.
- `koopa_raw_basic_block_t `代表基本块, 其中是指令列表.
- `koopa_raw_value_t `代表全局变量, 或者基本块中的指令.

代码中的`Visit`函数就是我们用来处理`raw program`的工具，它在`Visit.[h|cpp]`中被定义和实现，下文将展开叙述。

##### 2.4.3.2 `StackAddressAllocator`类

为了方便转汇编代码时计算程序到底需要分配多少栈空间，我们为此封装了一个`StackAddressAllocator`类。类声明如下：

```cpp
// 类声明
class StackAddressAllocator { // 分配栈上的局部变量的地址
public:
    // 存储变量地址的哈希映射表
    std::unordered_map<koopa_raw_value_t, size_t> var_addr;
    // ReturnAddress: 函数中有call则为4，用于保存ra寄存器
    // AllocatedPara: 该函数调用的函数中，参数最多的那个，需要额外分配的第9,10……个参数的空间
    // StackSpace: 为这个函数的局部变量分配的栈空间
    // delta: 对齐后的栈帧长度
    size_t ReturnAddress;
    size_t AllocatedPara;
    size_t StackSpace;
    size_t delta;

    StackAddressAllocator();
    void clear();
    void alloc(koopa_raw_value_t value, size_t width = 4);
    void setReturnAddress();
    void setAllocatedPara(size_t a);
    bool exists(koopa_raw_value_t value);
    size_t getOffset(koopa_raw_value_t value);
    size_t getDelta();
};
```

下面介绍类中声明的方法：

###### StackAddressAllocator()

```cpp
StackAddressAllocator::StackAddressAllocator() {
    ReturnAddress = 0;
    AllocatedPara = 0;
    StackSpace = 0;
    delta = 0;
}
```

构造函数，在初始化时只需将成员变量全置为0即可。

###### clear()

```cpp
void StackAddressAllocator::clear() {
    var_addr.clear();
    ReturnAddress = AllocatedPara = StackSpace = delta = 0;
}
```

将成员变量恢复初始状态。

###### alloc(koopa_raw_value_t value, size_t width = 4)

```cpp
void StackAddressAllocator::alloc(koopa_raw_value_t value, size_t width) {
    var_addr[value] = StackSpace;
    StackSpace += width;
}
```

计算需要分配的栈空间，将`value`的栈地址记录在`var_addr`中，再将`value`的宽度加到`StackSpace`中，便于下一次计算。

###### setReturnAddress()

```cpp
void StackAddressAllocator::setReturnAddress() {
    ReturnAddress = 4;
}
```

当此方法被调用时，将`ReturnAddress`的值置为4.

###### setAllocatedPara(size_t a)

```cpp
void StackAddressAllocator::setAllocatedPara(size_t a) {
    AllocatedPara = std::max(AllocatedPara, a);
}
```

设置函数参数的栈空间，这个函数会在遍历函数列表时被调用，以遍历到的参数最多的函数为准分配空间。

###### exists(koopa_raw_value_t value)

```cpp
bool StackAddressAllocator::exists(koopa_raw_value_t value) {
    return var_addr.count(value) > 0;
}
```

判断字典`var_addr`是否为空。

###### getOffset(koopa_raw_value_t value)

```cpp
size_t StackAddressAllocator::getOffset(koopa_raw_value_t value) {
    return var_addr[value] + AllocatedPara;
}
```

返回`value`在栈中的偏移量，此偏移量为`value`入栈之前分配出去的栈空间+函数参数占用的栈空间。

###### getDelta()

```cpp
size_t StackAddressAllocator::getDelta() {
    size_t d = StackSpace + ReturnAddress + AllocatedPara;
    delta = d % 16 ? d + 16 - d % 16 : d;
    return delta;
}
```

返回`delta`。`delta`的值就是`StackSpace`、`ReturnAddress`、`AllocatedPara`三者的和，若结果未对齐16位，则对齐后再返回。

##### 2.4.2.3 `Visit()`的实现

对于`raw program`的处理，我们的`Visit`函数采用了一种类似深度优先搜索的算法来遍历整个`program tree`.在实现上，我们利用了cpp作为面向对象语言的多态的特点，定义了许多针对不同对象的`Visit`方法：

```cpp
void Visit(const koopa_raw_program_t &program);			  // 访问raw program
void Visit(const koopa_raw_slice_t &slice) ;			  // 访问全局变量定义列表和函数定义列表
void Visit(const koopa_raw_function_t &func);			  // 访问函数
void Visit(const koopa_raw_basic_block_t &bb);			  // 访问基本块
void Visit(const koopa_raw_value_t &value);				  // 访问指令/全局变量
void Visit(const koopa_raw_return_t &value);			  // 访问return指令
int  Visit(const koopa_raw_integer_t &value);			  // 访问integer指令
void Visit(const koopa_raw_binary_t &value);			  // 访问二元运算指令
void Visit(const koopa_raw_load_t &load);				  // 访问加载指令
void Visit(const koopa_raw_store_t &store);				  // 访问存储指令
void Visit(const koopa_raw_branch_t &branch);			  // 访问条件分支指令
void Visit(const koopa_raw_jump_t &jump);				  // 访问无条件跳转指令
void Visit(const koopa_raw_call_t &call);				  // 访问函数调用
void Visit(const koopa_raw_get_elem_ptr_t& get_elem_ptr); // 访问GetElemPtr指令
void Visit(const koopa_raw_get_ptr_t& get_ptr);			  // 访问GetPtr指令
```

它们之间配合的大体的实现框架如下图所示：

<img src=".\pic\Visit结构.png" alt="Visit结构" style="zoom:38%;" />

在每一次访问到最底层的指令后，我们就将相应的`RISC-V`汇编追加写入到全局变量`RiscvString`中。当我们将`program tree`遍历完成时，汇编代码的编写也完成了。

## 2.5 符号表模块

### 2.5.1. 设计思路

本实验中的符号表实现是基于哈希表的数据结构，用于存储编译器中的变量和函数信息。我们使用了三个不同的类来实现符号表的功能：`SymbolTable`、`FunctionTable`和`SymbolTableStack`。

### 2.5.1.1 `SymbolTable` 类

`SymbolTable` 类用于表示一个符号表，其中的 `map` 成员变量是一个无序哈希映射，用于存储变量名和对应的 `Symbol` 对象。`Symbol` 对象包含变量在IR中的名称、变量的值以及变量的类型。

`SymbolTable` 类提供了以下功能：

- `insert` 函数：用于向符号表中插入一个变量。该函数接受变量名、IR名称、初始值和变量类型作为参数，并将变量插入到哈希表中。
- `is_exist` 函数：用于判断一个变量是否存在于符号表中。
- `Update` 函数：用于更新符号表中的变量值。
- `Get_value` 函数：用于获取符号表中变量的值。
- `Get_type` 函数：用于获取符号表中变量的类型。

### 2.5.1.2 `FunctionTable` 类

`FunctionTable` 类用于表示函数表，其中的 `map` 成员变量是一个无序哈希映射，用于存储函数名和对应的函数类型。函数类型可以是整型或无返回值类型。

`FunctionTable` 类提供了以下功能：

- `insert` 函数：用于向函数表中插入一个函数。该函数接受函数名和函数类型作为参数，并将函数插入到哈希表中。
- `is_exist` 函数：用于判断一个函数是否存在于函数表中。
- `Get_Type` 函数：用于获取函数的返回类型。

### 2.5.1.3 `SymbolTableStack` 类

`SymbolTableStack` 类用于表示符号表的栈，它是由多个 `SymbolTable` 对象组成的。符号表栈可以用于管理不同作用域中的变量和函数。每当进入一个新的作用域时，都会创建一个新的符号表，并将其添加到符号表栈的顶部；当退出一个作用域时，会将该符号表从符号表栈中移除。

`SymbolTableStack` 类提供了以下功能：

- `alloc` 函数：用于在符号表栈中分配一个新的符号表，即进入一个新的作用域。
- `quit` 函数：用于退出当前作用域，即将符号表栈顶部的符号表弹出。
- `is_exist` 函数：用于判断一个变量是否存在于符号表栈中的任何一个符号表中。
- `is_exist_global` 函数：用于判断一个变量是否存在于全局符号表中。

# 三、编译器测试

测试编译器前，需要先执行相关操作生成可执行文件，sh命令如下：

```sh
./build/compiler -riscv 测试文件.c -o hello.S
clang hello.S -c -o hello.o -target riscv32-unknown-linux-elf -march=rv64im -mabi=ilp32
ld.lld hello.o -L$CDE_LIBRARY_PATH/riscv32 -lsysy -o hello
```

我们使用的是北大模板，所以，如果要本地测试，需要：

```sh
qemu-riscv32-static hello
```

为了执行tester，我们需要编写如下的shell文件

```sh
#!/bin/bash
qemu-riscv32-static hello
```

然后执行：

```sh
./tester   shell文件名
```

目前，我们通过了前两个测试，截图如下：
<img src=".\pic\2.png" alt="校名" style="width:100%;"/>
<img src=".\pic\1.png" alt="校名" style="width:100%;"/>

# 四、参考文献

1. 北大编译实践在线文档：https://pku-minic.github.io/online-doc/#/

2. 《Modern Compiler Implementation in C》