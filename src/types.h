#ifndef TYPES_H
#define TYPES_H

#include <cassert>
#include <vector>
#include <iostream>
#include <tac.h>

class SemType {
private:
    enum class Category {
        VOID, BOOL, INT, FLOAT, STRING, PTR, ARRAY, FUNC, Nr
    } const category;
    union U {
        struct P {
            SemType const* points_to;
            bool points_to_const;
            P(SemType const* points_to, bool points_to_const)
                : points_to(points_to), points_to_const(points_to_const) {}
        } p;
        struct A {
            SemType const* element_type;
            size_t size;
            A(SemType const* element_type, size_t size)
                : element_type(element_type), size(size) {}
        } a;
        struct F {
            SemType const* ret;
            std::vector<SemType const*> params;
            F(SemType const* ret, std::vector<SemType const*> const& params)
                : ret(ret), params(params) {}
        } f;
        U()
            : p(nullptr, false) {}
        U(SemType const* x, bool y)
            : p(x, y) {}
        U(SemType const* x, size_t y)
            : a(x, y) {}
        U(SemType const* x, std::vector<SemType const*> const& y)
            : f(x, y) {}
        ~U() {}
    } const u;
    SemType(Category c)
        : category(c)
    {
        if (c == Category::PTR || c == Category::ARRAY || c == Category::FUNC)
            assert(false);
    }
    SemType(SemType const* x, bool y)
        : category(Category::PTR), u(x, y) {}
    SemType(SemType const* x, size_t y)
        : category(Category::ARRAY), u(x, y) {}
    SemType(SemType const* x, std::vector<SemType const*> const& y)
        : category(Category::FUNC), u(x, y) {}

public:
    ~SemType()
    {
        switch (category) {
        case Category::FUNC:
            (&this->u.f.params)->~vector();
        default:
            break;
        }
    }
    void print(std::ostream&) const;
    TAC::Type to_tactype() const;

    static SemType const* make_void();
    static SemType const* make_bool();
    static SemType const* make_int();
    static SemType const* make_float();
    static SemType const* make_string();
    static SemType const* make_ptr(SemType const* points_to, bool points_to_const);
    static SemType const* make_array(SemType const* element_type, size_t size);
    static SemType const* make_func(SemType const* ret, std::vector<SemType const*> const& params);

    bool is_void() const
    {
        return category == Category::VOID;
    }
    bool is_float() const
    {
        return category == Category::FLOAT;
    }
    bool is_func() const
    {
        return category == Category::FUNC;
    }
    bool is_array() const
    {
        return category == Category::ARRAY;
    }
    bool is_ptr() const
    {
        return category == Category::PTR;
    }
    SemType const* get_ret_type() const
    {
        assert(category == Category::FUNC);
        return u.f.ret;
    }
    SemType const* get_points_to_type() const
    {
        assert(category == Category::PTR);
        return u.p.points_to;
    }
    bool get_points_to_const() const
    {
        assert(category == Category::PTR);
        return u.p.points_to_const;
    }
    SemType const* get_element_type() const
    {
        switch (category) {
        case Category::ARRAY:
            return u.a.element_type;
        case Category::PTR:
            return u.p.points_to;
        default:
            assert(false);
        }
    }
    size_t size() const
    {
        switch (category) {
        case Category::INT:
        case Category::BOOL:
        case Category::STRING:
        case Category::PTR:
        case Category::FLOAT:
            return TAC::get_type_size(to_tactype());
        case Category::ARRAY:
            return u.a.size * u.a.element_type->size();
        default:
            assert(false);
        }
    }
    static bool check_assign(SemType const* s1, SemType const* s2);
    enum class StmtUn {
        Print, Read
    };
    static bool check(StmtUn op, SemType const* s);

    static SemType const* result(SemType const* s1, SemType const* s2, SemType const* s3);
    enum class ExprBin {
        AddSub, OtherArith, Comp, Logic, Array
    };
    static SemType const* result(ExprBin op, SemType const* s1, SemType const* s2);
    enum class ExprUn {
        Neg, Not, Deref
    };
    static SemType const* result(ExprUn op, SemType const* s);
    static SemType const* result(SemType const* func_type, std::vector<SemType const*> param_types);
};

#endif // TYPES_H
