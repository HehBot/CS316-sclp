#include <rtl.h>
#include <asm.h>
#include <vector>
#include <cassert>

using Register = ASM::Register;
using ASMStmtList = std::vector<std::shared_ptr<ASM::Stmt>>;

extern std::string func_under_processing_name;

std::shared_ptr<Register> rtlreg2asmreg(std::shared_ptr<RTL::Register> rtl_reg){
    return std::make_shared<Register>(rtl_reg->name);
}


void RTL::Label::gen_asm(ASMStmtList& stmts){
    stmts.push_back(std::make_shared<ASM::LabelStmt>(name));
}
void RTL::GotoStmt::gen_asm(ASMStmtList& stmts){
    stmts.push_back(std::make_shared<ASM::JStmt>(label->name));
}
void RTL::BGTZStmt::gen_asm(ASMStmtList& stmts){
    stmts.push_back(std::make_shared<ASM::BGTZStmt>(rtlreg2asmreg(reg), label->name));
}

void RTL::WriteStmt::gen_asm(ASMStmtList& stmts){
    stmts.push_back(std::make_shared<ASM::SyscallStmt>());
}
void RTL::ReadStmt::gen_asm(ASMStmtList& stmts){
    stmts.push_back(std::make_shared<ASM::SyscallStmt>());
}

void RTL::CallStmt::gen_asm(ASMStmtList& stmts){
    stmts.push_back(std::make_shared<ASM::JalStmt>(func_name));
}
void RTL::AssignCallStmt::gen_asm(ASMStmtList& stmts){
    stmts.push_back(std::make_shared<ASM::JalStmt>(func_name));
}
void RTL::CallPtrStmt::gen_asm(ASMStmtList& stmts){
    stmts.push_back(std::make_shared<ASM::JalrStmt>(rtlreg2asmreg(func_ptr)));
}
void RTL::AssignCallPtrStmt::gen_asm(ASMStmtList& stmts){
    stmts.push_back(std::make_shared<ASM::JalrStmt>(rtlreg2asmreg(func_ptr)));
}
void RTL::ReturnStmt::gen_asm(ASMStmtList& stmts){
    stmts.push_back(std::make_shared<ASM::JStmt>("epilogue_"+func_under_processing_name));
}

void RTL::PopStmt::gen_asm(ASMStmtList& stmts){
    if(is_float)
        stmts.push_back(std::make_shared<ASM::AddStmt>(std::make_shared<ASM::Register>("sp"), std::make_shared<ASM::Register>("sp"), std::make_shared<ASM::IntLit>(8)));
    else
        stmts.push_back(std::make_shared<ASM::AddStmt>(std::make_shared<ASM::Register>("sp"), std::make_shared<ASM::Register>("sp"), std::make_shared<ASM::IntLit>(4)));
}
void RTL::PushStmt::gen_asm(ASMStmtList& stmts){
    if(is_float) {
        stmts.push_back(std::make_shared<ASM::SDStmt>(rtlreg2asmreg(reg), std::make_shared<ASM::Register>("sp"), -4));
        stmts.push_back(std::make_shared<ASM::SubStmt>(std::make_shared<ASM::Register>("sp"), std::make_shared<ASM::Register>("sp"), std::make_shared<ASM::IntLit>(8)));
    } else {
        stmts.push_back(std::make_shared<ASM::SWStmt>(rtlreg2asmreg(reg), std::make_shared<ASM::Register>("sp"), 0));
        stmts.push_back(std::make_shared<ASM::SubStmt>(std::make_shared<ASM::Register>("sp"), std::make_shared<ASM::Register>("sp"), std::make_shared<ASM::IntLit>(4)));
    }
}

void RTL::MoveStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    assert((l != nullptr) && (r != nullptr));
    stmts.push_back(std::make_shared<ASM::MovStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r)));
}
void RTL::MoveDStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    assert((l != nullptr) && (r != nullptr));
    stmts.push_back(std::make_shared<ASM::MovDStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r)));
}

