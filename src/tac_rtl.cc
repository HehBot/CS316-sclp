#include <algorithm>
#include <tac.h>
#include <rtl.h>
#include <vector>
#include <error.h>
#include <cassert>

using Register = RTL::Register;
using RTLStmtList = std::vector<std::shared_ptr<RTL::Stmt>>;

std::string RTL::Context::get_string_id(std::string val)
{
    auto const itb = string_store.begin(), ite = string_store.end();
    size_t s;
    std::vector<std::string>::iterator it;
    if ((it = std::find(itb, ite, val)) != ite)
        s = it - itb;
    else {
        s = string_store.size();
        string_store.push_back(val);
    }

    return "_str_" + std::to_string(s);
}

RTL::Context ctx;

size_t constexpr NUM_INT_REGISTERS = 19;
std::shared_ptr<Register> int_register_list[NUM_INT_REGISTERS] = {
    std::make_shared<Register>("v0"),
    std::make_shared<Register>("t0"),
    std::make_shared<Register>("t1"),
    std::make_shared<Register>("t2"),
    std::make_shared<Register>("t3"),
    std::make_shared<Register>("t4"),
    std::make_shared<Register>("t5"),
    std::make_shared<Register>("t6"),
    std::make_shared<Register>("t7"),
    std::make_shared<Register>("t8"),
    std::make_shared<Register>("t9"),
    std::make_shared<Register>("s0"),
    std::make_shared<Register>("s1"),
    std::make_shared<Register>("s2"),
    std::make_shared<Register>("s3"),
    std::make_shared<Register>("s4"),
    std::make_shared<Register>("s5"),
    std::make_shared<Register>("s6"),
    std::make_shared<Register>("s7")
};
bool int_register_allocated[NUM_INT_REGISTERS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

size_t constexpr NUM_FLOAT_REGISTERS = 15;
std::shared_ptr<Register> float_register_list[NUM_FLOAT_REGISTERS] = {
    std::make_shared<Register>("f2"),
    std::make_shared<Register>("f4"),
    std::make_shared<Register>("f6"),
    std::make_shared<Register>("f8"),
    std::make_shared<Register>("f10"),
    std::make_shared<Register>("f12"),
    std::make_shared<Register>("f14"),
    std::make_shared<Register>("f16"),
    std::make_shared<Register>("f18"),
    std::make_shared<Register>("f20"),
    std::make_shared<Register>("f22"),
    std::make_shared<Register>("f24"),
    std::make_shared<Register>("f26"),
    std::make_shared<Register>("f28"),
    std::make_shared<Register>("f30")
};
bool float_register_allocated[NUM_FLOAT_REGISTERS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

std::shared_ptr<Register> REG_v0 = int_register_list[0];
std::shared_ptr<Register> REG_v1 = std::make_shared<Register>("v1");
std::shared_ptr<Register> REG_f12 = float_register_list[5];
std::shared_ptr<Register> REG_a0 = std::make_shared<Register>("a0");
std::shared_ptr<Register> REG_zero = std::make_shared<Register>("zero");
std::shared_ptr<Register> REG_f0 = std::make_shared<Register>("f0");

std::shared_ptr<Register> allocate_int_register()
{
    for (size_t i = 0; i < NUM_INT_REGISTERS; i++) {
        if (!int_register_allocated[i]) {
            int_register_allocated[i] = true;
            return int_register_list[i];
        }
    }
    assert(false);
}

void deallocate_int_register(std::shared_ptr<Register> reg)
{
    for (size_t i = 0; i < NUM_INT_REGISTERS; i++) {
        if (reg == int_register_list[i]) {
            int_register_allocated[i] = false;
            return;
        }
    }
    // assert(false);
}

void RTL::reset(){
    for(size_t i = 0; i < NUM_INT_REGISTERS; i++)
        int_register_allocated[i] = false;
    for(size_t i = 0; i < NUM_FLOAT_REGISTERS; i++)
        float_register_allocated[i] = false;
}

std::shared_ptr<Register> allocate_float_register()
{
    for (size_t i = 0; i < NUM_FLOAT_REGISTERS; i++) {
        if (!float_register_allocated[i]) {
            float_register_allocated[i] = true;
            return float_register_list[i];
        }
    }
    assert(false);
}

void deallocate_float_register(std::shared_ptr<Register> reg)
{
    for (size_t i = 0; i < NUM_FLOAT_REGISTERS; i++) {
        if (reg == float_register_list[i]) {
            float_register_allocated[i] = false;
            return;
        }
    }
    // assert(false);
}


void TAC::PrintStmt::gen_rtl(RTLStmtList& stmts)
{
    arg->gen_print_rtl(stmts);
}

void TAC::IntLit::gen_print_rtl(RTLStmtList& stmts)
{
    stmts.push_back(std::make_shared<RTL::ILoadStmt>(REG_v0, std::make_shared<RTL::IntLit>(1)));
    stmts.push_back(std::make_shared<RTL::ILoadStmt>(REG_a0, std::make_shared<RTL::IntLit>(val)));
    stmts.push_back(std::make_shared<RTL::WriteStmt>());
}

void TAC::FloatLit::gen_print_rtl(RTLStmtList& stmts)
{
    stmts.push_back(std::make_shared<RTL::ILoadStmt>(REG_v0, std::make_shared<RTL::IntLit>(3)));
    stmts.push_back(std::make_shared<RTL::ILoadDStmt>(REG_f12, std::make_shared<RTL::FloatLit>(val)));
    stmts.push_back(std::make_shared<RTL::WriteStmt>());
}

void TAC::StrLit::gen_print_rtl(RTLStmtList& stmts)
{
    std::string str_id = ctx.get_string_id(val);

    stmts.push_back(std::make_shared<RTL::ILoadStmt>(REG_v0, std::make_shared<RTL::IntLit>(4)));
    stmts.push_back(std::make_shared<RTL::LoadAddrStmt>(REG_a0, std::make_shared<RTL::Mem>(str_id, true, 0)));
    stmts.push_back(std::make_shared<RTL::WriteStmt>());
}

void TAC::Sym::gen_print_rtl(RTLStmtList& stmts)
{
    if (type == Type::STRING) {
        // ASSUMPTION: in_mem is always true in this case
        stmts.push_back(std::make_shared<RTL::ILoadStmt>(REG_v0, std::make_shared<RTL::IntLit>(4)));
        stmts.push_back(std::make_shared<RTL::LoadStmt>(REG_a0, std::make_shared<RTL::Mem>(name, is_global, fp_offset)));
        stmts.push_back(std::make_shared<RTL::WriteStmt>());
    } else if (type != Type::FLOAT) {
        if (in_mem) {
            stmts.push_back(std::make_shared<RTL::ILoadStmt>(REG_v0, std::make_shared<RTL::IntLit>(1)));
            stmts.push_back(std::make_shared<RTL::LoadStmt>(REG_a0, std::make_shared<RTL::Mem>(name, is_global, fp_offset)));
            stmts.push_back(std::make_shared<RTL::WriteStmt>());
        } else {
            if (int_register_allocated[0]) { // v0 is at index 0
                std::shared_ptr<RTL::Register> new_reg = allocate_int_register();
                stmts.push_back(std::make_shared<RTL::MoveStmt>(new_reg, reg));

                deallocate_int_register(reg);

                reg = new_reg;
            }
            stmts.push_back(std::make_shared<RTL::ILoadStmt>(REG_v0, std::make_shared<RTL::IntLit>(1)));
            stmts.push_back(std::make_shared<RTL::MoveStmt>(REG_a0, reg));
            stmts.push_back(std::make_shared<RTL::WriteStmt>());

            deallocate_int_register(reg);
        }
    } else {
        if (in_mem) {
            stmts.push_back(std::make_shared<RTL::ILoadStmt>(REG_v0, std::make_shared<RTL::IntLit>(3)));
            stmts.push_back(std::make_shared<RTL::LoadDStmt>(REG_f12, std::make_shared<RTL::Mem>(name, is_global, fp_offset)));
            stmts.push_back(std::make_shared<RTL::WriteStmt>());
        } else {
            stmts.push_back(std::make_shared<RTL::ILoadStmt>(REG_v0, std::make_shared<RTL::IntLit>(3)));
            stmts.push_back(std::make_shared<RTL::MoveDStmt>(REG_f12, reg));
            stmts.push_back(std::make_shared<RTL::WriteStmt>());
        }
    }
}

void TAC::ReadIntStmt::gen_rtl(RTLStmtList& stmts)
{
    std::shared_ptr<Register> loc_reg = loc->gen_rtl(stmts);
    stmts.push_back(std::make_shared<RTL::ILoadStmt>(REG_v0, std::make_shared<RTL::IntLit>(5)));
    stmts.push_back(std::make_shared<RTL::ReadStmt>());
    stmts.push_back(std::make_shared<RTL::AddrAssignStmt>(loc_reg, REG_v0));
}
void TAC::ReadFloatStmt::gen_rtl(RTLStmtList& stmts)
{
    std::shared_ptr<Register> loc_reg = loc->gen_rtl(stmts);
    stmts.push_back(std::make_shared<RTL::ILoadStmt>(REG_v0, std::make_shared<RTL::IntLit>(7)));
    stmts.push_back(std::make_shared<RTL::ReadStmt>());
    stmts.push_back(std::make_shared<RTL::AddrAssignDStmt>(loc_reg, REG_f0));
}


void TAC::Label::gen_rtl(RTLStmtList& stmts)
{
    stmts.push_back(std::make_shared<RTL::Label>(name));
}


std::shared_ptr<RTL::Register> TAC::Sym::gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts)
{
    if (in_mem) {
        if (type != Type::FLOAT) {
            reg = allocate_int_register();
            stmts.push_back(std::make_shared<RTL::LoadStmt>(reg, std::make_shared<RTL::Mem>(name, is_global, fp_offset)));
        } else {
            reg = allocate_float_register();
            stmts.push_back(std::make_shared<RTL::LoadDStmt>(reg, std::make_shared<RTL::Mem>(name, is_global, fp_offset)));
        }
    }
    return reg;
}


void TAC::GotoStmt::gen_rtl(RTLStmtList& stmts)
{
    stmts.push_back(std::make_shared<RTL::GotoStmt>(std::make_shared<RTL::Label>(label->name)));
}

void TAC::IfGotoStmt::gen_rtl(RTLStmtList& stmts)
{
    std::shared_ptr<Register> cond_reg = cond->gen_rtl(stmts);
    stmts.push_back(std::make_shared<RTL::BGTZStmt>(cond_reg, std::make_shared<RTL::Label>(label->name)));
    deallocate_int_register(cond_reg);
}


void TAC::AssignStmt::gen_rtl(RTLStmtList& stmts)
{
    if (rhs->type != TAC::Type::FLOAT) {
        // ASSUMPTION: If LHS is in_mem, then RHS is either immediate or a temp
        std::shared_ptr<Register> rhs_reg = rhs->gen_rtl(stmts);
        if (lhs->in_mem) {
            stmts.push_back(std::make_shared<RTL::StoreStmt>(std::make_shared<RTL::Mem>(lhs->name, lhs->is_global, lhs->fp_offset), rhs_reg));
            deallocate_int_register(rhs_reg);
        } else
            lhs->reg = rhs_reg;
    } else {
        // ASSUMPTION: If LHS is in_mem, then RHS is either immediate or a temp
        std::shared_ptr<Register> rhs_reg = rhs->gen_rtl(stmts);
        if (lhs->in_mem) {
            stmts.push_back(std::make_shared<RTL::StoreDStmt>(std::make_shared<RTL::Mem>(lhs->name, lhs->is_global, lhs->fp_offset), rhs_reg));
            deallocate_float_register(rhs_reg);
        } else
            lhs->reg = rhs_reg;
    }
}

std::shared_ptr<Register> TAC::IntLit::gen_rtl(RTLStmtList& stmts)
{
    std::shared_ptr<Register> reg = allocate_int_register();
    stmts.push_back(std::make_shared<RTL::ILoadStmt>(reg, std::make_shared<RTL::IntLit>(val)));
    return reg;
}

std::shared_ptr<Register> TAC::FloatLit::gen_rtl(RTLStmtList& stmts)
{
    std::shared_ptr<Register> reg = allocate_float_register();
    stmts.push_back(std::make_shared<RTL::ILoadDStmt>(reg, std::make_shared<RTL::FloatLit>(val)));
    return reg;
}

std::shared_ptr<Register> TAC::StrLit::gen_rtl(RTLStmtList& stmts)
{
    std::string str_id = ctx.get_string_id(val);

    std::shared_ptr<Register> reg = allocate_int_register();
    stmts.push_back(std::make_shared<RTL::LoadAddrStmt>(reg, std::make_shared<RTL::Mem>(str_id, true, 0)));
    return reg;
}

std::shared_ptr<Register> TAC::AddExpr::gen_rtl(RTLStmtList& stmts)
{
    if (type != Type::FLOAT) {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_res = allocate_int_register();
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::AddStmt>(reg_res, reg_left, reg_right));

        deallocate_int_register(reg_left);
        deallocate_int_register(reg_right);

        return reg_res;
    } else {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_res = allocate_float_register();
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::AddDStmt>(reg_res, reg_left, reg_right));

        deallocate_float_register(reg_left);
        deallocate_float_register(reg_right);

        return reg_res;
    }
}

