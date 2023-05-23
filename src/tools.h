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
    std::string alloc32i = " = alloc i32\n";
    std::string if_label = std::string("%") + "then";
    std::string else_label = std::string("%") + "else";
    std::string end_label = std::string("%") + "end";

    void append(const std::string &s){
        koopa_str += s;
    }
    void appendaddtab(const std::string &s){
        koopa_str+= "  "+s;
    }
    void label(const std::string &s){
        koopa_str += s + ":\n";
    }

    void ret(const std::string &name){
        koopa_str +="  ret " + name + "\n";
    }

    void logic(const std::string &res,const std::string &src1,const std::string &src2,const std::string &op){
        koopa_str +="  "+ res + " = " + op + " " + src1 + ", " + src2 + "\n";
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
//     bool immediate(int i){ return -2048 <= i && i < 2048; }

//     void binary(const std::string &op, const std::string &rd, const std::string &rs1, const std::string &rs2){
//     riscv_str += "  " + op + std::string(6-op.length(),' ') + rd + ", " + rs1 + ", " + rs2 + "\n";
//     }
    
//     void two(const std::string &op, const std::string &a, const std::string &b){
//         riscv_str += "  " + op + std::string(6 - op.length(), ' ') + a + ", " + b + "\n";
//     }



//     void mov(const std::string &from, const std::string &to){
//         riscv_str += "  mv    " + to + ", "  + from + '\n';
//     }





//     void load(const std::string &to, const std::string &base ,int offset){
//         if(offset >= -2048 && offset < 2048)
//             riscv_str += "  lw    " + to + ", " + std::to_string(offset) + "(" + base + ")\n";    
//         else{
//             this->li("t3", offset);
//             this->binary("add", "t3", "t3", base);
//             riscv_str += "  lw    " + to + ", " + "0" + "(" + "t3" + ")\n";    
//         }
//     }


//     void store(const std::string &from, const std::string &base ,int offset){
//         if(offset >= -2048 && offset < 2048)
//             riscv_str += "  sw    " + from + ", " + std::to_string(offset) + "(" + base + ")\n";    
//         else{
//             this->li("t3", offset);
//             this->binary("add", "t3", "t3", base);
//             riscv_str += "  sw    " + from + ", " + "0" + "(" + "t3" + ")\n";  
//         }
//     }

//     void sp(int delta){
//         if(delta >= -2048 && delta < 2048){
//             this->binary("addi", "sp", "sp", std::to_string(delta));
//         }else{
//             this->li("t0", delta);
//             this->binary("add", "sp", "sp", "t0");
//         }
//     }
    
//     void label(const std::string &name){
//         this->append(name + ":\n");
//     }

//     void bnez(const std::string &rs, const std::string &target){
//         this->two("bnez", rs, target);
//     }

//     void jump(const std::string &target){
//         this->append("  j     " + target + "\n");
//     }

//     void call(const std::string &func){
//         this->append("  call " + func + "\n");
//     }

//     void zeroInitInt(){
//         this->append("  .zero 4\n");
//     }

//     void word(int i){
//         this->append("  .word " + std::to_string(i) + "\n");
//     }

//     void la(const std::string &to, const std::string &name){
//         this->append("  la    " + to + ", " + name + "\n");
//     }

//     const char* c_str(){
//         return riscv_str.c_str();
//     }

// };

// class BlockController{
// private:
//     bool f = true;
// public:
//     bool alive(){
//         return f;
//     }

//     void finish(){
//         f = false;
//     }

//     void set(){
//         f = true;
//     }
// };

// class WhileName{
// public:
//     std::string entry_name, body_name, end_name;
//     WhileName(const std::string &_entry, const std::string & _body, const std::string &_end): entry_name(_entry), body_name(_body), end_name(_end){}
// };

// class WhileStack{
// private:
//     std::stack<WhileName> whiles;
// public:
//     void append(const std::string &_entry, const std::string & _body, const std::string &_end){
//         whiles.emplace(_entry, _body, _end);
//     }
    
//     void quit(){
//         whiles.pop();
//     }

//     std::string getEntryName(){
//         return whiles.top().entry_name;
//     }

//     std::string getBodyName(){
//         return whiles.top().body_name;
//     }

//     std::string getEndName(){
//         return whiles.top().end_name;
//     }


// // 后端riscv生成时，使用到的临时标号
// class TempLabelManager{
// private:
//     int cnt;
// public:
//     TempLabelManager():cnt(0){ }
//     std::string getTmpLabel(){
//         return "Label" + std::to_string(cnt++);
//     }
// };