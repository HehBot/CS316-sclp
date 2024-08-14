#include <error.h>
#include <types.h>
#include <cassert>
#include <iostream>
#include <memory>

void SemType::print(std::ostream& o) const
{
    static char const* names[(size_t)Category::Nr] = { "void", "bool", "int", "float", "string", "", "", "" };
    switch (category) {
    case Category::VOID:
    case Category::BOOL:
    case Category::INT:
    case Category::FLOAT:
    case Category::STRING:
        o << names[(size_t)category];
        break;
    case Category::PTR:
        u.p.points_to->print(o);
        if (u.p.points_to_const)
            o << " const";
        o << '*';
        break;
    case Category::ARRAY:
        o << '<';
        u.a.element_type->print(o);
        o << '>';
        o << '[' << u.a.size << ']';
        break;
    case Category::FUNC:
        o << '<';
        u.f.ret->print(o);
        o << '(';
        {
            auto it = u.f.params.begin();
            auto const it_end = u.f.params.end();
            if (it != it_end) {
                (*it)->print(o);
                ++it;
                for (; it != it_end; ++it) {
                    o << ", ";
                    (*it)->print(o);
                }
            }
        }
        o << ')';
        o << '>';
        break;
    default:
        assert(false);
    }
}
TAC::Type SemType::to_tactype() const
{
    switch (category) {
    case Category::BOOL:
        return TAC::Type::BOOL;
    case Category::INT:
        return TAC::Type::INT;
    case Category::FLOAT:
        return TAC::Type::FLOAT;
    case Category::STRING:
        return TAC::Type::STRING;
    case Category::PTR:
    case Category::ARRAY:
    case Category::FUNC:
        return TAC::Type::PTR;
    default:
        assert(false);
    }
}

SemType const* SemType::make_void()
{
    static SemType singleton{SemType::Category::VOID};
    return &singleton;
}
SemType const* SemType::make_bool()
{
    static SemType singleton{SemType::Category::BOOL};
    return &singleton;
}
SemType const* SemType::make_int()
{
    static SemType singleton{SemType::Category::INT};
    return &singleton;
}
SemType const* SemType::make_float()
{
    static SemType singleton{SemType::Category::FLOAT};
    return &singleton;
}
SemType const* SemType::make_string()
{
    static SemType singleton{SemType::Category::STRING};
    return &singleton;
}

SemType const* SemType::make_ptr(SemType const* points_to, bool points_to_const)
{
    static std::vector<std::shared_ptr<SemType>> cache;

    for (auto const& p : cache)
        if (p->u.p.points_to == points_to && p->u.p.points_to_const == points_to_const)
            return p.get();
    std::shared_ptr<SemType> p(new SemType(points_to, points_to_const));
    cache.push_back(p);
    return p.get();
}

SemType const* SemType::make_array(SemType const* element_type, size_t size)
{
    static std::vector<std::shared_ptr<SemType>> cache;

    if (element_type->category == SemType::Category::VOID) {
        aux_error_msg = "Array declared as void type";
        return nullptr;
    }
    else if (element_type->is_func()) {
        aux_error_msg = "Array of functions";
        return nullptr;
    } else if (size == 0) {
        aux_error_msg = "Array declared with zero size";
        return nullptr;
    }
    for (auto const& a : cache)
        if (a->u.a.element_type == element_type && a->u.a.size == size)
            return a.get();
    std::shared_ptr<SemType> a(new SemType(element_type, size));
    cache.push_back(a);
    return a.get();
}

SemType const* SemType::make_func(SemType const* ret, std::vector<SemType const*> const& params)
{
    static std::vector<std::shared_ptr<SemType>> cache;

    if (ret->is_func()) {
        aux_error_msg = "Function returning function";
        return nullptr;
    } else if (ret->is_array()) {
        aux_error_msg = "Function returning array";
        return nullptr;
    }

    for (auto const& param_type : params) {
        if (param_type->is_func()) {
            aux_error_msg = "Parameter declared as function";
            return nullptr;
        }
    }
    size_t l = params.size();
    for (auto const& f : cache) {
        if (f->u.f.ret != ret || f->u.f.params.size() != l)
            continue;
        bool match = true;
        for(size_t i = 0; i < l; ++i)
            if (params[i] != f->u.f.params[i]) {
                match = false;
                break;
            }
        if (match)
            return f.get();
    }
    std::shared_ptr<SemType> f(new SemType(ret, params));
    cache.push_back(f);
    return f.get();
}

