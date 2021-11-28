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

void IRBuilder::visit(SyntaxTree::ReturnStmt &node) {}

void IRBuilder::visit(SyntaxTree::BlockStmt &node) {}

void IRBuilder::visit(SyntaxTree::EmptyStmt &node) {}

void IRBuilder::visit(SyntaxTree::ExprStmt &node) {
    node.exp->accept(*this);
}

void IRBuilder::visit(SyntaxTree::UnaryCondExpr &node) {}

void IRBuilder::visit(SyntaxTree::BinaryCondExpr &node) {}

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

void IRBuilder::visit(SyntaxTree::IfStmt &node) {}

void IRBuilder::visit(SyntaxTree::WhileStmt &node) {}

void IRBuilder::visit(SyntaxTree::BreakStmt &node) {}

void IRBuilder::visit(SyntaxTree::ContinueStmt &node) {}
