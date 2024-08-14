#ifndef RTL_H
#define RTL_H

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <asm.h>

void print_string_escapes(std::string, std::ostream&);

namespace RTL {
    class Context {
    public:
        std::vector<std::string> string_store;
        std::string get_string_id(std::string val);
    };

    void reset();

    struct Base {
        virtual void print(std::ostream&) const = 0;
    };

    struct Val: public Base {
    };
    struct Register: public Val {
        std::string name;
        Register(std::string n) : name(n) {}
        virtual void print(std::ostream& o) const override
        {
            o << name;
        }
    };


    struct Mem: public Val {
        std::string name;
        bool is_global;
        ssize_t fp_offset;
        Mem(std::string n, bool is_global, ssize_t fp_offset) : name(n), is_global(is_global), fp_offset(fp_offset) {}
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
    struct StrLit: public Val {
        std::string val;
        StrLit(std::string v) : val(v) {}
        virtual void print(std::ostream& o) const override
        {
            o << '"';
            print_string_escapes(val, o);
            o << '"';
        }
    };

    struct Stmt : public Base {
        virtual void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) = 0;
    };
    struct Label: public Stmt {
        std::string name;
        Label(std::string n) : name(n) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\n  " << name << ":      \n";
        }
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct GotoStmt: public Stmt {
        std::shared_ptr<Label> label;
        GotoStmt(std::shared_ptr<Label> l) : label(l) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tgoto:        " << label->name << "\n";
        }
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct BGTZStmt: public Stmt {
        std::shared_ptr<Register> reg;
        std::shared_ptr<Label> label;
        BGTZStmt(std::shared_ptr<Register> reg, std::shared_ptr<Label> l) : reg(reg), label(l) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tbgtz:        ";
            reg->print(o);
            o << " , " << label->name << "\n";
        }
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };

    struct WriteStmt: public Stmt {
        virtual void print(std::ostream& o) const override
        {
            o << "\twrite        \n";
        }
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct ReadStmt: public Stmt {
        virtual void print(std::ostream& o) const override
        {
            o << "\tread         \n";
        }
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };

    struct CallStmt: public Stmt{
    private:
        std::string func_name;
    public:
        CallStmt(std::string func_name) : func_name(func_name) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tcall " << func_name << "\n";
        }
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct AssignCallStmt: public Stmt{
    private:
        std::shared_ptr<Register> reg;
        std::string func_name;
    public:
        AssignCallStmt(std::shared_ptr<Register> reg, std::string func_name) : reg(reg), func_name(func_name) {}
        virtual void print(std::ostream& o) const override
        {
            o << '\t';
            reg->print(o);
            o << " = call " << func_name << "\n";
        }
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };

    struct CallPtrStmt: public Stmt{
    private:
        std::shared_ptr<Register> func_ptr;
    public:
        CallPtrStmt(std::shared_ptr<Register> func_ptr) : func_ptr(func_ptr) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tcallptr ";
            func_ptr->print(o);
            o << "\n";
        }
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct AssignCallPtrStmt: public Stmt{
    private:
        std::shared_ptr<Register> reg;
        std::shared_ptr<Register> func_ptr;
    public:
        AssignCallPtrStmt(std::shared_ptr<Register> reg, std::shared_ptr<Register> func_ptr) : reg(reg), func_ptr(func_ptr) {}
        virtual void print(std::ostream& o) const override
        {
            o << '\t';
            reg->print(o);
            o << " = callptr ";
            func_ptr->print(o);
            o << "\n";
        }
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    
    struct ReturnStmt: public Stmt{
    private:
        std::shared_ptr<Register> return_reg;
    public:
        ReturnStmt(std::shared_ptr<Register> reg) : return_reg(reg) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\treturn      ";
            return_reg->print(o);
            o << '\n';
        }
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };

    struct PopStmt: public Stmt{
    private:
        bool is_float;
    public:
        PopStmt(bool is_float) : is_float(is_float) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tpop\n";
        }
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct PushStmt: public Stmt{
    private:
        std::shared_ptr<Register> reg;
        bool is_float;
    public:
        PushStmt(std::shared_ptr<Register> reg, bool is_float) : reg(reg), is_float(is_float) {}
        virtual void print(std::ostream& o) const override
        {
            o << "\tpush:        ";
            reg->print(o);
            o << '\n';
        }
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };

    struct BinaryStmt: public Stmt {
    private:
        std::string const cmd;
        std::string const print_op;
    public:
        std::shared_ptr<Val> lhs, rhs;
        BinaryStmt(std::string const cmd, std::shared_ptr<Val> l, std::shared_ptr<Val> r, std::string print_op) : cmd(cmd), print_op(print_op), lhs(l), rhs(r) {}
        virtual void print(std::ostream& o) const override
        {
            o << cmd;
            lhs->print(o);
            o << " " << print_op << " ";
            rhs->print(o);
            o << "\n";
        }
    };

    struct MoveStmt: public BinaryStmt {
        MoveStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r) : BinaryStmt("\tmove:        ", l, r, "<-") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct MoveDStmt: public BinaryStmt {
        MoveDStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r) : BinaryStmt("\tmove.d:      ", l, r, "<-") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };

    struct LoadStmt: public BinaryStmt {
        LoadStmt(std::shared_ptr<Register> l, std::shared_ptr<Mem> r) : BinaryStmt("\tload:        ", l, r, "<-") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct ILoadStmt: public BinaryStmt {
        ILoadStmt(std::shared_ptr<Register> l, std::shared_ptr<IntLit> r) : BinaryStmt("\tiLoad:       " ,l, r, "<-") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct LoadDStmt: public BinaryStmt {
        LoadDStmt(std::shared_ptr<Register> l, std::shared_ptr<Mem> r) : BinaryStmt("\tload.d:      ", l, r, "<-") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct ILoadDStmt: public BinaryStmt {
        ILoadDStmt(std::shared_ptr<Register> l, std::shared_ptr<FloatLit> r) : BinaryStmt("\tiLoad.d:     ", l, r, "<-") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct LoadAddrStmt: public BinaryStmt {
        LoadAddrStmt(std::shared_ptr<Register> l, std::shared_ptr<Mem> r) : BinaryStmt("\tload_addr:   ", l, r, "<-") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };

    struct StoreStmt: public BinaryStmt {
        StoreStmt(std::shared_ptr<Mem> l, std::shared_ptr<Register> r) : BinaryStmt("\tstore:       ", l, r, "<-") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct StoreDStmt: public BinaryStmt {
        StoreDStmt(std::shared_ptr<Mem> l, std::shared_ptr<Register> r) : BinaryStmt("\tstore.d:     ", l, r, "<-") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };

    struct UMinusStmt: public BinaryStmt {
        UMinusStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r) : BinaryStmt("\tuminus:      ", l, r, "<-") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct UMinusDStmt: public BinaryStmt {
        UMinusDStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r) : BinaryStmt("\tuminus.d:    ", l, r, "<-") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };

    struct NotStmt: public BinaryStmt {
        NotStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r) : BinaryStmt("\tnot:         ", l, r, "<-") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };

    struct SLTDStmt: public BinaryStmt {
        SLTDStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r) : BinaryStmt("\tslt.d:       ", l, r, ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct SLEDStmt: public BinaryStmt {
        SLEDStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r) : BinaryStmt("\tsle.d:       ", l, r, ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct SEQDStmt: public BinaryStmt {
        SEQDStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r) : BinaryStmt("\tseq.d:       ", l, r, ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };


    struct TernaryStmt: public Stmt {
    private:
        std::string const cmd;
        std::string const print_op_1;
        std::string const print_op_2;
    public:
        std::shared_ptr<Val> lhs, rhs, rrhs;
        TernaryStmt(std::string const cmd, std::shared_ptr<Val> l, std::shared_ptr<Val> r, std::shared_ptr<Val> rr, std::string print_op_1, std::string print_op_2) : cmd(cmd), print_op_1(print_op_1), print_op_2(print_op_2), lhs(l), rhs(r), rrhs(rr) {}
        virtual void print(std::ostream& o) const override
        {
            o << cmd;
            lhs->print(o);
            o << " " << print_op_1 << " ";
            rhs->print(o);
            o << " " << print_op_2 << " ";
            rrhs->print(o);
            o << "\n";
        }
    };

    struct AddStmt: public TernaryStmt {
        AddStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r, std::shared_ptr<Register> rr) : TernaryStmt("\tadd:         ", l, r, rr, "<-", ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct SubStmt: public TernaryStmt {
        SubStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r, std::shared_ptr<Register> rr) : TernaryStmt("\tsub:         ", l, r, rr, "<-", ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct MulStmt: public TernaryStmt {
        MulStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r, std::shared_ptr<Register> rr) : TernaryStmt("\tmul:         ", l, r, rr, "<-", ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct DivStmt: public TernaryStmt {
        DivStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r, std::shared_ptr<Register> rr) : TernaryStmt("\tdiv:         ", l, r, rr, "<-", ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };

    struct AddDStmt: public TernaryStmt {
        AddDStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r, std::shared_ptr<Register> rr) : TernaryStmt("\tadd.d:       ", l, r, rr, "<-", ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct SubDStmt: public TernaryStmt {
        SubDStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r, std::shared_ptr<Register> rr) : TernaryStmt("\tsub.d:       ", l, r, rr, "<-", ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct MulDStmt: public TernaryStmt {
        MulDStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r, std::shared_ptr<Register> rr) : TernaryStmt("\tmul.d:       ", l, r, rr, "<-", ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct DivDStmt: public TernaryStmt {
        DivDStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r, std::shared_ptr<Register> rr) : TernaryStmt("\tdiv.d:       ", l, r, rr, "<-", ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };

    struct SLTStmt: public TernaryStmt {
        SLTStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r, std::shared_ptr<Register> rr) : TernaryStmt("\tslt:         ", l, r, rr, "<-", ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct SLEStmt: public TernaryStmt {
        SLEStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r, std::shared_ptr<Register> rr) : TernaryStmt("\tsle:         ", l, r, rr, "<-", ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct SGTStmt: public TernaryStmt {
        SGTStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r, std::shared_ptr<Register> rr) : TernaryStmt("\tsgt:         ", l, r, rr, "<-", ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct SGEStmt: public TernaryStmt {
        SGEStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r, std::shared_ptr<Register> rr) : TernaryStmt("\tsge:         ", l, r, rr, "<-", ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct SEQStmt: public TernaryStmt {
        SEQStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r, std::shared_ptr<Register> rr) : TernaryStmt("\tseq:         ", l, r, rr, "<-", ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct SNEStmt: public TernaryStmt {
        SNEStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r, std::shared_ptr<Register> rr) : TernaryStmt("\tsne:         ", l, r, rr, "<-", ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };

    struct OrStmt: public TernaryStmt {
        OrStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r, std::shared_ptr<Register> rr) : TernaryStmt("\tor:          ", l, r, rr, "<-", ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };

    struct AndStmt: public TernaryStmt {
        AndStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r, std::shared_ptr<Register> rr) : TernaryStmt("\tand:         ", l, r, rr, "<-", ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };

    struct MovTStmt: public TernaryStmt {
        MovTStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r, std::shared_ptr<IntLit> rr) : TernaryStmt("\tmovt:        ", l, r, rr, "<-", ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct MovFStmt: public TernaryStmt {
        MovFStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r, std::shared_ptr<IntLit> rr) : TernaryStmt("\tmovf:        ", l, r, rr, "<-", ",") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };

    struct GetAddrStmt: public BinaryStmt {
        GetAddrStmt(std::shared_ptr<Register> l, std::shared_ptr<Mem> r) : BinaryStmt("\tget_addr:    ", l, r, "<-") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct DerefStmt: public BinaryStmt {
        DerefStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r): BinaryStmt("\tderef:       ", l, r, "<-") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct DerefDStmt: public BinaryStmt {
        DerefDStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r): BinaryStmt("\tderef.d:     ", l, r, "<-") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct AddrAssignStmt: public BinaryStmt {
        AddrAssignStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r): BinaryStmt("\tdrfs:        ", l, r, "*<-") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };
    struct AddrAssignDStmt: public BinaryStmt {
        AddrAssignDStmt(std::shared_ptr<Register> l, std::shared_ptr<Register> r): BinaryStmt("\tdrfs.d:      ", l, r, "*<-") {}
        void gen_asm(std::vector<std::shared_ptr<ASM::Stmt>>& stmts) override;
    };

}

#endif // RTL_H
