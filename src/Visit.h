#pragma once
#include "koopa.h"
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <cstring>
#include <cstdlib>
#include <string>

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

// 函数声明
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice) ;
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);

void Visit(const koopa_raw_return_t &value);
int  Visit(const koopa_raw_integer_t &value);
void Visit(const koopa_raw_binary_t &value);
void Visit(const koopa_raw_load_t &load);
void Visit(const koopa_raw_store_t &store);
void Visit(const koopa_raw_branch_t &branch);
void Visit(const koopa_raw_jump_t &jump);
void Visit(const koopa_raw_call_t &call);
void Visit(const koopa_raw_get_elem_ptr_t& get_elem_ptr);
void Visit(const koopa_raw_get_ptr_t& get_ptr);

void VisitGlobalVar(koopa_raw_value_t value);
void initGlobalArray(koopa_raw_value_t init);

size_t getTypeSize(koopa_raw_type_t ty);