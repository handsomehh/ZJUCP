#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <variant>
#include <iostream>
#include <cassert>
#include <cmath>
// #include <variant>
using namespace std;

// LabelCounter的作用是生成一个Label在Koopa IR中的名称
// 比方说，在if-else语句里面，可能会有多个if嵌套的情况
// 那么我们就要依次生成 %then1 %then2 ... 表示不同的跳转Label
class LabelCounter{
    private:
        std::unordered_map<std::string, int> map;
    public:
        std::string get_label_name(const std::string &name){
            auto it = map.find(name);
            if (it == map.end()){
                // 如果当前label是第一次出现，则将它加入map中
                map.insert(std::make_pair(name, 1));
                return name + "1";
            }
            else{
                return name + std::to_string(++it->second);
            }
        }

        void reset(){
            map.clear();
        }
        
};

// VarCounter的作用是生成一个variable在Koopa IR中的名称
// 例如，变量a在不同作用域中出现，那么它在IR中的名称分别是@a1,@a2...
class VarCounter{
    private:
        std::unordered_map<std::string, int> map;
    public:
        std::string get_var_name(const std::string &name){
            auto it = map.find(name);
            if (it == map.end()){
                // 如果当前label是第一次出现，则将它加入map中
                map.insert(std::make_pair(name, 1));
                return '@' + name + "1";
            }
            else{
                return '@' + name + std::to_string(++it->second);
            }
        }

        void reset(){
            map.clear();
        }
        
};

/*  临时变量名称计数器
    和前两者不同，该计数器生成的是那些，没有在代码中显式存在，用来存储临时变量的寄存器的名称
            c                   koopa ir
    例如 if (a == 9) ... =>  %1 = eq 9, 9
                            br %1, %then1, %end1
    这里的的%1就是该部分生成的
*/
class Counter{
  public:
    int count = 0;
    std::string Get_count(){
        int temp = count++;
        std::string name = "%"+std::to_string(temp);
        return name;
    }
    void Reset(){
        count = 0;
    }
};

class IntVar{
    public:
        int value;
        IntVar(int in):value(in){};
        IntVar():value(0){};
};

class ConstInt{
    public:
        int value;
        ConstInt(int in):value(in){};
        ConstInt():value(0){};
};

class Symbol{
    public:
        std::string ir_name; // 表示该symbol在IR中的名称，例如变量a，在IR中的名称可以是@a1, @a2...
        std::variant<IntVar, ConstInt> value; // 表示该symbol的值
        enum TYPE {INT,CONST,UNKNOWN}; // 表示该symbol的类型
        TYPE tag;
        Symbol(std::string ir_name_, int value_, TYPE tag_){
            ir_name = ir_name_;
            tag = tag_;
            if (tag == Symbol::INT){
                IntVar in(value_);
                value = in;
            }else if(tag == Symbol::CONST){
                ConstInt in(value_);
                value = in;
            }
        }
        void Print(){
            std::cout<<"  tag:";
            if (tag == Symbol::INT){
                std::cout<<"int "<< std::get<IntVar>(value).value<<std::endl;
                // IntVar in(value_);
                // value = in;
            }else if(tag == Symbol::CONST){
                std::cout<<"const "<< std::get<ConstInt>(value).value<<std::endl;
                // ConstInt in(value_);
                // value = in;
            }
        }
};
class SymbolTable{
    public:
    /*var*/
    std::unordered_map<std::string,Symbol*> map;
    enum TYPE {INT,CONST};
    /*fun*/
    int insert(const std::string &name, const std::string &ir_name, int value, TYPE mode){
        if(is_exist(name)){
            std::cout<<"var already exist :"<<name<<std::endl;
            return 0;
        }
        if(mode == SymbolTable::INT){
            Symbol* symbol = new Symbol(ir_name, value,Symbol::INT);
            map.insert({name,symbol});
            return 1;
        }else if(mode == SymbolTable::CONST){
            Symbol* symbol = new Symbol(ir_name, value, Symbol::CONST);
            map.insert({name,symbol});
            return 1;
        }else{
            std::cout<<"error input mode\n";
            return 0;
        }
    }
    int insert(const std::string &name, const std::string &ir_name, TYPE mode){
        if(is_exist(name)){
            std::cout<<"var already exist :"<<name<<std::endl;
            return 0;
        }
        if(mode == SymbolTable::INT){
            Symbol* symbol = new Symbol(ir_name, 0, Symbol::INT); // 如果定义的时候没有初值，默认初始化为0
            map.insert({name,symbol});
            return 1;
        }else if(mode == SymbolTable::CONST){
            std::cout<<"const should be initial,but not :"<<name<<std::endl;
            return 0;
        }else{
            std::cout<<"error input mode\n";
            return 0;
        }
    }

    bool Update(const std::string &name,int value){
        if(!is_exist(name)){
            std::cout<<"not found "<<name<<std::endl;
            return false;
        }
        auto res = map.find(name);
        if (res->second == nullptr){
            std::cout << "SysTable.h:168 -- shouldn't be nullptr" << endl;
            // map.insert({name,res});
        }else{
            std::get<IntVar>(res->second->value).value = value;
        }
        return true;
    }

    bool is_exist(const std::string &name){
        return map.find(name) != map.end();
    }

    int Get_value(const std::string &name){
        if(!is_exist(name)){
            return -1;
        }else{
            auto res = map.find(name);
            if(std::holds_alternative<IntVar>(res->second->value)){
                return std::get<IntVar>(res->second->value).value;
            }else{
                return std::get<ConstInt>(res->second->value).value;
            }
            // map.find(name)->value
        }
    }

