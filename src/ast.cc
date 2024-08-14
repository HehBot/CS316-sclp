#include <ast.h>
#include <cassert>
#include <error.h>
#include <memory>
#include <types.h>
#include <parse.h>

static TAC::Context tacctx;

// for the FuncDef currently being processed
static std::shared_ptr<Symbol> func_sym;
static std::vector<std::shared_ptr<Symbol>> func_params;

static SemType const* handle_prim_type(ParseNode* n)
{
    assert(n->type == PrimType);
    n = n->children[0];
    switch (n->type) {
    case Void:
        return SemType::make_void();
    case Bool:
        return SemType::make_bool();
    case Int:
        return SemType::make_int();
    case Float:
        return SemType::make_float();
    case String:
        return SemType::make_string();
    default:
        assert(false);
    }
}

static std::string handle_Name(ParseNode* name)
{
    assert(name->type == Name);
    return name->strval;
}
static std::string handle_OptName(ParseNode* opt_name)
{
    assert(opt_name->type == OptName);
    if (opt_name->children.size() == 1)
        return handle_Name(opt_name->children[0]);
    else
        return "";
}
static Symbol handle_ConstOptName(SemType const* built_type, ParseNode* const_opt_name)
{
    assert(const_opt_name->type == ConstOptName);
    ParseNode* d = const_opt_name->children[0];
    bool is_const;
    switch (d->type) {
        case NotConst:
            is_const = false;
            break;
        case Const:
            is_const = true;
            break;
        default:
            assert(false);
    }
    d = d->children[0];
    return Symbol{handle_OptName(d), built_type, is_const};
}

static SemType const* handle_ArrayNEList(SemType const* built_type, ParseNode* array_ne_list)
{
    assert(array_ne_list->type == ArrayNEList);
    for (ParseNode* s: array_ne_list->children) {
        assert(s->type == IntLit);
        built_type = SemType::make_array(built_type, s->intval);
        if (built_type == nullptr)
            sclp_error(array_ne_list->line, "Bad declaration");
    }
    return built_type;
}
static SemType const* handle_ConstAsteriskList(SemType const* built_type, ParseNode* const_asterisk_list)
{
    assert(const_asterisk_list->type == ConstAsteriskList);
    auto it = const_asterisk_list->children.begin();
    auto const it_end = const_asterisk_list->children.end();
    while (it != it_end) {
        assert((*it)->type == Asterisk);
        ++it;
        if (it == it_end) {
            built_type = SemType::make_ptr(built_type, false);
            break;
        } else if ((*it)->type == Const) {
            built_type = SemType::make_ptr(built_type, true);
            ++it;
        } else
            built_type = SemType::make_ptr(built_type, false);
    }
    return built_type;
}
static Symbol handle_TypeMod(SemType const* built_type, ParseNode* type_mod, SymbolTable& symtab);
static Symbol handle_ArrayMod(SemType const* built_type, ParseNode* array_mod, SymbolTable& symtab)
{
    assert(array_mod->type == ArrayMod);
    ParseNode* d = array_mod->children[0];
    if (d->type == ConstOptName) {
        built_type = handle_ArrayNEList(built_type, array_mod->children[1]);
        return handle_ConstOptName(built_type, d);
    } else if (d->type == TypeMod) {
        built_type = handle_ArrayNEList(built_type, array_mod->children[1]);
        return handle_TypeMod(built_type, d, symtab);
    }
    built_type = handle_ConstAsteriskList(built_type, d);
    built_type = handle_ArrayNEList(built_type, array_mod->children[2]);

    d = array_mod->children[1];
    switch (d->type) {
        case TypeMod:
            return handle_TypeMod(built_type, d, symtab);
        case ConstOptName:
            return handle_ConstOptName(built_type, d);
        default:
            assert(false);
    }
}
static Symbol handle_SingleDecl(ParseNode* single_decl, SymbolTable& symtab);
static SemType const* handle_ParamList(SemType const* ret_type, ParseNode* param_list, SymbolTable& symtab)
{
    assert(param_list->type == ParamList);
    std::vector<SemType const*> param_types;
    func_params.clear();
    symtab.begin_scope();
    for (ParseNode* single_decl : param_list->children) {
        Symbol s = handle_SingleDecl(single_decl, symtab);
        param_types.push_back(s.semtype);
        if (s.name.length() > 0) {
            std::shared_ptr<Symbol> sps;
            if ((sps = symtab.put_symbol(s)) == nullptr)
                sclp_error(single_decl->line, "Symbol " + s.name + " redeclared");
            func_params.push_back(sps);
        }
    }
    symtab.end_scope();
    ret_type = SemType::make_func(ret_type, param_types);
    if (ret_type == nullptr)
        sclp_error(param_list->line, "Bad declaration");
    return ret_type;
}
static Symbol handle_FuncMod(SemType const* built_type, ParseNode* func_mod, SymbolTable& symtab)
{
    assert(func_mod->type == FuncMod);
    ParseNode* d = func_mod->children[0];
    if (d->type == Name) {
        built_type = handle_ParamList(built_type, func_mod->children[1], symtab);
        Symbol s{d->strval, built_type, false};
        return s;
    } else if (d->type == TypeMod) {
        built_type = handle_ParamList(built_type, func_mod->children[1], symtab);
        return handle_TypeMod(built_type, d, symtab);
    }
    built_type = handle_ConstAsteriskList(built_type, d);
    built_type = handle_ParamList(built_type, func_mod->children[2], symtab);

    d = func_mod->children[1];
    switch (d->type) {
        case TypeMod:
            return handle_TypeMod(built_type, d, symtab);
        case Name:
            return Symbol{d->strval, built_type, false};
        default:
            assert(false);
    }
}

