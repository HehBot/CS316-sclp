#ifndef AST_H
#define AST_H

#include <error.h>
#include <types.h>
#include <sym.h>
#include <iostream>
#include <memory>
#include <vector>
#include <parse.h>
#include <tac.h>
#include <asm.h>

namespace AST {
    class Base {
    public:
        virtual ~Base() = default;
        virtual void print(std::ostream&, std::string) const = 0;
    };

    class Stmt : public Base {
    public:
        virtual ~Stmt() = default;
        virtual void tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const = 0;
        virtual size_t break_count() const
        {
            return 0;
        }
        virtual size_t continue_count() const
        {
            return 0;
        }

        // sclp_error in case of improper return
        // For void decl_ret
        //      true if there is an explicit proper return along some path
        //      false otherwise
        // For non-void decl_ret
        //      true if there is a proper return along all paths
        //      false otherwise
        virtual bool check_return(size_t line, SemType const* decl_ret) const
        {
            return false;
        }
    };
    class Expr : public Base {
    public:
        SemType const* const semtype;
        Expr(SemType const* semtype) : semtype(semtype) {}
        virtual ~Expr() = default;
        virtual std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const = 0;
    };
    class LValExpr : public Expr {
    public:
        bool is_sym;

        LValExpr(SemType const* semtype) : Expr(semtype), is_sym(false) {}
        virtual std::shared_ptr<TAC::Val> addr_tac(std::vector<std::shared_ptr<TAC::Stmt>>& stmts, TAC::Context& ctx) const = 0;
        virtual bool is_const() const = 0;
    };
    class Sym : public LValExpr {
    public:
        std::shared_ptr<Symbol> const sym;
        Sym(std::shared_ptr<Symbol> sym) : LValExpr(sym->semtype), sym(sym)
        {
            is_sym = true;
        }
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
        std::shared_ptr<TAC::Val> addr_tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
        bool is_const() const
        {
            return sym->is_const;
        }
    };
    class IntLit : public Expr {
    public:
        size_t const val;
        IntLit(size_t v) : Expr(SemType::make_int()), val(v) {}
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class FloatLit : public Expr {
    public:
        double const val;
        FloatLit(double v) : Expr(SemType::make_float()), val(v) {}
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class StrLit : public Expr {
    public:
        std::string const val;
        StrLit(std::string v) : Expr(SemType::make_string()), val(v) {}
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };

    // statements
    class AssignStmt : public Stmt {
    public:
        std::shared_ptr<LValExpr> const lhs;
        std::shared_ptr<Expr> const rhs;
        AssignStmt(size_t line, std::shared_ptr<LValExpr> lhs, std::shared_ptr<Expr> rhs)
            : lhs(lhs), rhs(rhs)
        {
            if (!SemType::check_assign(lhs->semtype, rhs->semtype))
                sclp_error(line, "Assignment type mismatch");
        }
        void print(std::ostream&, std::string) const override;
        void tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class PrintStmt : public Stmt {
    public:
        std::shared_ptr<Expr> const arg;
        PrintStmt(size_t line, std::shared_ptr<Expr> arg)
            : arg(arg)
        {
            if (!SemType::check(SemType::StmtUn::Print, arg->semtype))
                sclp_error(line, "Print type mismatch");
        }
        void print(std::ostream&, std::string) const override;
        void tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class ReadStmt : public Stmt {
    public:
        std::shared_ptr<LValExpr> const arg;
        ReadStmt(size_t line, std::shared_ptr<LValExpr> arg)
            : arg(arg)
        {
            if (!SemType::check(SemType::StmtUn::Read, arg->semtype))
                sclp_error(line, "Read type mismatch");
        }
        void print(std::ostream&, std::string) const override;
        void tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class CompoundStmt : public Stmt {
    private:
        size_t bc;
        size_t cc;

    public:
        std::vector<std::shared_ptr<Stmt>> const stmt_list;
        CompoundStmt(std::vector<std::shared_ptr<Stmt>> const& stmt_list) : bc(0), cc(0), stmt_list(stmt_list)
        {
            for (auto p : stmt_list) {
                bc += p->break_count();
                cc += continue_count();
            }
        }
        ~CompoundStmt() = default;
        void print(std::ostream&, std::string) const override;
        void tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
        size_t break_count() const
        {
            return bc;
        }
        size_t continue_count() const
        {
            return cc;
        }
        bool check_return(size_t line, SemType const* decl_ret) const override
        {
            bool a = false;
            for (auto s : stmt_list)
                a = a || s->check_return(line, decl_ret);
            return a;
        }
    };
    class IfStmt : public Stmt {
    public:
        std::shared_ptr<Expr> const cond;
        std::shared_ptr<Stmt> const body;
        IfStmt(size_t line, std::shared_ptr<Expr> cond, std::shared_ptr<Stmt> body)
            : cond(cond), body(body)
        {
            if (!SemType::check_assign(SemType::make_bool(), cond->semtype))
                sclp_error(line, "If condition type mismatch");
        }
        virtual void print(std::ostream&, std::string) const override;
        virtual void tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
        virtual bool check_return(size_t line, SemType const* decl_ret) const override
        {
            bool b = body->check_return(line, decl_ret);
            if (decl_ret->is_void())
                return b;
            else
                return false;
        }
    };
    class IfElseStmt : public IfStmt {
    public:
        std::shared_ptr<Stmt> const else_body;
        IfElseStmt(size_t line, std::shared_ptr<Expr> cond, std::shared_ptr<Stmt> body, std::shared_ptr<Stmt> else_body) : IfStmt(line, cond, body), else_body(else_body) {}
        void print(std::ostream&, std::string) const override;
        void tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
        bool check_return(size_t line, SemType const* decl_ret) const override
        {
            bool true_part = body->check_return(line, decl_ret);
            bool false_part = else_body->check_return(line, decl_ret);
            if (decl_ret->is_void())
                return true_part || false_part;
            else
                return true_part && false_part;
        }
    };
    class WhileStmt : public Stmt {
    public:
        std::shared_ptr<Expr> const cond;
        std::shared_ptr<Stmt> const body;
        WhileStmt(size_t line, std::shared_ptr<Expr> cond, std::shared_ptr<Stmt> body)
            : cond(cond), body(body)
        {
            if (!SemType::check_assign(SemType::make_bool(), cond->semtype))
                sclp_error(line, "While condition type mismatch");
        }
        void print(std::ostream&, std::string) const override;
        void tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
        virtual bool check_return(size_t line, SemType const* decl_ret) const override
        {
            if (body != nullptr) {
                bool b = body->check_return(line, decl_ret);
                if (decl_ret->is_void())
                    return b;
            }
            return false;
        }
    };
    class DoWhileStmt : public Stmt {
    public:
        std::shared_ptr<Stmt> const body;
        std::shared_ptr<Expr> const cond;
        DoWhileStmt(size_t line, std::shared_ptr<Stmt> body, std::shared_ptr<Expr> cond)
            : body(body), cond(cond)
        {
            if (!SemType::check_assign(SemType::make_bool(), cond->semtype))
                sclp_error(line, "While condition type mismatch");
        }
        void print(std::ostream&, std::string) const override;
        void tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
        virtual bool check_return(size_t line, SemType const* decl_ret) const override
        {
            // do-while body executes atleast once
            return body->check_return(line, decl_ret);
        }
    };
    class ForStmt : public Stmt {
    public:
        std::shared_ptr<AssignStmt> const pre_stmt;
        std::shared_ptr<Expr> const cond;
        std::shared_ptr<AssignStmt> const inc_stmt;
        std::shared_ptr<Stmt> const body;
        ForStmt(size_t line, std::shared_ptr<AssignStmt> pre_stmt, std::shared_ptr<Expr> cond, std::shared_ptr<AssignStmt> inc_stmt, std::shared_ptr<Stmt> body)
            : pre_stmt(pre_stmt), cond(cond), inc_stmt(inc_stmt), body(body)
        {
            if (cond != nullptr && !SemType::check_assign(SemType::make_bool(), cond->semtype))
                sclp_error(line, "For condition type mismatch");
        }
        void print(std::ostream&, std::string) const override;
        void tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
        virtual bool check_return(size_t line, SemType const* decl_ret) const override
        {
            if (body != nullptr) {
                bool b = body->check_return(line, decl_ret);
                if (decl_ret->is_void())
                    return b;
            }
            return false;
        }
    };
    class BreakStmt : public Stmt {
    public:
        size_t const line;
        BreakStmt(size_t line) : line(line) {}
        void print(std::ostream&, std::string) const override;
        void tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
        size_t break_count() const override
        {
            return 1;
        }
    };
    class ContinueStmt : public Stmt {
    public:
        size_t const line;
        ContinueStmt(size_t line) : line(line) {}
        void print(std::ostream&, std::string) const override;
        void tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
        size_t continue_count() const override
        {
            return 1;
        }
    };
    class ReturnStmt : public Stmt {
    public:
        std::shared_ptr<Expr> const ret;
        ReturnStmt(std::shared_ptr<Expr> ret) : ret(ret) {}
        void print(std::ostream&, std::string) const override;
        void tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
        bool check_return(size_t line, SemType const* decl_ret) const override
        {
            if (ret == nullptr) {
                if (!decl_ret->is_void())
                    sclp_error(line, "Void return in non-void function");
                return true;
            } else {
                if (!SemType::check_assign(decl_ret, ret->semtype))
                    sclp_error(line, "Return type mismatch");
                return true;
            }
        }
    };
    class FuncDefn {
    public:
        std::shared_ptr<Symbol> func;
        std::vector<std::shared_ptr<Symbol>> params;
        std::shared_ptr<CompoundStmt> body;

        TAC::Context ctx;

        std::vector<std::shared_ptr<TAC::Stmt>> tac;

        std::vector<std::shared_ptr<RTL::Stmt>> rtl;

        std::vector<std::shared_ptr<ASM::Stmt>> mips_asm;

        size_t stackframe_size;

        FuncDefn(size_t line, std::shared_ptr<Symbol> func, std::vector<std::shared_ptr<Symbol>> params, std::shared_ptr<CompoundStmt> body)
            : func(func), params(params), body(body)
        {
            SemType const* ret_type = func->semtype->get_ret_type();
            bool check_ret = body->check_return(line, ret_type);
            if (!ret_type->is_void() && !check_ret)
                sclp_error(line, "Non-void function does not return along one or more paths");
            else if (ret_type->is_void() && check_ret)
                ctx.return_label = TAC::Context::get_label();
        }
        void make_tac()
        {
            SemType const* ret_type = func->semtype->get_ret_type();
            if (!ret_type->is_void()) {
                ctx.return_label = TAC::Context::get_label();
                ctx.return_sym = ctx.get_stemp(ret_type->to_tactype());
            }

            for (auto p : params)
                ctx.add_param_symbol(p);

            body->tac(tac, ctx);

            if (ctx.return_label != nullptr)
                tac.push_back(ctx.return_label);
            if (ctx.return_sym != nullptr)
                tac.push_back(std::make_shared<TAC::ReturnStmt>(ctx.return_sym));

            stackframe_size = ctx.get_stackframe_size();
        }
        void print(std::ostream&) const;
    };

    // expressions
    class TernaryExpr : public Expr {
    public:
        std::shared_ptr<Expr> const cond, true_part, false_part;
        TernaryExpr(size_t line, std::shared_ptr<Expr> cond, std::shared_ptr<Expr> true_part, std::shared_ptr<Expr> false_part)
            : Expr(SemType::result(cond->semtype, true_part->semtype, false_part->semtype)), cond(cond), true_part(true_part), false_part(false_part)
        {
            if (semtype == nullptr)
                sclp_error(line, "Ternary type mismatch");
        }
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class BinExpr : public Expr {
    public:
        std::shared_ptr<Expr> const lhs, rhs;
        BinExpr(SemType const* st, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs) : Expr(st), lhs(lhs), rhs(rhs) {}
        virtual ~BinExpr() = default;
    };
    class BinOtherArithExpr : public BinExpr {
    public:
        BinOtherArithExpr(size_t line, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs)
            : BinExpr(SemType::result(SemType::ExprBin::OtherArith, lhs->semtype, rhs->semtype), lhs, rhs)
        {
            if (semtype == nullptr)
                sclp_error(line, "Arithmetic type mismatch");
        }
        virtual ~BinOtherArithExpr() = default;
    };
    class AddExpr : public BinExpr {
    public:
        AddExpr(size_t line, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs) : BinExpr(SemType::result(SemType::ExprBin::AddSub, lhs->semtype, rhs->semtype), lhs, rhs)
        {
            if (semtype == nullptr)
                sclp_error(line, "Arithmetic type mismatch");
        }
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class SubExpr : public BinExpr {
    public:
        SubExpr(size_t line, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs) : BinExpr(SemType::result(SemType::ExprBin::AddSub, lhs->semtype, rhs->semtype), lhs, rhs)
        {
            if (semtype == nullptr)
                sclp_error(line, "Arithmetic type mismatch");
        }
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class MulExpr : public BinOtherArithExpr {
    public:
        MulExpr(size_t line, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs) : BinOtherArithExpr(line, lhs, rhs) {}
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class DivExpr : public BinOtherArithExpr {
    public:
        DivExpr(size_t line, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs) : BinOtherArithExpr(line, lhs, rhs) {}
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class BinCompExpr : public BinExpr {
    public:
        BinCompExpr(size_t line, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs)
            : BinExpr(SemType::result(SemType::ExprBin::Comp, lhs->semtype, rhs->semtype), lhs, rhs)
        {
            if (semtype == nullptr)
                sclp_error(line, "Comparison type mismatch");
        }
        virtual ~BinCompExpr() = default;
    };
    class EqualExpr : public BinCompExpr {
    public:
        EqualExpr(size_t line, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs) : BinCompExpr(line, lhs, rhs) {}
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class NotEqualExpr : public BinCompExpr {
    public:
        NotEqualExpr(size_t line, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs) : BinCompExpr(line, lhs, rhs) {}
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class LessExpr : public BinCompExpr {
    public:
        LessExpr(size_t line, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs) : BinCompExpr(line, lhs, rhs) {}
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class GreaterExpr : public BinCompExpr {
    public:
        GreaterExpr(size_t line, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs) : BinCompExpr(line, lhs, rhs) {}
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class LessEqualExpr : public BinCompExpr {
    public:
        LessEqualExpr(size_t line, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs) : BinCompExpr(line, lhs, rhs) {}
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class GreaterEqualExpr : public BinCompExpr {
    public:
        GreaterEqualExpr(size_t line, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs) : BinCompExpr(line, lhs, rhs) {}
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class BinLogicExpr : public BinExpr {
    public:
        BinLogicExpr(size_t line, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs)
            : BinExpr(SemType::result(SemType::ExprBin::Logic, lhs->semtype, rhs->semtype), lhs, rhs)
        {
            if (semtype == nullptr)
                sclp_error(line, "Logic type mismatch");
        }
        virtual ~BinLogicExpr() = default;
    };
    class AndExpr : public BinLogicExpr {
    public:
        AndExpr(size_t line, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs) : BinLogicExpr(line, lhs, rhs) {}
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class OrExpr : public BinLogicExpr {
    public:
        OrExpr(size_t line, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs) : BinLogicExpr(line, lhs, rhs) {}
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };

    class UnExpr : public Expr {
    public:
        std::shared_ptr<Expr> const lhs;
        UnExpr(SemType const* st, std::shared_ptr<Expr> lhs) : Expr(st), lhs(lhs) {}
    };
    class NegExpr : public UnExpr {
    public:
        NegExpr(size_t line, std::shared_ptr<Expr> lhs)
            : UnExpr(SemType::result(SemType::ExprUn::Neg, lhs->semtype), lhs)
        {
            if (semtype == nullptr)
                sclp_error(line, "Arithmetic type mismatch");
        }
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class NotExpr : public UnExpr {
    public:
        NotExpr(size_t line, std::shared_ptr<Expr> lhs)
            : UnExpr(SemType::result(SemType::ExprUn::Not, lhs->semtype), lhs)
        {
            if (semtype == nullptr)
                sclp_error(line, "Logic type mismatch");
        }
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };

    class ArrayExpr : public LValExpr {
        bool is_c;
        LValExpr* base_lval;

    public:
        std::shared_ptr<Expr> const base, index;
        ArrayExpr(size_t line, std::shared_ptr<Expr> base, std::shared_ptr<Expr> index) : LValExpr(SemType::result(SemType::ExprBin::Array, base->semtype, index->semtype)), is_c(true), base_lval(nullptr), base(base), index(index)
        {
            if (semtype == nullptr)
                sclp_error(line, "Array type mismatch");
            if (base->semtype->is_ptr())
                is_c = base->semtype->get_points_to_const();
            else {
                //  [in this case base must be an LValExpr (infact of semtype ARRAY)]
                assert(base->semtype->is_array());
                base_lval = dynamic_cast<LValExpr*>(base.get());
                assert(base_lval != nullptr);
                is_c = base_lval->is_const();
            }
        }
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
        std::shared_ptr<TAC::Val> addr_tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
        bool is_const() const override
        {
            return is_c;
        }
    };
    class DerefExpr : public LValExpr {
        bool is_c;

    public:
        std::shared_ptr<Expr> const lhs;
        DerefExpr(size_t line, std::shared_ptr<Expr> lhs)
            : LValExpr(SemType::result(SemType::ExprUn::Deref, lhs->semtype)), lhs(lhs)
        {
            if (semtype == nullptr)
                sclp_error(line, "Dereference type mismatch");
            is_c = lhs->semtype->get_points_to_const();
        }
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
        std::shared_ptr<TAC::Val> addr_tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
        bool is_const() const override
        {
            return is_c;
        }
    };
    class AddrExpr : public Expr {
    public:
        std::shared_ptr<LValExpr> lhs;
        AddrExpr(std::shared_ptr<LValExpr> lhs) : Expr(SemType::make_ptr(lhs->semtype, lhs->is_const())), lhs(lhs) {}
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };

    // functions
    class CallExpr : public Expr {
    private:
        static std::vector<SemType const*> get_param_types(std::vector<std::shared_ptr<Expr>> const& params)
        {
            std::vector<SemType const*> param_types;
            for (auto const& p : params)
                param_types.push_back(p->semtype);
            return param_types;
        }

    public:
        std::vector<std::shared_ptr<Expr>> params;
        CallExpr(size_t line, SemType const* func_semtype, std::vector<std::shared_ptr<Expr>> const& params)
            : Expr(SemType::result(func_semtype, get_param_types(params))), params(params)
        {
            if (semtype == nullptr)
                sclp_error(line, "Function type mismatch");
        }
        virtual ~CallExpr() = default;
    };
    class FuncCallExpr : public CallExpr {
    public:
        std::shared_ptr<Symbol> const func;
        FuncCallExpr(size_t line, std::shared_ptr<Symbol> func, std::vector<std::shared_ptr<Expr>> const& params)
            : CallExpr(line, func->semtype, params), func(func)
        {
        }
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class FuncPtrCallExpr : public CallExpr {
    public:
        std::shared_ptr<Expr> const func_ptr;
        FuncPtrCallExpr(size_t line, std::shared_ptr<AST::DerefExpr> func, std::vector<std::shared_ptr<AST::Expr>> const& params)
            : CallExpr(line, func->semtype, params), func_ptr(func->lhs)
        {
        }
        void print(std::ostream&, std::string) const override;
        std::shared_ptr<TAC::Val> tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };
    class CallStmt : public Stmt {
    public:
        std::shared_ptr<CallExpr> const fc;

        CallStmt(size_t line, std::shared_ptr<CallExpr> fc) : fc(fc)
        {
            if (!fc->semtype->is_void())
                sclp_error(line, "Function return value ignored");
        }
        void print(std::ostream&, std::string) const override;
        void tac(std::vector<std::shared_ptr<TAC::Stmt>>&, TAC::Context&) const override;
    };

    std::vector<FuncDefn> from_parse_tree(ParseNode* parse_tree, SymbolTable&);
}

#endif // AST_H