void RTL::LoadStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Mem> r = std::dynamic_pointer_cast<RTL::Mem>(rhs);
    assert((l != nullptr) && (r != nullptr));
    if(r->is_global){
        stmts.push_back(std::make_shared<ASM::LWStmt>(rtlreg2asmreg(l), std::make_shared<ASM::Mem>(r->name), -1));
    } else{
        stmts.push_back(std::make_shared<ASM::LWStmt>(rtlreg2asmreg(l), std::make_shared<ASM::Register>("fp"), r->fp_offset));
    }
}
void RTL::ILoadStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::IntLit> r = std::dynamic_pointer_cast<RTL::IntLit>(rhs);
    assert((l != nullptr) && (r != nullptr));
    stmts.push_back(std::make_shared<ASM::LIStmt>(rtlreg2asmreg(l), r->val));
}
void RTL::LoadDStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Mem> r = std::dynamic_pointer_cast<RTL::Mem>(rhs);
    assert((l != nullptr) && (r != nullptr));
    if(r->is_global){
        stmts.push_back(std::make_shared<ASM::LDStmt>(rtlreg2asmreg(l), std::make_shared<ASM::Mem>(r->name), -1));
    } else{
        stmts.push_back(std::make_shared<ASM::LDStmt>(rtlreg2asmreg(l), std::make_shared<ASM::Register>("fp"), r->fp_offset));
    }
}
void RTL::ILoadDStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::FloatLit> r = std::dynamic_pointer_cast<RTL::FloatLit>(rhs);
    assert((l != nullptr) && (r != nullptr));
    stmts.push_back(std::make_shared<ASM::LIDStmt>(rtlreg2asmreg(l), r->val));
}
void RTL::LoadAddrStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Mem> r = std::dynamic_pointer_cast<RTL::Mem>(rhs);
    assert((l != nullptr) && (r != nullptr));
    stmts.push_back(std::make_shared<ASM::LAStmt>(rtlreg2asmreg(l), std::make_shared<ASM::Mem>(r->name)));
}

void RTL::StoreStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Mem> l = std::dynamic_pointer_cast<RTL::Mem>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    // std::cout << l->is_global << std::endl;
    assert(l != nullptr && r != nullptr);
    if(l->is_global){
        stmts.push_back(std::make_shared<ASM::SWStmt>(rtlreg2asmreg(r), std::make_shared<ASM::Mem>(l->name), -1));
    } else{
        // std::cout << l->fp_offset << std::endl;
        stmts.push_back(std::make_shared<ASM::SWStmt>(rtlreg2asmreg(r), std::make_shared<ASM::Register>("fp"), l->fp_offset));
    }
}
void RTL::StoreDStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Mem> l = std::dynamic_pointer_cast<RTL::Mem>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    // std::cout << l->is_global << std::endl;
    assert(l != nullptr && r != nullptr);
    if(l->is_global){
        stmts.push_back(std::make_shared<ASM::SDStmt>(rtlreg2asmreg(r), std::make_shared<ASM::Mem>(l->name), -1));
    } else{
        // std::cout << l->fp_offset << std::endl;
        stmts.push_back(std::make_shared<ASM::SDStmt>(rtlreg2asmreg(r), std::make_shared<ASM::Register>("fp"), l->fp_offset));
    }
}

void RTL::UMinusStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    assert(l != nullptr && r != nullptr);
    stmts.push_back(std::make_shared<ASM::NegStmt>(rtlreg2asmreg(l),rtlreg2asmreg(r)));
}
void RTL::UMinusDStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    assert(l != nullptr && r != nullptr);
    stmts.push_back(std::make_shared<ASM::NegDStmt>(rtlreg2asmreg(l),rtlreg2asmreg(r)));
}

void RTL::NotStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    assert((l != nullptr) && (r != nullptr));
    stmts.push_back(std::make_shared<ASM::XorIStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), std::make_shared<ASM::IntLit>(1)));
}

