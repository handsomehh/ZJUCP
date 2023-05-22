#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <variant>
#include <iostream>
#include <cassert>
// #include <variant>
using namespace std;

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
        std::variant<IntVar, ConstInt> value;
        enum TYPE {INT,CONST,UNKNOWN};
        TYPE tag;
        Symbol(int value_,TYPE tag_){
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
    int insert(const std::string &name,int value, TYPE mode){
        if(is_exist(name)){
            std::cout<<"var already exist :"<<name<<std::endl;
            return 0;
        }
        if(mode == SymbolTable::INT){
            Symbol* symbol = new Symbol(value,Symbol::INT);
            map.insert({name,symbol});
            return 1;
        }else if(mode == SymbolTable::CONST){
            Symbol* symbol = new Symbol(value,Symbol::CONST);
            map.insert({name,symbol});
            return 1;
        }else{
            std::cout<<"error input mode\n";
            return 0;
        }
    }
    int insert(const std::string &name,TYPE mode){
        if(is_exist(name)){
            std::cout<<"var already exist :"<<name<<std::endl;
            return 0;
        }
        if(mode == SymbolTable::INT){
            map.insert({name,nullptr});
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
            res->second = new Symbol(value,Symbol::INT);
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

    // 向符号表栈（最顶端的符号表）插入一个变量
    int insert(const std::string &name,int value, SymbolTable::TYPE mode){
        return symbol_table_stack.back()->insert(name, value, mode);
    }

    int insert(const std::string &name,SymbolTable::TYPE mode){
        return symbol_table_stack.back()->insert(name, mode);
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

    // 获取包含name，并且距离当前作用域最近的那个作用域中符号表里面name的值
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

    // 获取symbol table stack中符号表的张数
    int size(){
        return symbol_table_stack.size();
    }

    // 获取符号表栈中最新的符号表
    const std::unique_ptr<SymbolTable>& get_top_symbol_tb(){
        const auto& tb = symbol_table_stack.back();

    }
};