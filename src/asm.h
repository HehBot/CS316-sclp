#ifndef ASM_H
#define ASM_H

#include <iostream>
#include <memory>

namespace ASM {
    struct Base{
        virtual void print(std::ostream&) const = 0;
    };

    struct Val: public Base {
    };
    struct Register: public Val {
        std::string name;
        Register(std::string n) : name(n) {}
        virtual void print(std::ostream& o) const override
        {
            o << "$" << name;
        }
    };

    struct Mem: public Val {
        std::string name;
        Mem(std::string n) : name(n) {}
        virtual void print(std::ostream& o) const override
        {
            o << name;
        }
    };
    struct IntLit: public Val {
        size_t val;
        IntLit(size_t v) : val(v) {}
        virtual void print(std::ostream& o) const override
        {
            o << val;
        }
    };
    struct FloatLit: public Val {
        double val;
        FloatLit(double v) : val(v) {}
        virtual void print(std::ostream& o) const override
        {
            o << val;
        }
    };

    struct Stmt : public Base {
    };

    struct LabelStmt : public Stmt{
        std::string label;
        LabelStmt(std::string label) : label(label) {}
        virtual void print(std::ostream& o) const override
        {
            o << label << ":\n";
        }
    };

    struct SyscallStmt : public Stmt{
        virtual void print(std::ostream& o) const override
        {
            o << "\tsyscall\n";
        }
    };