std::shared_ptr<Register> TAC::SubExpr::gen_rtl(RTLStmtList& stmts)
{
    if (type != Type::FLOAT) {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_res = allocate_int_register();
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::SubStmt>(reg_res, reg_left, reg_right));

        deallocate_int_register(reg_left);
        deallocate_int_register(reg_right);

        return reg_res;
    } else {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_res = allocate_float_register();
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::SubDStmt>(reg_res, reg_left, reg_right));

        deallocate_float_register(reg_left);
        deallocate_float_register(reg_right);

        return reg_res;
    }
}

std::shared_ptr<Register> TAC::MulExpr::gen_rtl(RTLStmtList& stmts)
{
    if (type != Type::FLOAT) {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_res = allocate_int_register();
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::MulStmt>(reg_res, reg_left, reg_right));

        deallocate_int_register(reg_left);
        deallocate_int_register(reg_right);

        return reg_res;
    } else {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_res = allocate_float_register();
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::MulDStmt>(reg_res, reg_left, reg_right));

        deallocate_float_register(reg_left);
        deallocate_float_register(reg_right);

        return reg_res;
    }
}

std::shared_ptr<Register> TAC::DivExpr::gen_rtl(RTLStmtList& stmts)
{
    if (type != Type::FLOAT) {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_res = allocate_int_register();
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::DivStmt>(reg_res, reg_left, reg_right));

        deallocate_int_register(reg_left);
        deallocate_int_register(reg_right);

        return reg_res;
    } else {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_res = allocate_float_register();
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::DivDStmt>(reg_res, reg_left, reg_right));

        deallocate_float_register(reg_left);
        deallocate_float_register(reg_right);

        return reg_res;
    }
}


