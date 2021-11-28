#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRStmtBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>
#include <memory>

#ifdef DEBUG  // 用于调试信息,大家可以在编译过程中通过" -DDEBUG"来开启这一选项
#define DEBUG_OUTPUT std::cout << __LINE__ << std::endl;  // 输出行号的简单示例
#else
#define DEBUG_OUTPUT
#endif

#define CONST_INT(num) \
    ConstantInt::get(num, module)

#define CONST_FP(num) \
    ConstantFloat::get(num, module) // 得到常数值的表示,方便后面多次用到

int main(){
    auto module = new Module("SysY code");  // module name是什么无关紧要
    auto builder = new IRStmtBuilder(nullptr, module);
    Type *Int32Type = Type::get_int32_type(module);

    auto zero_initializer = ConstantZero::get(Int32Type, module);

    auto a = GlobalVariable::create("a", module, Int32Type, false, zero_initializer);
    auto b = GlobalVariable::create("b", module, Int32Type, false, zero_initializer);

    // main函数
    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                    "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    // BasicBlock的名字在生成中无所谓,但是可以方便阅读
    builder->set_insert_point(bb);

    auto retAlloca = builder->create_alloca(Int32Type);//在内存中分配返回值的位置
    builder->create_store(CONST_INT(0), retAlloca);
    builder->create_store(CONST_INT(0), b);
    builder->create_store(CONST_INT(3), a);

    auto condBB = BasicBlock::create(module, "condBB_while", mainFun);  // 条件BB
    auto trueBB = BasicBlock::create(module, "trueBB_while", mainFun);    // true分支
    auto falseBB = BasicBlock::create(module, "falseBB_while", mainFun);  // false分支

    auto retBB = BasicBlock::create(module, "retBB", mainFun);  // return分支,提前create,以便false分支可以br

    builder->create_br(condBB);

    builder->set_insert_point(condBB);  // while
    auto aLoad = builder->create_load(a);
    auto icmp = builder->create_icmp_gt(aLoad, CONST_INT(0));  // a>0

    builder->create_cond_br(icmp, trueBB, falseBB);  // 条件BR

    builder->set_insert_point(trueBB);  // if true; 分支的开始需要SetInsertPoint设置
    auto bLoad = builder->create_load(b);
    auto b_add_a=builder->create_iadd(bLoad,aLoad); //b+a
    builder->create_store(b_add_a,b); //b=b+a
    auto a_sub_1=builder->create_isub(aLoad,CONST_INT(1)); //a-1
    builder->create_store(a_sub_1,a); //a=a-1
    builder->create_br(condBB);

    builder->set_insert_point(falseBB);  // if false
    builder->create_br(retBB);  // br retBB

    builder->set_insert_point(retBB);  // ret分支
    bLoad = builder->create_load(b);
    builder->create_ret(bLoad);  // return b

    std::cout << module->print();
    delete module;
    return 0;
}
