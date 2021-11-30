#include "IRBuilder.h"

#define CONST_INT(num) ConstantInt::get(num, module.get())
#define CONST_FLOAT(num) ConstantFloat::get(num, module.get())

#define Binary_op(op,type,lhs_val,rhs_val) \
    if((type)==INT32_T){\
        tmp_val=builder->create_i##op((lhs_val),(rhs_val));\
    }\
    else{\
        tmp_val=builder->create_f##op((lhs_val),(rhs_val));\
    }

#define LVal_to_RVal(lval) \
    if(lval->get_type()==INT32PTR_T || lval->get_type()==FLOATPTR_T){\
        lval=builder->create_load(lval);\
    }

#define create_idiv create_isdiv

// You can define global variables and functions here
// to store state

// store temporary value
Value *tmp_val = nullptr;
std::vector<BasicBlock*> tmp_condbb_while;
std::vector<BasicBlock *> tmp_falsebb_while;
BasicBlock *tmp_falsebb;
bool break_or_continue;

// types
Type *VOID_T;
Type *INT1_T;
Type *INT32_T;
Type *FLOAT_T;
Type *INT32PTR_T;
Type *FLOATPTR_T;

void IRBuilder::visit(SyntaxTree::Assembly &node) {
    VOID_T = Type::get_void_type(module.get());
    INT1_T = Type::get_int1_type(module.get());
    INT32_T = Type::get_int32_type(module.get());
    FLOAT_T = Type::get_float_type(module.get());
    INT32PTR_T = Type::get_int32_ptr_type(module.get());
    FLOATPTR_T = Type::get_float_ptr_type(module.get());
    for (const auto &def : node.global_defs) {
        def->accept(*this);
    }
}

// You need to fill them

void IRBuilder::visit(SyntaxTree::InitVal &node) {}

void IRBuilder::visit(SyntaxTree::FuncDef &node) {}

void IRBuilder::visit(SyntaxTree::FuncFParamList &node) {}

void IRBuilder::visit(SyntaxTree::FuncParam &node) {}

void IRBuilder::visit(SyntaxTree::VarDef &node) {}

//返回的tmp_val是左值(地址)，用到值时要转为右值(用load指令)
void IRBuilder::visit(SyntaxTree::LVal &node) {
    auto lval=scope.find(node.name,false);
    if(lval){
        // auto lval_type=lval->get_type();

        if(!node.array_index.empty()){
            for (auto index : node.array_index) {
                index->accept(*this);
                //todo ...=tmp_val 用一个vector记录多维数组的下标，现在只实现了一维
            }
            LVal_to_RVal(tmp_val)
            tmp_val=builder->create_gep(lval,{CONST_INT(0), tmp_val});
        }
        else{
            tmp_val=lval;
        }

    }
    
}

void IRBuilder::visit(SyntaxTree::AssignStmt &node) {
    node.target->accept(*this);
    auto addr=tmp_val;
    node.value->accept(*this);
    auto val=tmp_val;
    if(addr->get_type()==INT32PTR_T && val->get_type()==FLOAT_T){
        val=builder->create_fptosi(val,INT32_T);
    }
    else if(addr->get_type()==FLOATPTR_T && val->get_type()==INT32_T){
        val=builder->create_sitofp(val,FLOAT_T);
    }
    builder->create_store(val,addr);
}

void IRBuilder::visit(SyntaxTree::Literal &node) {
    if(node.literal_type == SyntaxTree::Type::INT){
        tmp_val=CONST_INT(node.int_const);
    }
    else{
        tmp_val=CONST_FLOAT(node.float_const);
    }
}

void IRBuilder::visit(SyntaxTree::ReturnStmt &node) {
    if(node.ret!=nullptr){
        node.ret->accept(*this);
        this->builder->create_ret(tmp_val);
    }
    else{
        this->builder->create_void_ret();
    }
}

void IRBuilder::visit(SyntaxTree::BlockStmt &node) {
     this->scope.enter();
    for(auto stmt : node.body){
        stmt->accept(*this);
        if(break_or_continue=true){
            break;
        }
    }
    this->scope.exit();
}

void IRBuilder::visit(SyntaxTree::EmptyStmt &node) {}

void IRBuilder::visit(SyntaxTree::ExprStmt &node) {
    node.exp->accept(*this);
}