    // 获取符号表中某个变量的类型
    Symbol::TYPE Get_type(const std::string &name){
        if(!is_exist(name)){
            return Symbol::UNKNOWN;
        }else{
            auto res = map.find(name);
            return res->second->tag;
            // map.find(name)->value
        }
    }

    // 查找symbol table中变量name的IR名称并返回
    std::string Get_ir_name(const std::string &name){
        if(!is_exist(name)){
            return "";
        }else{
            auto res = map.find(name);
            return res->second->ir_name;
        }
    }

    void Print(){
        for(auto i : map){
            std::cout<<"name:"<<i.first<<std::endl;
            // std::cout<<"value"<<i->second<<std::endl;
            if(i.second){
                i.second->Print();
            }else{
                std::cout<<"have no value"<<std::endl;
            }
        }
    }

};

class SymbolTableStack{
private:
    std::deque<std::unique_ptr<SymbolTable>> symbol_table_stack;

    Counter* mycount = new Counter();
    LabelCounter* label_counter = new LabelCounter();
    VarCounter* var_counter = new VarCounter();

public:
    // 插入一张符号表
    void alloc(){
        symbol_table_stack.emplace_back(new SymbolTable());
    }

    // 弹出一张符号表
    void quit(){
        symbol_table_stack.pop_back();
    }

    // 判断符号表栈中是否存在某个元素
    // ret: int，表示该元素存在于从后往前的第几张表中，0表示不存在
    int is_exist(const std::string &name){
        int tb_id = symbol_table_stack.size();
        
        // 遍历所有符号表，只要在某一张符号表中存在即可
        for (auto rit = symbol_table_stack.rbegin(); rit != symbol_table_stack.rend(); ++rit){
            const auto& tb = *rit;
            if (tb->is_exist(name)) break;
            tb_id--;
        }

        return tb_id;
    }



    // 向符号表栈（最顶端的符号表）插入一个变量，带初始值
    int insert(const std::string &name, const std::string &ir_name, int value, SymbolTable::TYPE mode){
        return symbol_table_stack.back()->insert(name, ir_name, value, mode);
    }

    // 向符号表栈（最顶端的符号表）插入一个变量，不带初始值
    int insert(const std::string &name, const std::string &ir_name, SymbolTable::TYPE mode){
        return symbol_table_stack.back()->insert(name, ir_name, mode);
    }

    // 更改符号表栈（最近作用域对应的的符号表）中的变量值
    // ret: 修改成功，返回true；修改不成功（变量不存在），返回false
    bool Update(const std::string &name,int value){
        for (auto rit = symbol_table_stack.rbegin(); rit != symbol_table_stack.rend(); ++rit){
            const auto& tb = *rit;
            if (tb->is_exist(name)){
                return tb->Update(name, value);
            }
        }
        return false;
    }

    // 按照最上层符号表，生成临时变量名
    string Get_count(){
        return mycount->Get_count();
    }

    // 重记临时变量名生成器
    void Reset_count(){
        mycount->Reset();
    }

    // 返回一个Label在Koopa IR中的名称
    // 比方说，在if-else语句里面，可能会有多个if嵌套的情况
    // 那么我们就要依次生成 %then1 %then2 ... 表示不同的跳转Label
    string Get_label_name(const std::string &name){
        return label_counter->get_label_name(name);
    }

    // 返回一个变量或者常量在Koopa IR中的名称
    string Get_var_name(const std::string &name){
        return var_counter->get_var_name(name);
    }

    // 获取name变量在最近一个作用域的值
    int Get_value(const std::string &name){
        bool is_exist = false;
        int val = -1;
        for (auto rit = symbol_table_stack.rbegin(); rit != symbol_table_stack.rend(); ++rit){
            const auto& tb = *rit;
            int x = tb->Get_value(name);
            if (x != -1){
                is_exist = true;
                val = x;
                break;
            }
        }

        if (is_exist){
            return val;
        }
        else{
            std::cout<<"not found :"<<name<<std::endl;
        }
    }

    // 返回name变量在最近的一个作用域的类型：变量 or 常量
    Symbol::TYPE Get_type(const std::string &name){
        bool is_exist = false;
        Symbol::TYPE val = Symbol::UNKNOWN;
        for (auto rit = symbol_table_stack.rbegin(); rit != symbol_table_stack.rend(); ++rit){
            const auto& tb = *rit;
            Symbol::TYPE x = tb->Get_type(name);
            if (x != Symbol::UNKNOWN){
                is_exist = true;
                val = x;
                break;
            }
        }

        if (is_exist){
            return val;
        }
        else{
            std::cout<<"not found :"<<name<<std::endl;
            return val;
        }
    }

    // 获取name变量在最近一个作用域的IR名称
    std::string Get_ir_name(const std::string &name){
        bool is_exist = false;
        std::string ir_name = "";
        for (auto rit = symbol_table_stack.rbegin(); rit != symbol_table_stack.rend(); ++rit){
            const auto& tb = *rit;
            std::string x = tb->Get_ir_name(name);
            if (x != ""){
                is_exist = true;
                ir_name = x;
                break;
            }
        }

        if (is_exist){
            return ir_name;
        }
        else{
            std::cout<<"not found :"<< name <<std::endl;
        }
    }

    // 获取symbol table stack中符号表的张数
    int size(){
        return symbol_table_stack.size();
    }
};