static Symbol handle_TypeMod(SemType const* built_type, ParseNode* type_mod, SymbolTable& symtab)
{
    assert(type_mod->type == TypeMod);
    ParseNode* d = type_mod->children[0];
    switch (d->type) {
    case ArrayMod:
        return handle_ArrayMod(built_type, d, symtab);
    case FuncMod:
        return handle_FuncMod(built_type, d, symtab);
    case ConstOptName:
        return handle_ConstOptName(built_type, d);
    case ConstAsteriskList:
        built_type = handle_ConstAsteriskList(built_type, d);
        break;
    default:
        assert(false);
    }
    d = type_mod->children[1];
    switch (d->type) {
        case TypeMod:
            return handle_TypeMod(built_type, d, symtab);
        case ConstOptName:
            return handle_ConstOptName(built_type, d);
        default:
            assert(false);
    }
}

static void handle_MultiDecl(ParseNode* multi_decl, SymbolTable& symtab)
{
    assert(multi_decl->type == MultiDecl);
    SemType const* prim_type = handle_prim_type(multi_decl->children[0]);

    ParseNode* type_mod_ne_list = multi_decl->children[1];
    assert(type_mod_ne_list->type == TypeModNEList);
    for (ParseNode* type_mod : type_mod_ne_list->children) {
        Symbol s = handle_TypeMod(prim_type, type_mod, symtab);
        if (s.name.length() == 0)
            sclp_error(type_mod->line, "Useless declaration");
        if (s.semtype->is_void())
            sclp_error(type_mod->line, "Symbol " + s.name + " declared as void type");
        std::shared_ptr<Symbol> sps;
        if ((sps = symtab.put_symbol(s)) == nullptr)
            sclp_error(type_mod->line, "Symbol " + s.name + " redeclared");
        if (!sps->is_global)
            tacctx.get_symbol(sps);
    }
}
static Symbol handle_SingleDecl(ParseNode* single_decl, SymbolTable& symtab)
{
    assert(single_decl->type == SingleDecl);
    SemType const* prim_type = handle_prim_type(single_decl->children[0]);

    ParseNode* type_mod = single_decl->children[1];
    Symbol s = handle_TypeMod(prim_type, type_mod, symtab);
    if (s.semtype->is_void())
        sclp_error(type_mod->line, "Symbol " + s.name + (s.name.length() > 0 ? " " : "") + "declared as void type");
    return s;
}
static void handle_DeclStmt(ParseNode* decl_stmt, SymbolTable& symtab)
{
    assert(decl_stmt->type == DeclStmt);
    ParseNode* multi_decl = decl_stmt->children[0];
    handle_MultiDecl(multi_decl, symtab);
}