bool SemType::check_assign(SemType const* s1, SemType const* s2)
{
    if (s1->category == SemType::Category::VOID || s2->category == SemType::Category::VOID) {
        aux_error_msg = "Void value not ignored as it ought to be";
        return false;
    }
    switch (s1->category) {
    case SemType::Category::ARRAY:
        aux_error_msg = "Cannot assign to array type";
        return false;
    case SemType::Category::FUNC:
        aux_error_msg = "Cannot assign to a function";
        return false;
    case SemType::Category::PTR:
        {
            if (s2->category != SemType::Category::PTR
                    || s1->u.p.points_to != s2->u.p.points_to)
                return false;
            //  allow
            //      X const* a; X* b;
            //      a = b;
            return s1->u.p.points_to_const || !s2->u.p.points_to_const;
        }
    case SemType::Category::VOID:
        assert(false);
    default:
        return s1 == s2;
    }
}
bool SemType::check(SemType::StmtUn op, SemType const* s)
{
    if (s->category == SemType::Category::VOID) {
        aux_error_msg = "Void value not ignored as it ought to be";
        return false;
    }
    SemType::Category cat = s->category;
    switch (op) {
    case SemType::StmtUn::Print:
        if (cat == SemType::Category::STRING || cat == SemType::Category::INT || cat == SemType::Category::FLOAT)
            return true;
        else {
            aux_error_msg = "Can only print types string, int, or float, found type ";
            return false;
        }
    case SemType::StmtUn::Read:
        if (cat == SemType::Category::INT || cat == SemType::Category::FLOAT)
            return true;
        else {
            aux_error_msg = "Can only read types int or float, found type ";
            return false;
        }
    default:
        // only print and read should be here
        assert(false);
    }
}

SemType const* SemType::result(SemType const* s1, SemType const* s2, SemType const* s3)
{
    if (s1->category == SemType::Category::VOID) {
        aux_error_msg = "Void value not ignored as it ought to be";
        return nullptr;
    }
    // condition should be bool
    if (s1->category != SemType::Category::BOOL) {
        aux_error_msg = "Ternary operator condition must be of type bool, found type ";
        return nullptr;
    }
    // operands should be of same type (even void is fine!)
    if (s2 != s3) {
        aux_error_msg = "Ternary operator operands should be of same type, found types ";
        return nullptr;
    }
    return s2;
}
SemType const* SemType::result(SemType::ExprBin op, SemType const* s1, SemType const* s2)
{
    if (s1->category == SemType::Category::VOID || s2->category == SemType::Category::VOID) {
        aux_error_msg = "Void value not ignored as it ought to be";
        return nullptr;
    }

    switch (op) {
    case SemType::ExprBin::AddSub:
        if (s1->category == SemType::Category::PTR && s2->category == SemType::Category::INT)
            return s1;
    case SemType::ExprBin::OtherArith:
        if (s1 == s2 && (s1->category == SemType::Category::INT || s1->category == SemType::Category::FLOAT))
            return s1;
        break;
    case SemType::ExprBin::Comp:
        if (s1 == s2 && (s1->category == SemType::Category::INT || s1->category == SemType::Category::FLOAT))
            return SemType::make_bool();
        break;
    case SemType::ExprBin::Logic:
        if (s1 == s2 && s1->category == SemType::Category::BOOL)
            return s1;
        break;
    case SemType::ExprBin::Array:
        if ((s1->category == SemType::Category::ARRAY || s1->category == SemType::Category::PTR) && s2->category == SemType::Category::INT)
            return s1->get_element_type();
    }
    return nullptr;
}
SemType const* SemType::result(SemType::ExprUn op, SemType const* s)
{
    if (s->category == SemType::Category::VOID) {
        aux_error_msg = "Void value not ignored as it ought to be";
        return nullptr;
    }

    switch (op) {
    case SemType::ExprUn::Neg:
        if (s->category == SemType::Category::INT || s->category == SemType::Category::FLOAT)
            return s;
        break;
    case SemType::ExprUn::Not:
        if (s->category == SemType::Category::BOOL)
            return s;
        break;
    case SemType::ExprUn::Deref:
        if (s->category == SemType::Category::PTR)
            return s->u.p.points_to;
    }
    return nullptr;
}
SemType const* SemType::result(SemType const* func, std::vector<SemType const*> params)
{
    std::vector<SemType const*> const& params_required = func->u.f.params;
    if (params_required.size() != params.size())
        return nullptr;

    size_t i = 0;
    for (SemType const* param_required : params_required) {
        if (!check_assign(param_required, params[i]))
            return nullptr;
        ++i;
    }
    return func->u.f.ret;
}