void IRBuilder::visit(SyntaxTree::UnaryCondExpr &node) {}

void IRBuilder::visit(SyntaxTree::BinaryCondExpr &node) {
    Value *lval, *rval;
    node.lhs->accept(*this);
    lval = tmp_val;
    node.rhs->accept(*this);
    rval = tmp_val;
    if(lval->get_type()!=INT1_T){
        if (rval->get_type()!=INT1_T){
            if(lval->get_type()==FLOAT_T&&rval->get_type()==INT32_T){
                rval = this->builder->create_sitofp(rval, FLOAT_T);
            }
            else if(lval->get_type()==INT32_T&&rval->get_type()==FLOAT_T){
                lval = this->builder->create_sitofp(lval, FLOAT_T);
            }
        }
        else{
            if(lval->get_type()==INT32_T){
                rval = this->builder->create_zext(rval, INT32_T);
            }
            else{
                rval = this->builder->create_zext(rval, FLOAT_T);
            }
        }
    }
    else{
        if(rval->get_type()!=INT1_T){
            if(rval->get_type()==INT32_T){
                lval = this->builder->create_zext(lval, INT32_T);
            }
            else{
                lval = this->builder->create_zext(lval, FLOAT_T);
            }
        }
        else{
            lval = this->builder->create_zext(lval, INT32_T);
            rval = this->builder->create_zext(rval, INT32_T);
        }
    }
    if(lval->get_type()==INT32_T){
        if(node.op==SyntaxTree::BinaryCondOp::LT){
            tmp_val = this->builder->create_icmp_lt(lval, rval);
        }
        else if(node.op==SyntaxTree::BinaryCondOp::LTE){
            tmp_val = this->builder->create_icmp_le(lval, rval);
        }
        else if(node.op==SyntaxTree::BinaryCondOp::GT){
            tmp_val = this->builder->create_icmp_gt(lval, rval);
        }
        else if(node.op==SyntaxTree::BinaryCondOp::GTE){
            tmp_val = this->builder->create_icmp_ge(lval, rval);
        }
        else if(node.op==SyntaxTree::BinaryCondOp::EQ){
            tmp_val = this->builder->create_icmp_eq(lval, rval);
        }
        else if(node.op==SyntaxTree::BinaryCondOp::NEQ){
            tmp_val = this->builder->create_icmp_ne(lval, rval);
        }
    }
    else{
        if(node.op==SyntaxTree::BinaryCondOp::LT){
            tmp_val = this->builder->create_fcmp_lt(lval, rval);
        }
        else if(node.op==SyntaxTree::BinaryCondOp::LTE){
            tmp_val = this->builder->create_fcmp_le(lval, rval);
        }
        else if(node.op==SyntaxTree::BinaryCondOp::GT){
            tmp_val = this->builder->create_fcmp_gt(lval, rval);
        }
        else if(node.op==SyntaxTree::BinaryCondOp::GTE){
            tmp_val = this->builder->create_fcmp_ge(lval, rval);
        }
        else if(node.op==SyntaxTree::BinaryCondOp::EQ){
            tmp_val = this->builder->create_fcmp_eq(lval, rval);
        }
        else if(node.op==SyntaxTree::BinaryCondOp::NEQ){
            tmp_val = this->builder->create_fcmp_ne(lval, rval);
        }
    }
}

void IRBuilder::visit(SyntaxTree::BinaryExpr &node) {
    node.lhs->accept(*this);
    auto lhs_val = tmp_val;
    LVal_to_RVal(lhs_val)
    auto lhs_type = lhs_val->get_type();
    node.rhs->accept(*this);
    auto rhs_val = tmp_val;
    LVal_to_RVal(rhs_val)
    auto rhs_type = rhs_val->get_type();

    auto expr_type=FLOAT_T;
    if(lhs_type==FLOAT_T && !(rhs_type==FLOAT_T)){
        rhs_val=builder->create_sitofp(rhs_val,FLOAT_T);
    }
    else if(!(lhs_type==FLOAT_T) && rhs_type==FLOAT_T){
        lhs_val=builder->create_sitofp(lhs_val,FLOAT_T);
    }
    else if(lhs_type==INT32_T && rhs_type==INT32_T){
        expr_type=INT32_T;
    }
    
    
    switch(node.op){
        case SyntaxTree::BinOp::PLUS:
            Binary_op(add,expr_type,lhs_val,rhs_val)
            break;
        case SyntaxTree::BinOp::MINUS:
            Binary_op(sub,expr_type,lhs_val,rhs_val)
        break;
        case SyntaxTree::BinOp::MULTIPLY:
            Binary_op(mul,expr_type,lhs_val,rhs_val)
        break;
        case SyntaxTree::BinOp::DIVIDE:
            Binary_op(div,expr_type,lhs_val,rhs_val)
        break;
        case SyntaxTree::BinOp::MODULO:
            tmp_val=builder->create_isrem(lhs_val,rhs_val);//不检查取模类型
            break;
    }

}

