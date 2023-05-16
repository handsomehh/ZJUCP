#pragma once
#include <string>
#include <unordered_map>
#include <set>
#include <vector>
#include <stack>

class KoopaString{
private:
    std::string koopa_str;
public:
    void append(const std::string &s){
        koopa_str += s;
    }
    
    void label(const std::string &s){
        koopa_str += s + ":\n";
    }

    void ret(const std::string &name){
        koopa_str +="  ret " + name + "\n";
    }
    

    const char * c_str(){return koopa_str.c_str();}
};


class RiscvString{
private:
    std::string riscv_str;
public:
    void append(const std::string &s){
        riscv_str += s;
    }
    void ret(){
        riscv_str += "  ret\n";
    }
    void li(const std::string &to, int im){
        riscv_str += "  li    " + to + ", " + std::to_string(im) + "\n";
    }

    void label(const std::string &name){
        this->append(name + ":\n");
    }
    std::string Get_result(){
        return riscv_str;
    }
};