void RTL::SLTDStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    assert((l != nullptr) && (r != nullptr));
    stmts.push_back(std::make_shared<ASM::CLTDStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r)));
}
void RTL::SLEDStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    assert((l != nullptr) && (r != nullptr));
    stmts.push_back(std::make_shared<ASM::CLEDStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r)));
}
void RTL::SEQDStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    assert((l != nullptr) && (r != nullptr));
    stmts.push_back(std::make_shared<ASM::CEQDStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r)));
}

void RTL::AddStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    std::shared_ptr<RTL::Register> rr = std::dynamic_pointer_cast<RTL::Register>(rrhs);
    assert((l != nullptr) && (r != nullptr) && (rr != nullptr));
    stmts.push_back(std::make_shared<ASM::AddStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), rtlreg2asmreg(rr)));
}
void RTL::SubStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    std::shared_ptr<RTL::Register> rr = std::dynamic_pointer_cast<RTL::Register>(rrhs);
    assert((l != nullptr) && (r != nullptr) && (rr != nullptr));
    stmts.push_back(std::make_shared<ASM::SubStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), rtlreg2asmreg(rr)));
}
void RTL::MulStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    std::shared_ptr<RTL::Register> rr = std::dynamic_pointer_cast<RTL::Register>(rrhs);
    assert((l != nullptr) && (r != nullptr) && (rr != nullptr));
    stmts.push_back(std::make_shared<ASM::MulStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), rtlreg2asmreg(rr)));
}
void RTL::DivStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    std::shared_ptr<RTL::Register> rr = std::dynamic_pointer_cast<RTL::Register>(rrhs);
    assert((l != nullptr) && (r != nullptr) && (rr != nullptr));
    stmts.push_back(std::make_shared<ASM::DivStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), rtlreg2asmreg(rr)));
}

void RTL::AddDStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    std::shared_ptr<RTL::Register> rr = std::dynamic_pointer_cast<RTL::Register>(rrhs);
    assert((l != nullptr) && (r != nullptr) && (rr != nullptr));
    stmts.push_back(std::make_shared<ASM::AddDStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), rtlreg2asmreg(rr)));
}
void RTL::SubDStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    std::shared_ptr<RTL::Register> rr = std::dynamic_pointer_cast<RTL::Register>(rrhs);
    assert((l != nullptr) && (r != nullptr) && (rr != nullptr));
    stmts.push_back(std::make_shared<ASM::SubDStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), rtlreg2asmreg(rr)));
}
void RTL::MulDStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    std::shared_ptr<RTL::Register> rr = std::dynamic_pointer_cast<RTL::Register>(rrhs);
    assert((l != nullptr) && (r != nullptr) && (rr != nullptr));
    stmts.push_back(std::make_shared<ASM::MulDStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), rtlreg2asmreg(rr)));
}
void RTL::DivDStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    std::shared_ptr<RTL::Register> rr = std::dynamic_pointer_cast<RTL::Register>(rrhs);
    assert((l != nullptr) && (r != nullptr) && (rr != nullptr));
    stmts.push_back(std::make_shared<ASM::DivDStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), rtlreg2asmreg(rr)));
}