void IRBuilder::visit(SyntaxTree::UnaryExpr &node) {
    node.rhs->accept(*this);
    LVal_to_RVal(tmp_val)

    if(node.op==SyntaxTree::UnaryOp::MINUS){
        if(tmp_val->get_type()==INT32_T){
            tmp_val=builder->create_isub(CONST_INT(0),tmp_val);
        }
        else{
            tmp_val=builder->create_fsub(CONST_FLOAT(0),tmp_val);
        }
    }
}

void IRBuilder::visit(SyntaxTree::FuncCallStmt &node) {
    auto func=scope.find(node.name,true);
    if(func){
        std::vector<Value *> args{};
        auto &func_args_iter=dynamic_cast<Function *>(func)->arg_begin();
        for(auto exp:node.params){
            exp->accept(*this);
            LVal_to_RVal(tmp_val)
            if(tmp_val->get_type()==INT32_T && (*func_args_iter)->get_type()==FLOAT_T){
                tmp_val=builder->create_sitofp(tmp_val,FLOAT_T);
            }
            else if(tmp_val->get_type()==FLOAT_T && (*func_args_iter)->get_type()==INT32_T){
                tmp_val=builder->create_fptosi(tmp_val,INT32_T);
            }
            args.push_back(tmp_val);
            func_args_iter++;
        }
        tmp_val=builder->create_call(func,args);
    }
}

void IRBuilder::visit(SyntaxTree::IfStmt &node) {
   auto trueBB = BasicBlock::create(this->builder->get_module(), "trueBB_if", this->builder->get_module()->get_functions().back());
    auto falseBB = BasicBlock::create(this->builder->get_module(), "falseBB_if", this->builder->get_module()->get_functions().back());
    tmp_falsebb = falseBB;
    node.cond_exp->accept(*this);
    this->builder->create_cond_br(tmp_val, trueBB, falseBB);
    this->builder->set_insert_point(trueBB);
    node.if_statement->accept(*this);
    this->builder->set_insert_point(falseBB);
    if(node.else_statement!=nullptr){
        node.else_statement->accept(*this);
    }
}

void IRBuilder::visit(SyntaxTree::WhileStmt &node) {
    auto condBB=BasicBlock::create(this->builder->get_module(),"condBB_while",this->builder->get_module()->get_functions().back());
    auto trueBB=BasicBlock::create(this->builder->get_module(),"trueBB_while",this->builder->get_module()->get_functions().back());
    auto falseBB=BasicBlock::create(this->builder->get_module(),"condBB_while",this->builder->get_module()->get_functions().back());
    this->builder->create_br(condBB);
    this->builder->set_insert_point(condBB);
    tmp_falsebb = condBB;
    node.cond_exp->accept(*this);
    this->builder->create_cond_br(tmp_val, trueBB, falseBB);
    this->builder->set_insert_point(trueBB);
    tmp_condbb_while.push_back(condBB);
    tmp_falsebb_while.push_back(falseBB);
    break_or_continue = false;
    node.statement->accept(*this);
    this->builder->create_br(condBB);
    this->builder->set_insert_point(falseBB);
    tmp_condbb_while.pop_back();
    tmp_falsebb_while.pop_back();
    break_or_continue = false;
}

void IRBuilder::visit(SyntaxTree::BreakStmt &node) {
    this->builder->create_br(tmp_falsebb_while.back());
    break_or_continue = true;
}

void IRBuilder::visit(SyntaxTree::ContinueStmt &node) {
    this->builder->create_br(tmp_condbb_while.back());
    break_or_continue = true;
}