std::shared_ptr<Register> TAC::NegExpr::gen_rtl(RTLStmtList& stmts)
{
    if (lhs->type != Type::FLOAT) {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_res = allocate_int_register();

        stmts.push_back(std::make_shared<RTL::UMinusStmt>(reg_res, reg_left));

        deallocate_int_register(reg_left);

        return reg_res;
    } else {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_res = allocate_float_register();

        stmts.push_back(std::make_shared<RTL::UMinusDStmt>(reg_res, reg_left));

        deallocate_float_register(reg_left);

        return reg_res;
    }
}


std::shared_ptr<Register> TAC::EqualExpr::gen_rtl(RTLStmtList& stmts)
{
    if (rhs->type != Type::FLOAT) {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_res = allocate_int_register();
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::SEQStmt>(reg_res, reg_left, reg_right));

        deallocate_int_register(reg_left);
        deallocate_int_register(reg_right);

        return reg_res;
    } else {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::SEQDStmt>(reg_left, reg_right));

        deallocate_float_register(reg_left);
        deallocate_float_register(reg_right);

        std::shared_ptr<Register> reg1 = allocate_int_register();
        std::shared_ptr<Register> reg2 = allocate_int_register();

        stmts.push_back(std::make_shared<RTL::ILoadStmt>(reg1, std::make_shared<RTL::IntLit>(1)));
        stmts.push_back(std::make_shared<RTL::MoveStmt>(reg2, REG_zero));
        stmts.push_back(std::make_shared<RTL::MovTStmt>(reg2, reg1, std::make_shared<RTL::IntLit>(0)));

        deallocate_int_register(reg1);

        return reg2;
    }
}

