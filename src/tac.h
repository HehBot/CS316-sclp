#ifndef TAC_H
#define TAC_H

#include <cassert>
#include <iostream>
#include <memory>
#include <rtl.h>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Symbol;
void print_string_escapes(std::string, std::ostream&);

namespace TAC {
    enum class Type {
        BOOL, INT, FLOAT, STRING, PTR
    };
    static inline size_t get_type_size(Type t)
    {
        size_t s[5] = { 4, 4, 8, 4, 4 };
        return s[(size_t)t];
    }

    struct Base {
        virtual void print(std::ostream&) const = 0;
    };

    struct Stmt : public Base {
        virtual void gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) = 0;
    };
    struct Expr : public Base {
        Type const type;
        std::shared_ptr<RTL::Register> reg = nullptr;
        Expr(Type t)
            : type(t)
        {
        }
        virtual std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) = 0;
    };

    struct Val : public Expr {
        Val(Type t)
            : Expr(t)
        {
        }
        virtual void gen_print_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) = 0;
        virtual void gen_push_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) = 0;
    };
    struct UnExpr : public Expr {
        std::shared_ptr<Val> const lhs;
        UnExpr(Type t, std::shared_ptr<Val> l, std::string const print_op) : Expr(t), lhs(l), print_op(print_op) {}
        void print(std::ostream& o) const override
        {
            o << print_op << ' ';
            lhs->print(o);
        }

    private:
        std::string const print_op;
    };
    struct BinExpr : public Expr {
        std::shared_ptr<Val> const lhs, rhs;
        BinExpr(Type t, std::shared_ptr<Val> l, std::shared_ptr<Val> r, std::string const print_op) : Expr(t), lhs(l), rhs(r), print_op(print_op) {}
        void print(std::ostream& o) const override
        {
            lhs->print(o);
            o << " " << print_op << " ";
            rhs->print(o);
        }

    private:
        std::string const print_op;
    };

    struct Sym : public Val {
        std::string name;

        bool in_mem;
        bool is_global;
        ssize_t fp_offset;

        Sym(std::string n, Type t, bool in_mem)
            : Val(t), name(n), in_mem(in_mem), is_global(false)
        {
        }
        void print(std::ostream& o) const override
        {
            o << name;
        }
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
        void gen_print_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
        void gen_push_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };

    struct IntLit : public Val {
        size_t val;
        IntLit(size_t v) : Val(Type::INT), val(v) {}
        void print(std::ostream& o) const override
        {
            o << val;
        }
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
        void gen_print_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
        void gen_push_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct FloatLit : public Val {
        double val;
        FloatLit(double v) : Val(Type::FLOAT), val(v) {}
        void print(std::ostream& o) const override
        {
            o << val;
        }
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
        void gen_print_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
        void gen_push_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct StrLit : public Val {
        std::string val;
        StrLit(std::string v) : Val(Type::STRING), val(v) {}
        void print(std::ostream& o) const override
        {
            o << '"';
            print_string_escapes(val, o);
            o << '"';
        }
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
        void gen_print_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
        void gen_push_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };

    struct AddExpr : public BinExpr {
        AddExpr(std::shared_ptr<Val> l, std::shared_ptr<Val> r) : BinExpr(l->type, l, r, "+") {}
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct SubExpr : public BinExpr {
        SubExpr(std::shared_ptr<Val> l, std::shared_ptr<Val> r) : BinExpr(l->type, l, r, "-") {}
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct NegExpr : public UnExpr {
        NegExpr(std::shared_ptr<Val> l) : UnExpr(l->type, l, "-") {}
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct MulExpr : public BinExpr {
        MulExpr(std::shared_ptr<Val> l, std::shared_ptr<Val> r) : BinExpr(l->type, l, r, "*") {}
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct DivExpr : public BinExpr {
        DivExpr(std::shared_ptr<Val> l, std::shared_ptr<Val> r) : BinExpr(l->type, l, r, "/") {}
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct EqualExpr : public BinExpr {
        EqualExpr(std::shared_ptr<Val> l, std::shared_ptr<Val> r) : BinExpr(Type::BOOL, l, r, "==") {}
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct NotEqualExpr : public BinExpr {
        NotEqualExpr(std::shared_ptr<Val> l, std::shared_ptr<Val> r) : BinExpr(Type::BOOL, l, r, "!=") {}
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct GreaterExpr : public BinExpr {
        GreaterExpr(std::shared_ptr<Val> l, std::shared_ptr<Val> r) : BinExpr(Type::BOOL, l, r, ">") {}
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct LessExpr : public BinExpr {
        LessExpr(std::shared_ptr<Val> l, std::shared_ptr<Val> r) : BinExpr(Type::BOOL, l, r, "<") {}
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct GreaterEqualExpr : public BinExpr {
        GreaterEqualExpr(std::shared_ptr<Val> l, std::shared_ptr<Val> r) : BinExpr(Type::BOOL, l, r, ">=") {}
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct LessEqualExpr : public BinExpr {
        LessEqualExpr(std::shared_ptr<Val> l, std::shared_ptr<Val> r) : BinExpr(Type::BOOL, l, r, "<=") {}
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct NotExpr : public UnExpr {
        NotExpr(std::shared_ptr<Val> l) : UnExpr(Type::BOOL, l, "!") {}
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct AndExpr : public BinExpr {
        AndExpr(std::shared_ptr<Val> l, std::shared_ptr<Val> r) : BinExpr(Type::BOOL, l, r, "&&") {}
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct OrExpr : public BinExpr {
        OrExpr(std::shared_ptr<Val> l, std::shared_ptr<Val> r) : BinExpr(Type::BOOL, l, r, "||") {}
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct CallExpr : public Expr {
        std::vector<std::shared_ptr<Val>> params;
        CallExpr(Type t, std::vector<std::shared_ptr<Val>> params) : Expr(t), params(params) {}
    };
    struct FuncCallExpr : public CallExpr {
        std::string func_name;
        FuncCallExpr(Type t, std::string func_name, std::vector<std::shared_ptr<Val>> const& params) : CallExpr(t, params), func_name(func_name) {}
        void print(std::ostream& o) const override
        {
            o << func_name << '(';
            if (params.size() > 0) {
                auto it = params.begin(), ite = params.end();
                (*it)->print(o);
                ++it;
                while (it != ite) {
                    o << ", ";
                    (*it)->print(o);
                    ++it;
                }
            }
            o << ')';
        }
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct FuncPtrCallExpr : public CallExpr {
        std::shared_ptr<Val> func_ptr;
        FuncPtrCallExpr(Type t, std::shared_ptr<Val> func_ptr, std::vector<std::shared_ptr<Val>> const& params) : CallExpr(t, params), func_ptr(func_ptr) {}
        void print(std::ostream& o) const override
        {
            o << "(*";
            func_ptr->print(o);
            o << ")(";
            if (params.size() > 0) {
                auto it = params.begin(), ite = params.end();
                (*it)->print(o);
                ++it;
                while (it != ite) {
                    o << ", ";
                    (*it)->print(o);
                    ++it;
                }
            }
            o << ')';
        }
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct DerefExpr : public Expr {
        std::shared_ptr<Val> const arg;
        DerefExpr(Type t, std::shared_ptr<Val> a) : Expr(t), arg(a) {}
        void print(std::ostream& o) const override
        {
            o << "*";
            arg->print(o);
        }
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct AddrExpr : public Expr {
        std::shared_ptr<Sym> const arg;
        AddrExpr(std::shared_ptr<Sym> a) : Expr(Type::PTR), arg(a) {}
        void print(std::ostream& o) const override
        {
            o << "&";
            arg->print(o);
        }
        std::shared_ptr<RTL::Register> gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };

    struct PrintStmt : public Stmt {
        std::shared_ptr<Val> const arg;
        PrintStmt(std::shared_ptr<Val> a) : arg(a) {}
        void print(std::ostream& o) const override
        {
            o << "\twrite  ";
            arg->print(o);
            o << '\n';
        }
        void gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct ReadIntStmt : public Stmt {
        std::shared_ptr<Val> const loc;
        ReadIntStmt(std::shared_ptr<Val> l) : loc(l) {}
        void print(std::ostream& o) const override
        {
            o << "\treadi  ";
            loc->print(o);
            o << '\n';
        }
        void gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct ReadFloatStmt : public Stmt {
        std::shared_ptr<Val> const loc;
        ReadFloatStmt(std::shared_ptr<Val> l) : loc(l) {}
        void print(std::ostream& o) const override
        {
            o << "\treadf  ";
            loc->print(o);
            o << '\n';
        }
        void gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct AssignStmt : public Stmt {
        std::shared_ptr<Sym> const lhs;
        std::shared_ptr<Expr> const rhs;
        AssignStmt(std::shared_ptr<Sym> l, std::shared_ptr<Expr> r) : lhs(l), rhs(r) {}
        void print(std::ostream& o) const override
        {
            o << '\t' << lhs->name << " = ";
            rhs->print(o);
            o << '\n';
        }
        void gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct AddrAssignStmt : public Stmt {
        std::shared_ptr<Val> const lhs;
        std::shared_ptr<Expr> const rhs;
        AddrAssignStmt(std::shared_ptr<Val> l, std::shared_ptr<Expr> r) : lhs(l), rhs(r) {}
        void print(std::ostream& o) const override
        {
            o << '\t' << "*";
            lhs->print(o);
            o << " = ";
            rhs->print(o);
            o << '\n';
        }
        void gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct Label : public Stmt {
        std::string name;
        Label(std::string n) : name(n) {}
        void print(std::ostream& o) const override
        {
            o << name << ":\n";
        }
        void gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct GotoStmt : public Stmt {
        std::shared_ptr<Label> const label;
        GotoStmt(std::shared_ptr<Label> l) : label(l) {}
        void print(std::ostream& o) const override
        {
            o << "\tgoto " << label->name << '\n';
        }
        void gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct IfGotoStmt : public Stmt {
        std::shared_ptr<Val> const cond;
        std::shared_ptr<Label> const label;
        IfGotoStmt(std::shared_ptr<Val> c, std::shared_ptr<Label> l) : cond(c), label(l) {}
        void print(std::ostream& o) const override
        {
            o << "\tif(";
            cond->print(o);
            o << ") goto " << label->name << '\n';
        }
        void gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct CallStmt : public Stmt {
        std::shared_ptr<CallExpr> const e;
        CallStmt(std::shared_ptr<CallExpr> e) : e(e) {}
        void print(std::ostream& o) const override
        {
            o << "\t";
            e->print(o);
            o << "\n";
        }
        void gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };
    struct ReturnStmt : public Stmt {
        std::shared_ptr<Sym> const ret;
        ReturnStmt(std::shared_ptr<Sym> ret) : ret(ret) {};
        void print(std::ostream& o) const override
        {
            o << "\t return ";
            ret->print(o);
            o << "\n";
        }
        void gen_rtl(std::vector<std::shared_ptr<RTL::Stmt>>& stmts) override;
    };

    class Context {
        static std::string const temp_prefix, stemp_prefix;
        static std::string const symbol_suffix;
        static std::string const label_prefix;

        // variables
        size_t next_temp;
        size_t next_stemp;
        std::unordered_set<std::string> names_used;
        std::unordered_map<std::shared_ptr<Symbol>, std::shared_ptr<TAC::Sym>> table;

        // labels
        static size_t next_label;   // shared across contexts, hence static

        // for assembly generation
        size_t stackframe_size;
        size_t paramframe_size;

    public:
        Context();
        size_t get_stackframe_size() const
        {
            return stackframe_size;
        }

        std::shared_ptr<Sym> get_temp(Type t);
        std::shared_ptr<Sym> get_stemp(Type t);
        std::shared_ptr<Sym> get_symbol(std::shared_ptr<Symbol>);
        std::shared_ptr<Sym> add_param_symbol(std::shared_ptr<Symbol>);
        static std::shared_ptr<Label> get_label();

        std::shared_ptr<Label> return_label;
        std::shared_ptr<Sym> return_sym;
        std::shared_ptr<Label> break_label, continue_label;
    };

}

#endif // TAC_H
