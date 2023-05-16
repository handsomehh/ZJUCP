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