std::shared_ptr<Register> TAC::NotEqualExpr::gen_rtl(RTLStmtList& stmts)
{
    if (rhs->type != Type::FLOAT) {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_res = allocate_int_register();
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::SNEStmt>(reg_res, reg_left, reg_right));

        deallocate_int_register(reg_left);
        deallocate_int_register(reg_right);

        return reg_res;
    } else {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::SEQDStmt>(reg_left, reg_right));

        deallocate_float_register(reg_left);
        deallocate_float_register(reg_right);

        std::shared_ptr<Register> reg1 = allocate_int_register();
        std::shared_ptr<Register> reg2 = allocate_int_register();

        stmts.push_back(std::make_shared<RTL::ILoadStmt>(reg1, std::make_shared<RTL::IntLit>(1)));
        stmts.push_back(std::make_shared<RTL::MoveStmt>(reg2, REG_zero));
        stmts.push_back(std::make_shared<RTL::MovFStmt>(reg2, reg1, std::make_shared<RTL::IntLit>(0)));

        deallocate_int_register(reg1);

        return reg2;
    }
}

std::shared_ptr<Register> TAC::GreaterExpr::gen_rtl(RTLStmtList& stmts)
{
    if (rhs->type != Type::FLOAT) {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_res = allocate_int_register();
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::SGTStmt>(reg_res, reg_left, reg_right));

        deallocate_int_register(reg_left);
        deallocate_int_register(reg_right);

        return reg_res;
    } else {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::SLEDStmt>(reg_left, reg_right));

        deallocate_float_register(reg_left);
        deallocate_float_register(reg_right);

        std::shared_ptr<Register> reg1 = allocate_int_register();
        std::shared_ptr<Register> reg2 = allocate_int_register();

        stmts.push_back(std::make_shared<RTL::ILoadStmt>(reg1, std::make_shared<RTL::IntLit>(1)));
        stmts.push_back(std::make_shared<RTL::MoveStmt>(reg2, REG_zero));
        stmts.push_back(std::make_shared<RTL::MovFStmt>(reg2, reg1, std::make_shared<RTL::IntLit>(0)));

        deallocate_int_register(reg1);

        return reg2;
    }
}

