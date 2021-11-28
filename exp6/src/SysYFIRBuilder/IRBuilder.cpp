#include "IRBuilder.h"

#define CONST_INT(num) ConstantInt::get(num, module.get())
#define CONST_FLOAT(num) ConstantFloat::get(num, module.get())

// You can define global variables and functions here
// to store state

// store temporary value
Value *tmp_val = nullptr;
std::vector<BasicBlock*> tmp_condbb;
std::vector<BasicBlock *> tmp_falsebb;

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

void IRBuilder::visit(SyntaxTree::LVal &node) {}

void IRBuilder::visit(SyntaxTree::AssignStmt &node) {}

void IRBuilder::visit(SyntaxTree::Literal &node) {}

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
        if(dynamic_cast<SyntaxTree::BreakStmt*>(stmt.get())!=nullptr||dynamic_cast<SyntaxTree::ContinueStmt*>(stmt.get())!=nullptr){
            break;
        }
    }
    this->scope.exit();
}

void IRBuilder::visit(SyntaxTree::EmptyStmt &node) {}

void IRBuilder::visit(SyntaxTree::ExprStmt &node) {}

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

void IRBuilder::visit(SyntaxTree::BinaryExpr &node) {}

void IRBuilder::visit(SyntaxTree::UnaryExpr &node) {}

void IRBuilder::visit(SyntaxTree::FuncCallStmt &node) {}

void IRBuilder::visit(SyntaxTree::IfStmt &node) {
    auto trueBB = BasicBlock::create(this->builder->get_module(), "trueBB_if", this->builder->get_module()->get_functions().back());
    auto falseBB = BasicBlock::create(this->builder->get_module(), "falseBB_if", this->builder->get_module()->get_functions().back());
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
    node.cond_exp->accept(*this);
    this->builder->create_cond_br(tmp_val, trueBB, falseBB);
    this->builder->set_insert_point(trueBB);
    tmp_condbb.push_back(condBB);
    tmp_falsebb.push_back(falseBB);
    node.statement->accept(*this);
    this->builder->create_br(condBB);
    this->builder->set_insert_point(falseBB);
}

void IRBuilder::visit(SyntaxTree::BreakStmt &node) {
    this->builder->create_br(tmp_falsebb.back());
    tmp_falsebb.pop_back();
}

void IRBuilder::visit(SyntaxTree::ContinueStmt &node) {
    this->builder->create_br(tmp_condbb.back());
    tmp_condbb.pop_back();
}
