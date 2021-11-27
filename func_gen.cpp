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
    auto module = new Module("SysY code");
    auto builder = new IRStmtBuilder(nullptr, module);
    Type *Int32Type = Type::get_int32_type(module);
    Type *FloatType = Type::get_float_type(module);
    std::vector<Type *> params_add(2, Int32Type);
    auto addFunTy = FunctionType::get(Int32Type, params_add);
    auto addFun = Function::create(addFunTy, "add", module);
    auto bb = BasicBlock::create(module, "entry", addFun);
    builder->set_insert_point(bb);
    auto retAlloca = builder->create_alloca(Int32Type);
    auto aAlloca = builder->create_alloca(Int32Type);
    auto bAlloca = builder->create_alloca(Int32Type);
    std::vector<Value *> args;
    for (auto arg = addFun->arg_begin(); arg != addFun->arg_end();arg++){
        args.push_back(*arg);
    }
    builder->create_store(args[0], aAlloca);
    builder->create_store(args[1], bAlloca);
    auto aLoad = builder->create_load(aAlloca);
    auto bLoad = builder->create_load(bAlloca);
    auto add = builder->create_iadd(aLoad, bLoad);
    auto sub = builder->create_isub(add, CONST_INT(1));
    builder->create_store(sub, retAlloca);
    auto retLoad = builder->create_load(retAlloca);
    builder->create_ret(retLoad);
    auto mainFunTy = FunctionType::get(Int32Type, {});
    auto mainFun = Function::create(mainFunTy, "main", module);
    bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);
    retAlloca = builder->create_alloca(Int32Type);
    auto resAlloca = builder->create_alloca(Int32Type);
    aAlloca = builder->create_alloca(Int32Type);
    bAlloca = builder->create_alloca(Int32Type);
    auto cAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(3), aAlloca);
    builder->create_store(CONST_INT(2), bAlloca);
    builder->create_store(CONST_INT(5), cAlloca);
    aLoad = builder->create_load(aAlloca);
    bLoad = builder->create_load(bAlloca);
    auto call = builder->create_call(addFun, {aLoad, bLoad});
    builder->create_store(call, resAlloca);
    auto resLoad = builder->create_load(resAlloca);
    auto cLoad = builder->create_load(cAlloca);
    add = builder->create_iadd(cLoad, resLoad);
    builder->create_store(add, retAlloca);
    retLoad = builder->create_load(retAlloca);
    builder->create_ret(retLoad);
    std::cout << module->print();
    delete module;
    return 0;
}
