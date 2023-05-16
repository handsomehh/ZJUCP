#pragma once
#include "koopa.h"
// 函数声明

void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice) ;
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);

void Visit(const koopa_raw_return_t &value);
int Visit(const koopa_raw_integer_t &value);