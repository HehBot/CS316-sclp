#include <ast.h>
#include <tac.h>

using TACVal =  std::shared_ptr<TAC::Val>;
using TACSym =  std::shared_ptr<TAC::Sym>;
using TACExpr = std::shared_ptr<TAC::Expr>;

using TACLabel = std::shared_ptr<TAC::Label>;
using TACStmt = std::shared_ptr<TAC::Stmt>;
using TACStmtList = std::vector<std::shared_ptr<TAC::Stmt>>;

using TACType = TAC::Type;

TACVal AST::Sym::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    return ctx.get_symbol(sym);
}
TACVal AST::Sym::addr_tac(TACStmtList& stmts, TAC::Context& ctx) const
{
//     if (sym->semtype->is_func()) {
//         return ctx.get_symbol(sym);
//     } else {
        TACSym t = ctx.get_temp(TACType::PTR);
        TACSym s = ctx.get_symbol(sym);
        stmts.push_back(std::make_shared<TAC::AssignStmt>(t, std::make_shared<TAC::AddrExpr>(s)));
        return t;
//     }
}
TACVal AST::IntLit::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    return std::make_shared<TAC::IntLit>(val);
}
TACVal AST::FloatLit::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    return std::make_shared<TAC::FloatLit>(val);
}
TACVal AST::StrLit::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    return std::make_shared<TAC::StrLit>(val);
}