    struct JRStmt : public Stmt{
        std::shared_ptr<Register> reg;
        JRStmt(std::shared_ptr<Register> reg) : reg(reg) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tjr ";
            reg->print(o);
            o << "\n";
        }
    };
    struct JStmt : public Stmt{
        std::string label;
        JStmt(std::string label) : label(label) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tj " << label << "\n";
        }
    };
    struct JalStmt : public Stmt{
        std::string func_name;
        JalStmt(std::string func_name) : func_name(func_name) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tjal " << func_name << "\n";
        }
    };
    struct JalrStmt : public Stmt{
        std::shared_ptr<Register> func_ptr;
        JalrStmt(std::shared_ptr<Register> func_ptr) : func_ptr(func_ptr) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tjalr ";
            func_ptr->print(o);
            o << "\n";
        }
    };

    struct BGTZStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::string label;
        BGTZStmt(std::shared_ptr<Register> reg, std::string label) : reg(reg), label(label) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tbgtz ";
            reg->print(o);
            o << ", " << label << "\n";
        }
    };
    struct NegStmt : public Stmt{
        std::shared_ptr<Register> reg1, reg2;
        NegStmt(std::shared_ptr<Register> reg1, std::shared_ptr<Register> reg2) : reg1(reg1), reg2(reg2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tneg ";
            reg1->print(o);
            o << ", ";
            reg2->print(o);
            o << "\n";
        }
    };
    struct NegDStmt : public Stmt{
        std::shared_ptr<Register> reg1, reg2;
        NegDStmt(std::shared_ptr<Register> reg1, std::shared_ptr<Register> reg2) : reg1(reg1), reg2(reg2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tneg.d ";
            reg1->print(o);
            o << ", ";
            reg2->print(o);
            o << "\n";
        }
    };

    struct MovStmt : public Stmt{
        std::shared_ptr<Register> reg1, reg2;
        MovStmt(std::shared_ptr<Register> reg1, std::shared_ptr<Register> reg2) : reg1(reg1), reg2(reg2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tmove ";
            reg1->print(o);
            o << ", ";
            reg2->print(o);
            o << "\n";
        }
    };
    struct MovDStmt : public Stmt{
        std::shared_ptr<Register> reg1, reg2;
        MovDStmt(std::shared_ptr<Register> reg1, std::shared_ptr<Register> reg2) : reg1(reg1), reg2(reg2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tmov.d ";
            reg1->print(o);
            o << ", ";
            reg2->print(o);
            o << "\n";
        }
    };


    struct SWStmt : public Stmt{
    public:
        std::shared_ptr<Register> reg1;
        std::shared_ptr<Val> v;
        int offset;
        SWStmt(std::shared_ptr<Register> reg1, std::shared_ptr<Val> v, int offset) : reg1(reg1), v(v), offset(offset) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tsw ";
            reg1->print(o);
            o << ", ";
            if(offset == -1)
                v->print(o);
            else{
                o << offset << "(";
                v->print(o);
                o << ")";
            }
            o << "\n";
        } 
    };
    struct SDStmt : public Stmt{
    public:
        std::shared_ptr<Register> reg1;
        std::shared_ptr<Val> v;
        int offset;
        SDStmt(std::shared_ptr<Register> reg1, std::shared_ptr<Val> v, int offset) : reg1(reg1), v(v), offset(offset) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\ts.d ";
            reg1->print(o);
            o << ", ";
            if(offset == -1)
                v->print(o);
            else{
                o << offset << "(";
                v->print(o);
                o << ")";
            }
            o << "\n";
        } 
    };
    struct LWStmt : public Stmt{
    public:
        std::shared_ptr<Register> reg1;
        std::shared_ptr<Val> v;
        int offset;
        LWStmt(std::shared_ptr<Register> reg1, std::shared_ptr<Val> v, int offset) : reg1(reg1), v(v), offset(offset) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tlw ";
            reg1->print(o);
            o << ", ";
            if(offset == -1)
                v->print(o);
            else{
                o << offset << "(";
                v->print(o);
                o << ")";
            }
            o << "\n";
        } 
    };
    struct LDStmt : public Stmt{
    public:
        std::shared_ptr<Register> reg1;
        std::shared_ptr<Val> v;
        int offset;
        LDStmt(std::shared_ptr<Register> reg1, std::shared_ptr<Val> v, int offset) : reg1(reg1), v(v), offset(offset) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tl.d ";
            reg1->print(o);
            o << ", ";
            if(offset == -1)
                v->print(o);
            else{
                o << offset << "(";
                v->print(o);
                o << ")";
            }
            o << "\n";
        } 
    };

    struct LIStmt : public Stmt{
    public:
        std::shared_ptr<Register> reg;
        size_t val;
        LIStmt(std::shared_ptr<Register> reg, size_t val) : reg(reg), val(val) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tli ";
            reg->print(o);
            o << ", " << val;
            o << "\n";
        } 
    };
    struct LIDStmt : public Stmt{
    public:
        std::shared_ptr<Register> reg;
        double val;
        LIDStmt(std::shared_ptr<Register> reg, double val) : reg(reg), val(val) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tli.d ";
            reg->print(o);
            o << ", " << val;
            o << "\n";
        } 
    };
    struct LAStmt : public Stmt{
    public:
        std::shared_ptr<Register> reg;
        std::shared_ptr<Mem> mem;
        LAStmt(std::shared_ptr<Register> reg, std::shared_ptr<Mem> mem) : reg(reg), mem(mem) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tla ";
            reg->print(o);
            o << ", ";
            mem->print(o);
            o << "\n";
        } 
    };

    struct AddStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::shared_ptr<Val> val1, val2;
        AddStmt(std::shared_ptr<Register> reg, std::shared_ptr<Val> val1, std::shared_ptr<Val> val2) : reg(reg), val1(val1), val2(val2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tadd ";
            reg->print(o);
            o << ", ";
            val1->print(o);
            o << ", ";
            val2->print(o);
            o << "\n";
        }
    };
    struct AddDStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::shared_ptr<Val> val1, val2;
        AddDStmt(std::shared_ptr<Register> reg, std::shared_ptr<Val> val1, std::shared_ptr<Val> val2) : reg(reg), val1(val1), val2(val2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tadd.d ";
            reg->print(o);
            o << ", ";
            val1->print(o);
            o << ", ";
            val2->print(o);
            o << "\n";
        }
    };
    struct SubStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::shared_ptr<Val> val1, val2;
        SubStmt(std::shared_ptr<Register> reg, std::shared_ptr<Val> val1, std::shared_ptr<Val> val2) : reg(reg), val1(val1), val2(val2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tsub ";
            reg->print(o);
            o << ", ";
            val1->print(o);
            o << ", ";
            val2->print(o);
            o << "\n";
        }
    };
    struct SubDStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::shared_ptr<Val> val1, val2;
        SubDStmt(std::shared_ptr<Register> reg, std::shared_ptr<Val> val1, std::shared_ptr<Val> val2) : reg(reg), val1(val1), val2(val2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tsub.d ";
            reg->print(o);
            o << ", ";
            val1->print(o);
            o << ", ";
            val2->print(o);
            o << "\n";
        }
    };
    struct MulStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::shared_ptr<Val> val1, val2;
        MulStmt(std::shared_ptr<Register> reg, std::shared_ptr<Val> val1, std::shared_ptr<Val> val2) : reg(reg), val1(val1), val2(val2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tmul ";
            reg->print(o);
            o << ", ";
            val1->print(o);
            o << ", ";
            val2->print(o);
            o << "\n";
        }
    };
    struct MulDStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::shared_ptr<Val> val1, val2;
        MulDStmt(std::shared_ptr<Register> reg, std::shared_ptr<Val> val1, std::shared_ptr<Val> val2) : reg(reg), val1(val1), val2(val2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tmul.d ";
            reg->print(o);
            o << ", ";
            val1->print(o);
            o << ", ";
            val2->print(o);
            o << "\n";
        }
    };
    struct DivStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::shared_ptr<Val> val1, val2;
        DivStmt(std::shared_ptr<Register> reg, std::shared_ptr<Val> val1, std::shared_ptr<Val> val2) : reg(reg), val1(val1), val2(val2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tdiv ";
            reg->print(o);
            o << ", ";
            val1->print(o);
            o << ", ";
            val2->print(o);
            o << "\n";
        }
    };
    struct DivDStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::shared_ptr<Val> val1, val2;
        DivDStmt(std::shared_ptr<Register> reg, std::shared_ptr<Val> val1, std::shared_ptr<Val> val2) : reg(reg), val1(val1), val2(val2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tdiv.d ";
            reg->print(o);
            o << ", ";
            val1->print(o);
            o << ", ";
            val2->print(o);
            o << "\n";
        }
    };

    struct SLTStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::shared_ptr<Val> val1, val2;
        SLTStmt(std::shared_ptr<Register> reg, std::shared_ptr<Val> val1, std::shared_ptr<Val> val2) : reg(reg), val1(val1), val2(val2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tslt ";
            reg->print(o);
            o << ", ";
            val1->print(o);
            o << ", ";
            val2->print(o);
            o << "\n";
        }
    };
    struct SLEStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::shared_ptr<Val> val1, val2;
        SLEStmt(std::shared_ptr<Register> reg, std::shared_ptr<Val> val1, std::shared_ptr<Val> val2) : reg(reg), val1(val1), val2(val2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tsle ";
            reg->print(o);
            o << ", ";
            val1->print(o);
            o << ", ";
            val2->print(o);
            o << "\n";
        }
    };
    struct SGTStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::shared_ptr<Val> val1, val2;
        SGTStmt(std::shared_ptr<Register> reg, std::shared_ptr<Val> val1, std::shared_ptr<Val> val2) : reg(reg), val1(val1), val2(val2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tsgt ";
            reg->print(o);
            o << ", ";
            val1->print(o);
            o << ", ";
            val2->print(o);
            o << "\n";
        }
    };
    struct SGEStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::shared_ptr<Val> val1, val2;
        SGEStmt(std::shared_ptr<Register> reg, std::shared_ptr<Val> val1, std::shared_ptr<Val> val2) : reg(reg), val1(val1), val2(val2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tsge ";
            reg->print(o);
            o << ", ";
            val1->print(o);
            o << ", ";
            val2->print(o);
            o << "\n";
        }
    };
    struct SNEStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::shared_ptr<Val> val1, val2;
        SNEStmt(std::shared_ptr<Register> reg, std::shared_ptr<Val> val1, std::shared_ptr<Val> val2) : reg(reg), val1(val1), val2(val2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tsne ";
            reg->print(o);
            o << ", ";
            val1->print(o);
            o << ", ";
            val2->print(o);
            o << "\n";
        }
    };
    struct SEQStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::shared_ptr<Val> val1, val2;
        SEQStmt(std::shared_ptr<Register> reg, std::shared_ptr<Val> val1, std::shared_ptr<Val> val2) : reg(reg), val1(val1), val2(val2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tseq ";
            reg->print(o);
            o << ", ";
            val1->print(o);
            o << ", ";
            val2->print(o);
            o << "\n";
        }
    };

    struct OrStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::shared_ptr<Val> val1, val2;
        OrStmt(std::shared_ptr<Register> reg, std::shared_ptr<Val> val1, std::shared_ptr<Val> val2) : reg(reg), val1(val1), val2(val2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tor ";
            reg->print(o);
            o << ", ";
            val1->print(o);
            o << ", ";
            val2->print(o);
            o << "\n";
        }
    };
    struct AndStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::shared_ptr<Val> val1, val2;
        AndStmt(std::shared_ptr<Register> reg, std::shared_ptr<Val> val1, std::shared_ptr<Val> val2) : reg(reg), val1(val1), val2(val2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tand ";
            reg->print(o);
            o << ", ";
            val1->print(o);
            o << ", ";
            val2->print(o);
            o << "\n";
        }
    };
    struct XorIStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::shared_ptr<Val> val1, val2;
        XorIStmt(std::shared_ptr<Register> reg, std::shared_ptr<Val> val1, std::shared_ptr<Val> val2) : reg(reg), val1(val1), val2(val2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\txori ";
            reg->print(o);
            o << ", ";
            val1->print(o);
            o << ", ";
            val2->print(o);
            o << "\n";
        }
    };

    struct CLTDStmt : public Stmt{
        std::shared_ptr<Register> reg1, reg2;
        CLTDStmt(std::shared_ptr<Register> reg1, std::shared_ptr<Register> reg2) : reg1(reg1), reg2(reg2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tc.lt.d ";
            reg1->print(o);
            o << ", ";
            reg2->print(o);
            o << "\n";
        }
    };
    struct CLEDStmt : public Stmt{
        std::shared_ptr<Register> reg1, reg2;
        CLEDStmt(std::shared_ptr<Register> reg1, std::shared_ptr<Register> reg2) : reg1(reg1), reg2(reg2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tc.le.d ";
            reg1->print(o);
            o << ", ";
            reg2->print(o);
            o << "\n";
        }
    };
    struct CEQDStmt : public Stmt{
        std::shared_ptr<Register> reg1, reg2;
        CEQDStmt(std::shared_ptr<Register> reg1, std::shared_ptr<Register> reg2) : reg1(reg1), reg2(reg2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tc.eq.d ";
            reg1->print(o);
            o << ", ";
            reg2->print(o);
            o << "\n";
        }
    };

    struct MovTStmt : public Stmt{
        std::shared_ptr<Register> reg1, reg2;
        std::shared_ptr<IntLit> val;
        MovTStmt(std::shared_ptr<Register> reg1, std::shared_ptr<Register> reg2, std::shared_ptr<IntLit> val) : reg1(reg1), reg2(reg2), val(val) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tmovt ";
            reg1->print(o);
            o << ", ";
            reg2->print(o);
            o << ", ";
            val->print(o);
            o << "\n";
        }
    };
    struct MovFStmt : public Stmt{
        std::shared_ptr<Register> reg1, reg2;
        std::shared_ptr<IntLit> val;
        MovFStmt(std::shared_ptr<Register> reg1, std::shared_ptr<Register> reg2, std::shared_ptr<IntLit> val) : reg1(reg1), reg2(reg2), val(val) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tmovf ";
            reg1->print(o);
            o << ", ";
            reg2->print(o);
            o << ", ";
            val->print(o);
            o << "\n";
        }
    };

    struct LAAddrStmt : public Stmt{
        std::shared_ptr<Register> reg;
        std::shared_ptr<Val> v;
        int offset;

        LAAddrStmt(std::shared_ptr<Register> reg, std::shared_ptr<Val> v, int offset) : reg(reg), v(v), offset(offset) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tla ";
            reg->print(o);
            o << ", ";
            if(offset == -1)
                v->print(o);
            else{
                o << offset << "($fp)";
            }
            o << "\n";
        }
    };

    struct DerefStmt : public Stmt{
        std::shared_ptr<Register> reg1, reg2;

        DerefStmt(std::shared_ptr<Register> reg1, std::shared_ptr<Register> reg2) : reg1(reg1), reg2(reg2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tlw ";
            reg1->print(o);
            o << ", 0(";
            reg2->print(o);
            o << ")\n";
        }
    };
    struct DerefDStmt : public Stmt{
        std::shared_ptr<Register> reg1, reg2;

        DerefDStmt(std::shared_ptr<Register> reg1, std::shared_ptr<Register> reg2) : reg1(reg1), reg2(reg2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tl.d ";
            reg1->print(o);
            o << ", 0(";
            reg2->print(o);
            o << ")\n";
        }
    };

    struct DRFSStmt : public Stmt{
        std::shared_ptr<Register> reg1, reg2;

        DRFSStmt(std::shared_ptr<Register> reg1, std::shared_ptr<Register> reg2) : reg1(reg1), reg2(reg2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tsw ";
            reg2->print(o);
            o << ", 0(";
            reg1->print(o);
            o << ")\n";
        }
    };
    struct DRFSDStmt : public Stmt{
        std::shared_ptr<Register> reg1, reg2;

        DRFSDStmt(std::shared_ptr<Register> reg1, std::shared_ptr<Register> reg2) : reg1(reg1), reg2(reg2) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\ts.d ";
            reg2->print(o);
            o << ", 0(";
            reg1->print(o);
            o << ")\n";
        }
    };
}

#endif // ASM_H