std::shared_ptr<Register> TAC::GreaterEqualExpr::gen_rtl(RTLStmtList& stmts)
{
    if (rhs->type != Type::FLOAT) {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_res = allocate_int_register();
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::SGEStmt>(reg_res, reg_left, reg_right));

        deallocate_int_register(reg_left);
        deallocate_int_register(reg_right);

        return reg_res;
    } else {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::SLTDStmt>(reg_left, reg_right));

        deallocate_float_register(reg_left);
        deallocate_float_register(reg_right);

        std::shared_ptr<Register> reg1 = allocate_int_register();
        std::shared_ptr<Register> reg2 = allocate_int_register();

        stmts.push_back(std::make_shared<RTL::ILoadStmt>(reg1, std::make_shared<RTL::IntLit>(1)));
        stmts.push_back(std::make_shared<RTL::MoveStmt>(reg2, REG_zero));
        stmts.push_back(std::make_shared<RTL::MovFStmt>(reg2, reg1, std::make_shared<RTL::IntLit>(0)));

        deallocate_int_register(reg1);

        return reg2;
    }
}

std::shared_ptr<Register> TAC::LessExpr::gen_rtl(RTLStmtList& stmts)
{
    if (rhs->type != Type::FLOAT) {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_res = allocate_int_register();
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::SLTStmt>(reg_res, reg_left, reg_right));

        deallocate_int_register(reg_left);
        deallocate_int_register(reg_right);

        return reg_res;
    } else {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::SLTDStmt>(reg_left, reg_right));

        deallocate_float_register(reg_left);
        deallocate_float_register(reg_right);

        std::shared_ptr<Register> reg1 = allocate_int_register();
        std::shared_ptr<Register> reg2 = allocate_int_register();

        stmts.push_back(std::make_shared<RTL::ILoadStmt>(reg1, std::make_shared<RTL::IntLit>(1)));
        stmts.push_back(std::make_shared<RTL::MoveStmt>(reg2, REG_zero));
        stmts.push_back(std::make_shared<RTL::MovTStmt>(reg2, reg1, std::make_shared<RTL::IntLit>(0)));

        deallocate_int_register(reg1);

        return reg2;
    }
}