static std::shared_ptr<AST::Expr> handle_Expr(ParseNode* d, SymbolTable& symtab);
static std::shared_ptr<AST::DerefExpr> handle_DerefExpr(ParseNode* deref_expr, SymbolTable& symtab)
{
    assert(deref_expr->type == Expr_DEREF);
    std::shared_ptr<AST::Expr> left = handle_Expr(deref_expr->children[0], symtab);
    return std::make_shared<AST::DerefExpr>(deref_expr->line, left);
}
static std::shared_ptr<AST::LValExpr> handle_Expr_LValExpr(ParseNode* d, SymbolTable& symtab)
{
    assert(d->type == Expr);

    d = d->children[0];
    switch (d->type) {
    case Name: {
        std::shared_ptr<Symbol> sym = symtab.get_symbol(d->strval);
        if (sym == nullptr)
            sclp_error(d->line, "Symbol " + d->strval + " not declared");
        return std::make_shared<AST::Sym>(sym);
    }
    case Expr_ARRAY: {
        std::shared_ptr<AST::Expr> left = handle_Expr(d->children[0], symtab);
        std::shared_ptr<AST::Expr> right = handle_Expr(d->children[1], symtab);
        return std::make_shared<AST::ArrayExpr>(d->line, left, right);
    }
    case Expr_DEREF:
        return handle_DerefExpr(d, symtab);
    default:
        assert(false);
    }
}
static std::shared_ptr<AST::CallExpr> handle_FuncCall(ParseNode* func_call, SymbolTable& symtab);
static std::shared_ptr<AST::Expr> handle_Expr(ParseNode* d, SymbolTable& symtab)
{
    assert(d->type == Expr);

    d = d->children[0];
    switch (d->type) {
    case Name: {
        std::shared_ptr<Symbol> sym = symtab.get_symbol(d->strval);
        if (sym == nullptr)
            sclp_error(d->line, "Symbol " + d->strval + " not declared");
        return std::make_shared<AST::Sym>(sym);
    }
    case IntLit:
        return std::make_shared<AST::IntLit>(d->intval);
    case FloatLit:
        return std::make_shared<AST::FloatLit>(d->floatval);
    case StrLit:
        return std::make_shared<AST::StrLit>(d->strval);
    case Expr_FUNC_CALL:
        return handle_FuncCall(d, symtab);
    default:
        break;
    }

    // unary operators
    if (d->type == Expr_ADDR) {
        std::shared_ptr<AST::LValExpr> left = handle_Expr_LValExpr(d->children[0], symtab);
        return std::make_shared<AST::AddrExpr>(left);
    }

    std::shared_ptr<AST::Expr> left = handle_Expr(d->children[0], symtab);
    switch (d->type) {
    case Expr_UMINUS:
        return std::make_shared<AST::NegExpr>(d->line, left);
    case Expr_NOT:
        return std::make_shared<AST::NotExpr>(d->line, left);
    case Expr_DEREF:
        return std::make_shared<AST::DerefExpr>(d->line, left);
    default:
        break;
    }

    // binary operators
    std::shared_ptr<AST::Expr> right = handle_Expr(d->children[1], symtab);
    switch (d->type) {
    case Expr_PLUS:
        return std::make_shared<AST::AddExpr>(d->line, left, right);
    case Expr_MINUS:
        return std::make_shared<AST::SubExpr>(d->line, left, right);
    case Expr_MULT:
        return std::make_shared<AST::MulExpr>(d->line, left, right);
    case Expr_DIV:
        return std::make_shared<AST::DivExpr>(d->line, left, right);
    case Expr_OR:
        return std::make_shared<AST::OrExpr>(d->line, left, right);
    case Expr_AND:
        return std::make_shared<AST::AndExpr>(d->line, left, right);
    case Expr_NOT_EQUAL:
        return std::make_shared<AST::NotEqualExpr>(d->line, left, right);
    case Expr_EQUAL:
        return std::make_shared<AST::EqualExpr>(d->line, left, right);
    case Expr_GREATER_THAN:
        return std::make_shared<AST::GreaterExpr>(d->line, left, right);
    case Expr_GREATER_THAN_EQUAL:
        return std::make_shared<AST::GreaterEqualExpr>(d->line, left, right);
    case Expr_LESS_THAN:
        return std::make_shared<AST::LessExpr>(d->line, left, right);
    case Expr_LESS_THAN_EQUAL:
        return std::make_shared<AST::LessEqualExpr>(d->line, left, right);
    case Expr_ARRAY:
        return std::make_shared<AST::ArrayExpr>(d->line, left, right);
    default:
        break;
    }

    // ternary operators
    std::shared_ptr<AST::Expr> rightmost = handle_Expr(d->children[2], symtab);
    switch (d->type) {
    case Expr_TERNARY:
        return std::make_shared<AST::TernaryExpr>(d->line, left, right, rightmost);
    default:
        assert(false);
    }
}

