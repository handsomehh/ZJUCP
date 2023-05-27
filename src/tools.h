#pragma once
#include <string>
#include <unordered_map>
#include <set>
#include <vector>
#include <stack>
#include <memory>

class While_Struct{
public:
    While_Struct(std::string _while_entry_name, std::string _while_body_name, std::string _end_name){
        while_entry_name = _while_entry_name;
        while_body_name = _while_body_name;
        end_name = _end_name;
    }

    std::string while_entry_name;
    std::string while_body_name;
    std::string end_name;
};

class While_Stack{
private:
    std::deque<std::unique_ptr<While_Struct>> while_stk;

public:
    void insert(const std::unique_ptr<While_Struct> &while_label_combination){
        while_stk.emplace_back(new While_Struct(while_label_combination->while_entry_name, while_label_combination->while_body_name, while_label_combination->end_name));
    }

    void pop(){
        while_stk.pop_back();
    }

    std::string Get_end_name(){
        return while_stk.back()->end_name;
    }

    std::string Get_entry_name(){
        return while_stk.back()->while_entry_name;
    }
};

class KoopaString
{
private:
    std::string koopa_str;

public:
    std::string alloc32i = " = alloc i32\n";
    std::string alloc32i_g = " = alloc i32, ";
    std::string zero = "zeroinit";
    std::string if_label = std::string("%") + "then";
    std::string else_label = std::string("%") + "else";
    std::string end_label = std::string("%") + "end";
    std::string while_entry_label = std::string("%") + "while_entry";
    std::string while_body_label = std::string("%") + "while_body";
    std::string head_func_decal = "decl @getint(): i32\ndecl @getch(): i32\ndecl @getarray(*i32): i32\ndecl @putint(i32)\ndecl @putch(i32)\ndecl @putarray(i32, *i32)\ndecl @starttime()\ndecl @stoptime()\n\n";
    void func_decl()
    {
        koopa_str += head_func_decal;
    }
    void append(const std::string &s)
    {
        koopa_str += s;
    }
    void appendaddtab(const std::string &s)
    {
        koopa_str += "  " + s;
    }
    void label(const std::string &s)
    {
        koopa_str += s + ":\n";
    }

    void ret(const std::string &name)
    {
        koopa_str += "  ret " + name + "\n";
    }

    void logic(const std::string &res, const std::string &src1, const std::string &src2, const std::string &op)
    {
        koopa_str += "  " + res + " = " + op + " " + src1 + ", " + src2 + "\n";
    }

    // [NEED MODIFI] 获取数组的在koopa IR中的类型表达，例如int a[5] => [i32, 5]
    std::string getArrType(const std::vector<int> &arr_size){
        std::string type = "i32";
        for(int i = arr_size.size() - 1; i >= 0; --i){
            type = "[" + type + ", " + std::to_string(arr_size[i]) + "]";
        }
        return type;
    }

    // [NEED MODIFI] 数组内容在ptr所指的内存区域，数组类型由len描述. ptr[i]为常量，或者是KoopaIR中的名字
    std::string getInitList(std::string *ptr, const std::vector<int> &len){
        std::string ret = "{";
        if(len.size() == 1){
            int n = len[0];
            ret += ptr[0];
            for(int i = 1; i < n; ++i){
                ret += ", " + ptr[i];
            }
        } else {
            int n = len[0], width = 1;
            std::vector<int> sublen(len.begin() + 1, len.end());
            for(auto iter = len.end() - 1; iter != len.begin(); --iter)
                width *= *iter;
            ret += getInitList(ptr, sublen);
            for(int i = 1; i < n; ++i){
                ret += ", " + getInitList(ptr + width * i, sublen);
            }
        }
        ret += "}";
        return ret;
    }

    void getelemptr(const std::string& to, const std::string &from, const int i){
        koopa_str += "  " + to + " = getelemptr " + from + ", " + std::to_string(i) + "\n";
    }

    void getelemptr(const std::string& to, const std::string &from, const std::string& i){
        koopa_str += "  " + to + " = getelemptr " + from + ", " + i + "\n";
    }

    const char *c_str() { return koopa_str.c_str(); }
};

class RiscvString
{
private:
    std::string riscv_str;

public:
    void append(const std::string &s)
    {
        riscv_str += s;
    }
    void ret()
    {
        riscv_str += "  ret\n";
    }
    void li(const std::string &to, int im)
    {
        riscv_str += "  li    " + to + ", " + std::to_string(im) + "\n";
    }

    void label(const std::string &name)
    {
        this->append(name + ":\n");
    }

    std::string Get_result()
    {
        return riscv_str;
    }
};