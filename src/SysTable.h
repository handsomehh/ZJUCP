#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <variant>
#include <iostream>
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
    Counter* mycount = new Counter();
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
            std::cout<<"not found :"<<name<<std::endl;
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