static std::shared_ptr<AST::PrintStmt> handle_PrintStmt(ParseNode* print_stmt, SymbolTable& symtab)
{
    assert(print_stmt->type == PrintStmt);
    std::shared_ptr<AST::Expr> expr = handle_Expr(print_stmt->children[0], symtab);
    return std::make_shared<AST::PrintStmt>(print_stmt->line, expr);
}

static std::shared_ptr<AST::ReadStmt> handle_ReadStmt(ParseNode* read_stmt, SymbolTable& symtab)
{
    assert(read_stmt->type == ReadStmt);
    std::shared_ptr<AST::LValExpr> expr = handle_Expr_LValExpr(read_stmt->children[0], symtab);

    if (expr->is_const())
        sclp_error(read_stmt->line, "Cannot read into const lvalue-expression");

    return std::make_shared<AST::ReadStmt>(read_stmt->line, expr);
}

static std::shared_ptr<AST::AssignStmt> handle_AssignStmt(ParseNode* assign_stmt, SymbolTable& symtab)
{
    assert(assign_stmt->type == AssignStmt);

    std::shared_ptr<AST::LValExpr> lhs = handle_Expr_LValExpr(assign_stmt->children[0], symtab);
    std::shared_ptr<AST::Expr> expr = handle_Expr(assign_stmt->children[1], symtab);

    if (lhs->is_const())
        sclp_error(assign_stmt->line, "Cannot assign to const lvalue-expression");

    return std::make_shared<AST::AssignStmt>(assign_stmt->line, lhs, expr);
}

static std::shared_ptr<AST::Stmt> handle_Stmt(ParseNode* stmt, SymbolTable& symtab);
static std::shared_ptr<AST::CompoundStmt> handle_CompoundStmt(ParseNode* compound_stmt, SymbolTable& symtab)
{
    assert(compound_stmt->type == CompoundStmt);

    ParseNode* stmt_list = compound_stmt->children[0];
    assert(stmt_list->type == StmtList);

    std::vector<std::shared_ptr<AST::Stmt>> children;
    for (ParseNode* stmt : stmt_list->children) {
        std::shared_ptr<AST::Stmt> a = handle_Stmt(stmt, symtab);
        if (a != nullptr)
            children.push_back(a);
    }

    return std::make_shared<AST::CompoundStmt>(children);
}

static std::shared_ptr<AST::IfStmt> handle_IfStmt(ParseNode* if_stmt, SymbolTable& symtab)
{
    assert(if_stmt->type == IfStmt);
    std::shared_ptr<AST::Expr> cond = handle_Expr(if_stmt->children[0], symtab);
    std::shared_ptr<AST::Stmt> body = handle_Stmt(if_stmt->children[1], symtab);

    return std::make_shared<AST::IfStmt>(if_stmt->line, cond, body);
}

static std::shared_ptr<AST::IfElseStmt> handle_IfElseStmt(ParseNode* if_else_stmt, SymbolTable& symtab)
{
    assert(if_else_stmt->type == IfElseStmt);
    std::shared_ptr<AST::Expr> cond = handle_Expr(if_else_stmt->children[0], symtab);
    std::shared_ptr<AST::Stmt> body = handle_Stmt(if_else_stmt->children[1], symtab);
    std::shared_ptr<AST::Stmt> else_body = handle_Stmt(if_else_stmt->children[2], symtab);

    return std::make_shared<AST::IfElseStmt>(if_else_stmt->line, cond, body, else_body);
}