std::shared_ptr<Register> TAC::LessEqualExpr::gen_rtl(RTLStmtList& stmts)
{
    if (rhs->type != Type::FLOAT) {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_res = allocate_int_register();
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::SLEStmt>(reg_res, reg_left, reg_right));

        deallocate_int_register(reg_left);
        deallocate_int_register(reg_right);

        return reg_res;
    } else {
        std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
        std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

        stmts.push_back(std::make_shared<RTL::SLEDStmt>(reg_left, reg_right));

        deallocate_float_register(reg_left);
        deallocate_float_register(reg_right);

        std::shared_ptr<Register> reg1 = allocate_int_register();
        std::shared_ptr<Register> reg2 = allocate_int_register();

        stmts.push_back(std::make_shared<RTL::ILoadStmt>(reg1, std::make_shared<RTL::IntLit>(1)));
        stmts.push_back(std::make_shared<RTL::MoveStmt>(reg2, REG_zero));
        stmts.push_back(std::make_shared<RTL::MovTStmt>(reg2, reg1, std::make_shared<RTL::IntLit>(0)));

        deallocate_int_register(reg1);

        return reg2;
    }
}


std::shared_ptr<Register> TAC::NotExpr::gen_rtl(RTLStmtList& stmts)
{
    assert(lhs->type != Type::FLOAT);

    std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
    std::shared_ptr<Register> reg_res = allocate_int_register();

    stmts.push_back(std::make_shared<RTL::NotStmt>(reg_res, reg_left));

    deallocate_int_register(reg_left);

    return reg_res;
}

std::shared_ptr<Register> TAC::AndExpr::gen_rtl(RTLStmtList& stmts)
{
    assert(rhs->type != Type::FLOAT);

    std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
    std::shared_ptr<Register> reg_res = allocate_int_register();
    std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

    stmts.push_back(std::make_shared<RTL::AndStmt>(reg_res, reg_left, reg_right));

    deallocate_int_register(reg_left);
    deallocate_int_register(reg_right);

    return reg_res;
}

std::shared_ptr<Register> TAC::OrExpr::gen_rtl(RTLStmtList& stmts)
{
    assert(rhs->type != Type::FLOAT);

    std::shared_ptr<Register> reg_left = lhs->gen_rtl(stmts);
    std::shared_ptr<Register> reg_res = allocate_int_register();
    std::shared_ptr<Register> reg_right = rhs->gen_rtl(stmts);

    stmts.push_back(std::make_shared<RTL::OrStmt>(reg_res, reg_left, reg_right));

    deallocate_int_register(reg_left);
    deallocate_int_register(reg_right);

    return reg_res;
}