void RTL::SLTStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    std::shared_ptr<RTL::Register> rr = std::dynamic_pointer_cast<RTL::Register>(rrhs);
    assert((l != nullptr) && (r != nullptr) && (rr != nullptr));
    stmts.push_back(std::make_shared<ASM::SLTStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), rtlreg2asmreg(rr)));
}
void RTL::SLEStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    std::shared_ptr<RTL::Register> rr = std::dynamic_pointer_cast<RTL::Register>(rrhs);
    assert((l != nullptr) && (r != nullptr) && (rr != nullptr));
    stmts.push_back(std::make_shared<ASM::SLEStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), rtlreg2asmreg(rr)));
}
void RTL::SGTStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    std::shared_ptr<RTL::Register> rr = std::dynamic_pointer_cast<RTL::Register>(rrhs);
    assert((l != nullptr) && (r != nullptr) && (rr != nullptr));
    stmts.push_back(std::make_shared<ASM::SGTStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), rtlreg2asmreg(rr)));
}
void RTL::SGEStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    std::shared_ptr<RTL::Register> rr = std::dynamic_pointer_cast<RTL::Register>(rrhs);
    assert((l != nullptr) && (r != nullptr) && (rr != nullptr));
    stmts.push_back(std::make_shared<ASM::SGEStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), rtlreg2asmreg(rr)));
}
void RTL::SEQStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    std::shared_ptr<RTL::Register> rr = std::dynamic_pointer_cast<RTL::Register>(rrhs);
    assert((l != nullptr) && (r != nullptr) && (rr != nullptr));
    stmts.push_back(std::make_shared<ASM::SEQStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), rtlreg2asmreg(rr)));
}
void RTL::SNEStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    std::shared_ptr<RTL::Register> rr = std::dynamic_pointer_cast<RTL::Register>(rrhs);
    assert((l != nullptr) && (r != nullptr) && (rr != nullptr));
    stmts.push_back(std::make_shared<ASM::SNEStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), rtlreg2asmreg(rr)));
}

void RTL::OrStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    std::shared_ptr<RTL::Register> rr = std::dynamic_pointer_cast<RTL::Register>(rrhs);
    assert((l != nullptr) && (r != nullptr) && (rr != nullptr));
    stmts.push_back(std::make_shared<ASM::OrStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), rtlreg2asmreg(rr)));
}
void RTL::AndStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    std::shared_ptr<RTL::Register> rr = std::dynamic_pointer_cast<RTL::Register>(rrhs);
    assert((l != nullptr) && (r != nullptr) && (rr != nullptr));
    stmts.push_back(std::make_shared<ASM::AndStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), rtlreg2asmreg(rr)));
}

void RTL::MovTStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    std::shared_ptr<RTL::IntLit> rr = std::dynamic_pointer_cast<RTL::IntLit>(rrhs);
    assert((l != nullptr) && (r != nullptr) && (rr != nullptr));
    stmts.push_back(std::make_shared<ASM::MovTStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), std::make_shared<ASM::IntLit>(rr->val)));
}
void RTL::MovFStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    std::shared_ptr<RTL::IntLit> rr = std::dynamic_pointer_cast<RTL::IntLit>(rrhs);
    assert((l != nullptr) && (r != nullptr) && (rr != nullptr));
    stmts.push_back(std::make_shared<ASM::MovFStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r), std::make_shared<ASM::IntLit>(rr->val)));
}


void RTL::GetAddrStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Mem> r = std::dynamic_pointer_cast<RTL::Mem>(rhs);
    assert((l != nullptr) && (r != nullptr));
    stmts.push_back(std::make_shared<ASM::LAAddrStmt>(rtlreg2asmreg(l), std::make_shared<ASM::Mem>(r->name), r->fp_offset));
}

void RTL::DerefStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    assert((l != nullptr) && (r != nullptr));
    stmts.push_back(std::make_shared<ASM::DerefStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r)));
}
void RTL::DerefDStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    assert((l != nullptr) && (r != nullptr));
    stmts.push_back(std::make_shared<ASM::DerefDStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r)));
}
void RTL::AddrAssignStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    assert((l != nullptr) && (r != nullptr));
    stmts.push_back(std::make_shared<ASM::DRFSStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r)));
}
void RTL::AddrAssignDStmt::gen_asm(ASMStmtList& stmts){
    std::shared_ptr<RTL::Register> l = std::dynamic_pointer_cast<RTL::Register>(lhs);
    std::shared_ptr<RTL::Register> r = std::dynamic_pointer_cast<RTL::Register>(rhs);
    assert((l != nullptr) && (r != nullptr));
    stmts.push_back(std::make_shared<ASM::DRFSDStmt>(rtlreg2asmreg(l), rtlreg2asmreg(r)));
}