static std::shared_ptr<AST::WhileStmt> handle_WhileStmt(ParseNode* while_stmt, SymbolTable& symtab)
{
    assert(while_stmt->type == WhileStmt);
    std::shared_ptr<AST::Expr> cond = handle_Expr(while_stmt->children[0], symtab);
    std::shared_ptr<AST::Stmt> body = (while_stmt->children[1] == nullptr ? nullptr : handle_Stmt(while_stmt->children[1], symtab));

    return std::make_shared<AST::WhileStmt>(while_stmt->line, cond, body);
}

static std::shared_ptr<AST::DoWhileStmt> handle_DoWhileStmt(ParseNode* do_while_stmt, SymbolTable& symtab)
{
    assert(do_while_stmt->type == DoWhileStmt);
    std::shared_ptr<AST::Stmt> body = handle_Stmt(do_while_stmt->children[0], symtab);
    std::shared_ptr<AST::Expr> cond = handle_Expr(do_while_stmt->children[1], symtab);

    return std::make_shared<AST::DoWhileStmt>(do_while_stmt->line, body, cond);
}

static std::shared_ptr<AST::ForStmt> handle_ForStmt(ParseNode* for_stmt, SymbolTable& symtab)
{
    assert(for_stmt->type == ForStmt);
    std::shared_ptr<AST::AssignStmt> pre_stmt = (for_stmt->children[0] == nullptr ? nullptr : handle_AssignStmt(for_stmt->children[0], symtab));
    std::shared_ptr<AST::Expr> cond = (for_stmt->children[1] == nullptr ? nullptr : handle_Expr(for_stmt->children[1], symtab));
    std::shared_ptr<AST::AssignStmt> inc_stmt = (for_stmt->children[2] == nullptr ? nullptr : handle_AssignStmt(for_stmt->children[2], symtab));

    std::shared_ptr<AST::Stmt> body = (for_stmt->children[3] == nullptr ? nullptr : handle_Stmt(for_stmt->children[3], symtab));

    return std::make_shared<AST::ForStmt>(for_stmt->line, pre_stmt, cond, inc_stmt, body);
}

static std::shared_ptr<AST::CallExpr> handle_FuncCall(ParseNode* func_call, SymbolTable& symtab)
{
    assert(func_call->type == Expr_FUNC_CALL);
    ParseNode* d = func_call->children[0];
    
    ParseNode* expr_list = func_call->children[1];
    assert(expr_list->type == ExprList);

    std::vector<std::shared_ptr<AST::Expr>> params;
    for (ParseNode* expr : expr_list->children)
        params.push_back(handle_Expr(expr, symtab));

    if (d->type == Expr) {
        d = d->children[0];
        if (d->type == Expr_DEREF) {
            std::shared_ptr<AST::DerefExpr> e = handle_DerefExpr(d, symtab);
            return std::make_shared<AST::FuncPtrCallExpr>(d->line, e, params);
        } else if (d->type != Name)
            sclp_error(d->line, "Bad function call expression");
    }

    std::shared_ptr<Symbol> func_sym = symtab.get_symbol(handle_Name(d));
    if (func_sym == nullptr)
        sclp_error(d->line, "Symbol " + d->strval + " not declared");

    return std::make_shared<AST::FuncCallExpr>(func_call->line, func_sym, params);
}
static std::shared_ptr<AST::CallStmt> handle_CallStmt(ParseNode* func_call_stmt, SymbolTable& symtab)
{
    assert(func_call_stmt->type == CallStmt);

    ParseNode* func_call = func_call_stmt->children[0];
    std::shared_ptr<AST::CallExpr> ce = handle_FuncCall(func_call, symtab);

    return std::make_shared<AST::CallStmt>(func_call->line, ce);
}