TACVal AST::TernaryExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal c = cond->tac(stmts, ctx);

    TACLabel false_label = TAC::Context::get_label();
    TACLabel exit_label = TAC::Context::get_label();
    TACSym result = ctx.get_stemp(semtype->to_tactype());

    TACStmtList true_part_tac;
    TACVal t = true_part->tac(true_part_tac, ctx);
    TACStmtList false_part_tac;
    TACVal f = false_part->tac(false_part_tac, ctx);

    TACSym not_c = ctx.get_temp(TACType::BOOL);
    std::shared_ptr<TAC::NotExpr> not_c_expr = std::make_shared<TAC::NotExpr>(c);

    stmts.push_back(std::make_shared<TAC::AssignStmt>(not_c, not_c_expr));
    stmts.push_back(std::make_shared<TAC::IfGotoStmt>(not_c, false_label));
    stmts.insert(stmts.end(), true_part_tac.begin(), true_part_tac.end());
    stmts.push_back(std::make_shared<TAC::AssignStmt>(result, t));
    stmts.push_back(std::make_shared<TAC::GotoStmt>(exit_label));
    stmts.push_back(false_label);
    stmts.insert(stmts.end(), false_part_tac.begin(), false_part_tac.end());
    stmts.push_back(std::make_shared<TAC::AssignStmt>(result, f));
    stmts.push_back(exit_label);

    return result;
}
TACVal AST::AddExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal l = lhs->tac(stmts, ctx);
    TACVal r = rhs->tac(stmts, ctx);
    if (lhs->semtype->is_ptr()) {
        TACSym o = ctx.get_temp(TACType::INT);
        stmts.push_back(std::make_shared<TAC::AssignStmt>(o, std::make_shared<TAC::MulExpr>(r, std::make_shared<TAC::IntLit>(lhs->semtype->get_points_to_type()->size()))));
        TACSym s = ctx.get_temp(TACType::PTR);
        stmts.push_back(std::make_shared<TAC::AssignStmt>(s, std::make_shared<TAC::AddExpr>(l, o)));
        return s;
    } else {
        TACExpr expr = std::make_shared<TAC::AddExpr>(l, r);
        TACSym result = ctx.get_temp(semtype->to_tactype());
        stmts.push_back(std::make_shared<TAC::AssignStmt>(result, expr));
        return result;
    }
}
TACVal AST::SubExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal l = lhs->tac(stmts, ctx);
    TACVal r = rhs->tac(stmts, ctx);
    TACExpr expr = std::make_shared<TAC::SubExpr>(l, r);
    TACSym result = ctx.get_temp(semtype->to_tactype());
    stmts.push_back(std::make_shared<TAC::AssignStmt>(result, expr));
    return result;
}
TACVal AST::NegExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal l = lhs->tac(stmts, ctx);
    TACExpr expr = std::make_shared<TAC::NegExpr>(l);
    TACSym result = ctx.get_temp(semtype->to_tactype());
    stmts.push_back(std::make_shared<TAC::AssignStmt>(result, expr));
    return result;
}
TACVal AST::MulExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal l = lhs->tac(stmts, ctx);
    TACVal r = rhs->tac(stmts, ctx);
    TACExpr expr = std::make_shared<TAC::MulExpr>(l, r);
    TACSym result = ctx.get_temp(semtype->to_tactype());
    stmts.push_back(std::make_shared<TAC::AssignStmt>(result, expr));
    return result;
}
TACVal AST::DivExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal l = lhs->tac(stmts, ctx);
    TACVal r = rhs->tac(stmts, ctx);
    TACExpr expr = std::make_shared<TAC::DivExpr>(l, r);
    TACSym result = ctx.get_temp(semtype->to_tactype());
    stmts.push_back(std::make_shared<TAC::AssignStmt>(result, expr));
    return result;
}
TACVal AST::EqualExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal l = lhs->tac(stmts, ctx);
    TACVal r = rhs->tac(stmts, ctx);
    TACExpr expr = std::make_shared<TAC::EqualExpr>(l, r);
    TACSym result = ctx.get_temp(TACType::BOOL);
    stmts.push_back(std::make_shared<TAC::AssignStmt>(result, expr));
    return result;
}
TACVal AST::NotEqualExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal l = lhs->tac(stmts, ctx);
    TACVal r = rhs->tac(stmts, ctx);
    TACExpr expr = std::make_shared<TAC::NotEqualExpr>(l, r);
    TACSym result = ctx.get_temp(TACType::BOOL);
    stmts.push_back(std::make_shared<TAC::AssignStmt>(result, expr));
    return result;
}
TACVal AST::GreaterExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal l = lhs->tac(stmts, ctx);
    TACVal r = rhs->tac(stmts, ctx);
    TACExpr expr = std::make_shared<TAC::GreaterExpr>(l, r);
    TACSym result = ctx.get_temp(TACType::BOOL);
    stmts.push_back(std::make_shared<TAC::AssignStmt>(result, expr));
    return result;
}
TACVal AST::LessExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal l = lhs->tac(stmts, ctx);
    TACVal r = rhs->tac(stmts, ctx);
    TACExpr expr = std::make_shared<TAC::LessExpr>(l, r);
    TACSym result = ctx.get_temp(TACType::BOOL);
    stmts.push_back(std::make_shared<TAC::AssignStmt>(result, expr));
    return result;
}
TACVal AST::GreaterEqualExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal l = lhs->tac(stmts, ctx);
    TACVal r = rhs->tac(stmts, ctx);
    TACExpr expr = std::make_shared<TAC::GreaterEqualExpr>(l, r);
    TACSym result = ctx.get_temp(TACType::BOOL);
    stmts.push_back(std::make_shared<TAC::AssignStmt>(result, expr));
    return result;
}
TACVal AST::LessEqualExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal l = lhs->tac(stmts, ctx);
    TACVal r = rhs->tac(stmts, ctx);
    TACExpr expr = std::make_shared<TAC::LessEqualExpr>(l, r);
    TACSym result = ctx.get_temp(TACType::BOOL);
    stmts.push_back(std::make_shared<TAC::AssignStmt>(result, expr));
    return result;
}
TACVal AST::AndExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal l = lhs->tac(stmts, ctx);
    TACVal r = rhs->tac(stmts, ctx);
    TACExpr expr = std::make_shared<TAC::AndExpr>(l, r);
    TACSym result = ctx.get_temp(TACType::BOOL);
    stmts.push_back(std::make_shared<TAC::AssignStmt>(result, expr));
    return result;
}
TACVal AST::OrExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal l = lhs->tac(stmts, ctx);
    TACVal r = rhs->tac(stmts, ctx);
    TACExpr expr = std::make_shared<TAC::OrExpr>(l, r);
    TACSym result = ctx.get_temp(TACType::BOOL);
    stmts.push_back(std::make_shared<TAC::AssignStmt>(result, expr));
    return result;
}
TACVal AST::NotExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal l = lhs->tac(stmts, ctx);
    TACExpr expr = std::make_shared<TAC::NotExpr>(l);
    TACSym result = ctx.get_temp(TACType::BOOL);
    stmts.push_back(std::make_shared<TAC::AssignStmt>(result, expr));
    return result;
}
TACVal AST::FuncCallExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACType ret_tac_type;
    TACSym r;
    if (!semtype->is_void()) {
        ret_tac_type = semtype->to_tactype();
        r = ctx.get_temp(ret_tac_type);
    }

    std::vector<TACVal> tac_params;
    for (auto p : params)
        tac_params.push_back(p->tac(stmts, ctx));

    if (!semtype->is_void()) {
        TACExpr call = std::make_shared<TAC::FuncCallExpr>(ret_tac_type, func->name, tac_params);
        stmts.push_back(std::make_shared<TAC::AssignStmt>(r, call));
        return r;
    } else {
        std::shared_ptr<TAC::CallExpr> call = std::make_shared<TAC::FuncCallExpr>((TAC::Type)-1, func->name, tac_params);
        stmts.push_back(std::make_shared<TAC::CallStmt>(call));
        return nullptr;
    }
}
TACVal AST::FuncPtrCallExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACType ret_tac_type;
    TACSym r;
    if (!semtype->is_void()) {
        ret_tac_type = semtype->to_tactype();
        r = ctx.get_temp(ret_tac_type);
    }

    std::vector<TACVal> tac_params;
    for (auto p : params)
        tac_params.push_back(p->tac(stmts, ctx));
    
    TACVal fp = func_ptr->tac(stmts, ctx);

    if (!semtype->is_void()) {
        TACExpr call = std::make_shared<TAC::FuncPtrCallExpr>(ret_tac_type, fp, tac_params);
        stmts.push_back(std::make_shared<TAC::AssignStmt>(r, call));
        return r;
    } else {
        std::shared_ptr<TAC::CallExpr> call = std::make_shared<TAC::FuncPtrCallExpr>((TAC::Type)-1, fp, tac_params);
        stmts.push_back(std::make_shared<TAC::CallStmt>(call));
        return nullptr;
    }
}
TACVal AST::AddrExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    return lhs->addr_tac(stmts, ctx);
}
TACVal AST::DerefExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal a = lhs->tac(stmts, ctx);
    TACType t = semtype->to_tactype();
    TACSym r = ctx.get_temp(t);
    stmts.push_back(std::make_shared<TAC::AssignStmt>(r, std::make_shared<TAC::DerefExpr>(t, a)));
    return r;
}
TACVal AST::DerefExpr::addr_tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    // as &*x is always equal to x
    return lhs->tac(stmts, ctx);
}
TACVal AST::ArrayExpr::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal ptr = this->addr_tac(stmts, ctx);
    TACType t = base->semtype->get_element_type()->to_tactype();
    TACSym r = ctx.get_temp(t);
    stmts.push_back(std::make_shared<TAC::AssignStmt>(r, std::make_shared<TAC::DerefExpr>(t, ptr)));
    return r;
}
TACVal AST::ArrayExpr::addr_tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal b;
    // as &x[y] is always equal to
    if (base->semtype->is_ptr())
        //  (x + sizeof(*x)*y) if x is a pointer
        b = base->tac(stmts, ctx);
    else {
        //  (&x + sizeof(*x)*y) otherwise
        //  [in this case x must be an LValExpr (infact of semtype ARRAY)]
        assert(base_lval != nullptr);
        b = base_lval->addr_tac(stmts, ctx);
    }
    TACVal i = index->tac(stmts, ctx);
    TACSym o = ctx.get_temp(TACType::INT);
    stmts.push_back(std::make_shared<TAC::AssignStmt>(o, std::make_shared<TAC::MulExpr>(std::make_shared<TAC::IntLit>(semtype->size()), i)));
    TACSym p = ctx.get_temp(TACType::PTR);
    stmts.push_back(std::make_shared<TAC::AssignStmt>(p, std::make_shared<TAC::AddExpr>(b, o)));
    return p;
}