void TAC::IntLit::gen_push_rtl(RTLStmtList& stmts)
{
    std::shared_ptr<Register> reg = allocate_int_register();
    stmts.push_back(std::make_shared<RTL::ILoadStmt>(reg, std::make_shared<RTL::IntLit>(val)));
    stmts.push_back(std::make_shared<RTL::PushStmt>(reg, false));
    deallocate_int_register(reg);
}
void TAC::FloatLit::gen_push_rtl(RTLStmtList& stmts)
{
    std::shared_ptr<Register> reg = allocate_float_register();
    stmts.push_back(std::make_shared<RTL::ILoadDStmt>(reg, std::make_shared<RTL::FloatLit>(val)));
    stmts.push_back(std::make_shared<RTL::PushStmt>(reg, true));
    deallocate_float_register(reg);
}
void TAC::StrLit::gen_push_rtl(RTLStmtList& stmts)
{
    std::shared_ptr<Register> reg = allocate_int_register();
    std::string str_id = ctx.get_string_id(val);
    stmts.push_back(std::make_shared<RTL::LoadAddrStmt>(reg, std::make_shared<RTL::Mem>(str_id, true, 0)));
    stmts.push_back(std::make_shared<RTL::PushStmt>(reg, false));
    deallocate_int_register(reg);
}
void TAC::Sym::gen_push_rtl(RTLStmtList& stmts)
{
    if(type != Type::FLOAT) {
        if(in_mem){
            std::shared_ptr<Register> reg_new = allocate_int_register();
            stmts.push_back(std::make_shared<RTL::LoadStmt>(reg_new, std::make_shared<RTL::Mem>(name, is_global, fp_offset)));
            stmts.push_back(std::make_shared<RTL::PushStmt>(reg_new, false));
            deallocate_int_register(reg_new);
        } else{
            stmts.push_back(std::make_shared<RTL::PushStmt>(reg, false));
            deallocate_int_register(reg);
        }
    } else {
        if(in_mem){
            std::shared_ptr<Register> reg_new = allocate_float_register();
            stmts.push_back(std::make_shared<RTL::LoadDStmt>(reg_new, std::make_shared<RTL::Mem>(name, is_global, fp_offset)));
            stmts.push_back(std::make_shared<RTL::PushStmt>(reg_new, true));
            deallocate_float_register(reg_new);
        } else{
            stmts.push_back(std::make_shared<RTL::PushStmt>(reg, true));
            deallocate_float_register(reg);
        }
    }
}

void TAC::CallStmt::gen_rtl(RTLStmtList& stmts)
{
    e->gen_rtl(stmts);
}
std::shared_ptr<Register> TAC::FuncCallExpr::gen_rtl(RTLStmtList& stmts)
{
    if((int)type == -1){
        for(auto it = params.rbegin(); it != params.rend(); it++)
            (*it)->gen_push_rtl(stmts);
        stmts.push_back(std::make_shared<RTL::CallStmt>(func_name));
        for(size_t i = 0; i < params.size(); i++)
            if(params[i]->type != Type::FLOAT)
                stmts.push_back(std::make_shared<RTL::PopStmt>(false));
            else
                stmts.push_back(std::make_shared<RTL::PopStmt>(true));
        return nullptr;
    } else if(type != Type::FLOAT){
        for(auto it = params.rbegin(); it != params.rend(); it++)
            (*it)->gen_push_rtl(stmts);
        stmts.push_back(std::make_shared<RTL::AssignCallStmt>(REG_v1, func_name));
        for(size_t i = 0; i < params.size(); i++)
            if(params[i]->type != Type::FLOAT)
                stmts.push_back(std::make_shared<RTL::PopStmt>(false));
            else
                stmts.push_back(std::make_shared<RTL::PopStmt>(true));
        std::shared_ptr<Register> reg_res = allocate_int_register();
        stmts.push_back(std::make_shared<RTL::MoveStmt>(reg_res, REG_v1));
        return reg_res;
    } else{
        for(auto it = params.rbegin(); it != params.rend(); it++)
            (*it)->gen_push_rtl(stmts);
        stmts.push_back(std::make_shared<RTL::AssignCallStmt>(REG_f0, func_name));
        for(size_t i = 0; i < params.size(); i++)
            if(params[i]->type != Type::FLOAT)
                stmts.push_back(std::make_shared<RTL::PopStmt>(false));
            else
                stmts.push_back(std::make_shared<RTL::PopStmt>(true));
        std::shared_ptr<Register> reg_res = allocate_float_register();
        stmts.push_back(std::make_shared<RTL::MoveDStmt>(reg_res, REG_f0));
        return reg_res;
    }
}
std::shared_ptr<Register> TAC::FuncPtrCallExpr::gen_rtl(RTLStmtList& stmts)
{
    for(auto it = params.rbegin(); it != params.rend(); it++)
        (*it)->gen_push_rtl(stmts);
    std::shared_ptr<Register> func_ptr_reg = func_ptr->gen_rtl(stmts);
    if((int)type == -1){
        stmts.push_back(std::make_shared<RTL::CallPtrStmt>(func_ptr_reg));
        for(size_t i = 0; i < params.size(); i++)
            if(params[i]->type != Type::FLOAT)
                stmts.push_back(std::make_shared<RTL::PopStmt>(false));
            else
                stmts.push_back(std::make_shared<RTL::PopStmt>(true));
        return nullptr;
    } else if(type != Type::FLOAT){
        stmts.push_back(std::make_shared<RTL::AssignCallPtrStmt>(REG_v1, func_ptr_reg));
        for(size_t i = 0; i < params.size(); i++)
            if(params[i]->type != Type::FLOAT)
                stmts.push_back(std::make_shared<RTL::PopStmt>(false));
            else
                stmts.push_back(std::make_shared<RTL::PopStmt>(true));
        std::shared_ptr<Register> reg_res = allocate_int_register();
        stmts.push_back(std::make_shared<RTL::MoveStmt>(reg_res, REG_v1));
        return reg_res;
    } else{
        stmts.push_back(std::make_shared<RTL::AssignCallPtrStmt>(REG_f0, func_ptr_reg));
        for(size_t i = 0; i < params.size(); i++)
            if(params[i]->type != Type::FLOAT)
                stmts.push_back(std::make_shared<RTL::PopStmt>(false));
            else
                stmts.push_back(std::make_shared<RTL::PopStmt>(true));
        std::shared_ptr<Register> reg_res = allocate_float_register();
        stmts.push_back(std::make_shared<RTL::MoveDStmt>(reg_res, REG_f0));
        return reg_res;
    }
}