static std::shared_ptr<AST::ReturnStmt> handle_ReturnStmt(ParseNode* return_stmt, SymbolTable& symtab)
{
    assert(return_stmt->type == ReturnStmt);

    if (return_stmt->children.size() == 0) {
        if (!func_sym->semtype->get_ret_type()->is_void())
            sclp_error(return_stmt->line, "Return statement does not return value in non-void function");
        return std::make_shared<AST::ReturnStmt>(std::shared_ptr<AST::Expr>(nullptr));
    } else {
        ParseNode* expr = return_stmt->children[0];
        std::shared_ptr<AST::Expr> e = handle_Expr(expr, symtab);
        if (!SemType::check_assign(func_sym->semtype->get_ret_type(), e->semtype))
            sclp_error(expr->line, "Returned expression does not match declared return type");
        return std::make_shared<AST::ReturnStmt>(e);
    }
}

static std::shared_ptr<AST::Stmt> handle_Stmt(ParseNode* stmt, SymbolTable& symtab)
{
    assert(stmt->type == Stmt);
    ParseNode* d = stmt->children[0];
    switch (d->type) {
        case DeclStmt:
            handle_DeclStmt(d, symtab);
            return nullptr;
        case PrintStmt:
            return handle_PrintStmt(d, symtab);
        case ReadStmt:
            return handle_ReadStmt(d, symtab);
        case AssignStmt:
            return handle_AssignStmt(d, symtab);
        case CompoundStmt: {
            symtab.begin_scope();
            std::shared_ptr<AST::CompoundStmt> a;
            a = handle_CompoundStmt(d, symtab);
            symtab.end_scope();
            return a;
        }
        case IfStmt:
            return handle_IfStmt(d, symtab);
        case IfElseStmt:
            return handle_IfElseStmt(d, symtab);
        case WhileStmt:
            return handle_WhileStmt(d, symtab);
        case DoWhileStmt:
            return handle_DoWhileStmt(d, symtab);
        case ForStmt:
            return handle_ForStmt(d, symtab);
        case BreakStmt:
            return std::make_shared<AST::BreakStmt>(d->line);
        case ContinueStmt:
            return std::make_shared<AST::ContinueStmt>(d->line);
        case CallStmt:
            return handle_CallStmt(d, symtab);
        case ReturnStmt:
            return handle_ReturnStmt(d, symtab);
        default:
            assert(false);
    }
}

static AST::FuncDefn handle_FuncDef(ParseNode* func_def, SymbolTable& symtab)
{
    assert(func_def->type == FuncDef);

    ParseNode* single_decl = func_def->children[0];

    Symbol fs = handle_SingleDecl(single_decl, symtab);

    if (!fs.semtype->is_func())
        sclp_error(single_decl->line, "Definition for non-function symbol");

    func_sym = symtab.put_symbol(fs);
    if (func_sym == nullptr)
        sclp_error(single_decl->line, "Function signature does not match previous declaration");

    symtab.begin_scope();

    std::vector<std::shared_ptr<Symbol>> func_params_sym;
    for (auto s : func_params)
        if (s->name.length() > 0) {
            auto param_sym = symtab.put_symbol(*s);
            if (param_sym == nullptr)
                sclp_error(single_decl->line, "Symbol " + s->name + " redeclared");
            func_params_sym.push_back(param_sym);
        }
    
    std::shared_ptr<AST::CompoundStmt> a = handle_CompoundStmt(func_def->children[1], symtab);

    symtab.end_scope();

    return AST::FuncDefn(func_def->line, func_sym, func_params_sym, a);
}

std::vector<AST::FuncDefn> AST::from_parse_tree(ParseNode* program, SymbolTable& symtab)
{
    assert(program->type == Program);

    ParseNode* global_decl_defn_list = program->children[0];
    assert(global_decl_defn_list->type == GlobalDeclDefnList);

    std::vector<AST::FuncDefn> ret;

    for (ParseNode* d : global_decl_defn_list->children) {
        switch (d->type) {
        case DeclStmt:
            handle_DeclStmt(d, symtab);
            break;
        case FuncDef:
            tacctx = TAC::Context();
            ret.push_back(handle_FuncDef(d, symtab));
            ret[ret.size() - 1].ctx = tacctx;
            break;
        default:
            assert(false);
        }
    }

    return ret;
}
