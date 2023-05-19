#include "src/SysTable.h"

#include <iostream>

SymbolTable sb;
using namespace std;

int main(){

    sb.insert("x",1,SymbolTable::INT);
    sb.Print();
    sb.insert("y",1,SymbolTable::CONST);
    sb.Print();

    sb.insert("z",SymbolTable::INT);
    sb.Update("z",3);
    sb.Print();

    sb.insert("w",SymbolTable::CONST);
    sb.Print();

    sb.insert("x",2,SymbolTable::INT);

    std::cout<<"x get value"<<sb.Get_value("x");

}