void AST::PrintStmt::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal a = arg->tac(stmts, ctx);
    stmts.push_back(std::make_shared<TAC::PrintStmt>(a));
}
void AST::ReadStmt::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal l = arg->addr_tac(stmts, ctx);
    if (arg->semtype->is_float())
        stmts.push_back(std::make_shared<TAC::ReadFloatStmt>(l));
    else
        stmts.push_back(std::make_shared<TAC::ReadIntStmt>(l));
}
void AST::CompoundStmt::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    for (auto const& s : stmt_list)
        s->tac(stmts, ctx);
}
void AST::AssignStmt::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal r = rhs->tac(stmts, ctx);
    if (lhs->is_sym) {
        TACSym l = ctx.get_symbol(((AST::Sym*)lhs.get())->sym);
        stmts.push_back(std::make_shared<TAC::AssignStmt>(l, r));
    } else {
        TACVal l = lhs->addr_tac(stmts, ctx);
        stmts.push_back(std::make_shared<TAC::AddrAssignStmt>(l, r));
    }
}
void AST::IfStmt::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal c = cond->tac(stmts, ctx);

    TACStmtList body_tac;
    body->tac(body_tac, ctx);

    TACSym not_c = ctx.get_temp(TACType::BOOL);
    std::shared_ptr<TAC::NotExpr> not_c_expr = std::make_shared<TAC::NotExpr>(c);

    TACLabel false_label = TAC::Context::get_label();

    stmts.push_back(std::make_shared<TAC::AssignStmt>(not_c, not_c_expr));
    stmts.push_back(std::make_shared<TAC::IfGotoStmt>(not_c, false_label));
    stmts.insert(stmts.end(), body_tac.begin(), body_tac.end());
    stmts.push_back(std::make_shared<TAC::GotoStmt>(false_label));
    stmts.push_back(false_label);
}
void AST::IfElseStmt::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACVal c = cond->tac(stmts, ctx);

    TACStmtList body_tac;
    body->tac(body_tac, ctx);

    TACSym not_c = ctx.get_temp(TACType::BOOL);
    std::shared_ptr<TAC::NotExpr> not_c_expr = std::make_shared<TAC::NotExpr>(c);

    TACLabel exit_label = TAC::Context::get_label();
    TACLabel false_label = TAC::Context::get_label();

    stmts.push_back(std::make_shared<TAC::AssignStmt>(not_c, not_c_expr));
    stmts.push_back(std::make_shared<TAC::IfGotoStmt>(not_c, false_label));
    stmts.insert(stmts.end(), body_tac.begin(), body_tac.end());
    stmts.push_back(std::make_shared<TAC::GotoStmt>(exit_label));
    stmts.push_back(false_label);
    else_body->tac(stmts, ctx);
    stmts.push_back(exit_label);
}
void AST::WhileStmt::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACLabel loopback_label;
    TACLabel exit_label;

    TACStmtList cond_tac;
    TACVal c = cond->tac(cond_tac, ctx);

    TACStmtList body_tac;
    if (body != nullptr) {
        TACLabel old_continue = ctx.continue_label, old_break = ctx.break_label;
        if (body->break_count() > 0 || body->continue_count() > 0) {
            loopback_label = TAC::Context::get_label();
            exit_label = TAC::Context::get_label();
            ctx.continue_label = loopback_label, ctx.break_label = exit_label;
            body->tac(body_tac, ctx);
        } else {
            ctx.continue_label = loopback_label, ctx.break_label = exit_label;
            body->tac(body_tac, ctx);
            loopback_label = TAC::Context::get_label();
            exit_label = TAC::Context::get_label();
        }
        ctx.continue_label = old_continue, ctx.break_label = old_break;
    } else {
        loopback_label = TAC::Context::get_label();
        exit_label = TAC::Context::get_label();
    }

    TACSym not_c = ctx.get_temp(TACType::BOOL);
    std::shared_ptr<TAC::NotExpr> not_c_expr = std::make_shared<TAC::NotExpr>(c);

    stmts.push_back(loopback_label);

    stmts.insert(stmts.end(), cond_tac.begin(), cond_tac.end());
    stmts.push_back(std::make_shared<TAC::AssignStmt>(not_c, not_c_expr));
    stmts.push_back(std::make_shared<TAC::IfGotoStmt>(not_c, exit_label));

    stmts.insert(stmts.end(), body_tac.begin(), body_tac.end());

    stmts.push_back(std::make_shared<TAC::GotoStmt>(loopback_label));
    stmts.push_back(exit_label);
}
void AST::DoWhileStmt::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    TACLabel loopback_label;
    TACLabel exit_label;

    TACLabel old_continue = ctx.continue_label, old_break = ctx.break_label;

    TACStmtList body_tac;
    if (body->break_count() > 0 || body->continue_count() > 0) {
        loopback_label = TAC::Context::get_label();
        exit_label = TAC::Context::get_label();
        ctx.continue_label = loopback_label, ctx.break_label = exit_label;
        body->tac(body_tac, ctx);
    } else {
        ctx.continue_label = loopback_label, ctx.break_label = exit_label;
        body->tac(body_tac, ctx);
        loopback_label = TAC::Context::get_label();
    }

    ctx.continue_label = old_continue, ctx.break_label = old_break;

    stmts.push_back(loopback_label);
    stmts.insert(stmts.end(), body_tac.begin(), body_tac.end());
    TACVal c = cond->tac(stmts, ctx);
    stmts.push_back(std::make_shared<TAC::IfGotoStmt>(c, loopback_label));

    if (body->break_count() > 0)
        stmts.push_back(exit_label);
}
void AST::ForStmt::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    if (pre_stmt != nullptr)
        pre_stmt->tac(stmts, ctx);

    TACLabel loopback_label = TAC::Context::get_label();
    TACLabel exit_label;

    stmts.push_back(loopback_label);
    if (cond != nullptr) {
        exit_label = TAC::Context::get_label();

        TACVal c = cond->tac(stmts, ctx);

        TACSym not_c = ctx.get_temp(TACType::BOOL);
        std::shared_ptr<TAC::NotExpr> not_c_expr = std::make_shared<TAC::NotExpr>(c);
        stmts.push_back(std::make_shared<TAC::AssignStmt>(not_c, not_c_expr));
        stmts.push_back(std::make_shared<TAC::IfGotoStmt>(not_c, exit_label));
    }

    if (body != nullptr) {
        TACLabel continue_label;
        if (body->continue_count() > 0)
            continue_label = TAC::Context::get_label();
        if (body->break_count() > 0 && exit_label == nullptr)
            exit_label = TAC::Context::get_label();

        TACLabel old_continue = ctx.continue_label, old_break = ctx.break_label;
        ctx.continue_label = continue_label, ctx.break_label = exit_label;
        body->tac(stmts, ctx);
        ctx.continue_label = old_continue, ctx.break_label = old_break;
        if (continue_label != nullptr)
            stmts.push_back(continue_label);
    }

    if (inc_stmt != nullptr)
        inc_stmt->tac(stmts, ctx);
    stmts.push_back(std::make_shared<TAC::GotoStmt>(loopback_label));
    if (exit_label != nullptr)
        stmts.push_back(exit_label);
}
void AST::BreakStmt::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    if (ctx.break_label == nullptr)
        sclp_error(line, "Break statement outside loop");
    stmts.push_back(std::make_shared<TAC::GotoStmt>(ctx.break_label));
}
void AST::ContinueStmt::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    if (ctx.continue_label == nullptr)
        sclp_error(line, "Continue statement outside loop");
    stmts.push_back(std::make_shared<TAC::GotoStmt>(ctx.continue_label));
}
void AST::CallStmt::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    fc->tac(stmts, ctx);
}
void AST::ReturnStmt::tac(TACStmtList& stmts, TAC::Context& ctx) const
{
    if (ret != nullptr) {
        assert(ctx.return_sym != nullptr);
        TACVal r = ret->tac(stmts, ctx);
        stmts.push_back(std::make_shared<TAC::AssignStmt>(ctx.return_sym, r));
        stmts.push_back(std::make_shared<TAC::GotoStmt>(ctx.return_label));
    } else {
        assert(ctx.return_sym == nullptr);
        stmts.push_back(std::make_shared<TAC::GotoStmt>(ctx.return_label));
    }
}