void TAC::ReturnStmt::gen_rtl(RTLStmtList& stmts)
{
    if(ret->type != Type::FLOAT){
        stmts.push_back(std::make_shared<RTL::LoadStmt>(REG_v1, std::make_shared<RTL::Mem>(ret->name, ret->is_global, ret->fp_offset)));
        stmts.push_back(std::make_shared<RTL::ReturnStmt>(REG_v1));
    } else{
        stmts.push_back(std::make_shared<RTL::LoadDStmt>(REG_f0, std::make_shared<RTL::Mem>(ret->name, ret->is_global, ret->fp_offset)));
        stmts.push_back(std::make_shared<RTL::ReturnStmt>(REG_f0));
    }
}

std::shared_ptr<RTL::Register> TAC::DerefExpr::gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts)
{
    // TODO @nilabha
    if(arg->type != Type::FLOAT){
        std::shared_ptr<Register> reg_new = allocate_int_register();
        stmts.push_back(std::make_shared<RTL::DerefStmt>(reg_new, arg->reg));
        deallocate_int_register(arg->reg);
        return reg_new;
    } else{
        std::shared_ptr<Register> reg_new = allocate_float_register();
        stmts.push_back(std::make_shared<RTL::DerefDStmt>(reg_new, arg->reg));
        deallocate_float_register(arg->reg);
        return reg_new;
    }
}
std::shared_ptr<RTL::Register> TAC::AddrExpr::gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts)
{
    // TODO @nilabha
    std::shared_ptr<Register> reg_new = allocate_int_register();
    // int fp_offset = arg->is_global ? -1 : arg->fp_offset;
    stmts.push_back(std::make_shared<RTL::GetAddrStmt>(reg_new, std::make_shared<RTL::Mem>(arg->name, arg->is_global, arg->fp_offset)));
    return reg_new;
}
void TAC::AddrAssignStmt::gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts){
    if(rhs->type != Type::FLOAT){
        std::shared_ptr<Register> rhs_reg = rhs->gen_rtl(stmts);
        stmts.push_back(std::make_shared<RTL::AddrAssignStmt>(lhs->reg, rhs_reg));
    } else{
        std::shared_ptr<Register> rhs_reg = rhs->gen_rtl(stmts);
        stmts.push_back(std::make_shared<RTL::AddrAssignDStmt>(lhs->reg, rhs_reg));
    }
}
