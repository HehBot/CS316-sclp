
#include <tac.h>
#include <types.h>
#include <sym.h>
#include <memory>
#include <string>
#include <cassert>
#include <iostream>

using namespace TAC;

size_t Context::next_label = 0;
Context::Context()
    : next_temp(0), next_stemp(0),
        stackframe_size(4), // TODO @nilabha justify these
        paramframe_size(8)
{
}

std::string const Context::temp_prefix = "temp", Context::stemp_prefix = "stemp";
std::string const Context::label_prefix = "Label";

std::shared_ptr<Sym> Context::get_temp(Type t)
{
    std::string name = temp_prefix + std::to_string(next_temp);
    next_temp++;
    while (names_used.count(name) > 0) {
        name = temp_prefix + std::to_string(next_temp);
        next_temp++;
    }
    names_used.insert(name);
    return std::make_shared<Sym>(name, t, false);
}
std::shared_ptr<Sym> Context::get_stemp(Type t)
{
    std::string name = stemp_prefix + std::to_string(next_stemp);
    next_stemp++;
    while (names_used.count(name) > 0) {
        name = stemp_prefix + std::to_string(next_stemp);
        next_stemp++;
    }
    names_used.insert(name);

    std::shared_ptr<Sym> ret = std::make_shared<Sym>(name, t, true);
    size_t sz = TAC::get_type_size(t);
    ret->fp_offset = -(stackframe_size + sz - 4);
    stackframe_size += sz;
    return ret;
}

std::shared_ptr<Sym> Context::get_symbol(std::shared_ptr<Symbol> s)
{
    auto it = table.find(s);
    if (it != table.end())
        return it->second;

    std::shared_ptr<Sym> tacsym;
    if (names_used.count(s->name) > 0)
        tacsym = get_temp(s->semtype->to_tactype());
    else {
        names_used.insert(s->name);
        tacsym = std::make_shared<Sym>(s->name, s->semtype->to_tactype(), true);
    }

    if (!s->is_global) {
        size_t sz = s->semtype->size();
        tacsym->fp_offset = -(stackframe_size + sz - 4);
        stackframe_size += sz;
    } else{
        tacsym->is_global = true;
        tacsym->fp_offset = -1;
    }
    table[s] = tacsym;
    return tacsym;
}
std::shared_ptr<Sym> Context::add_param_symbol(std::shared_ptr<Symbol> s)
{
    auto it = table.find(s);
    assert(it == table.end());

    std::shared_ptr<Sym> tacsym;
    assert(names_used.count(s->name) == 0);
    names_used.insert(s->name);
    tacsym = std::make_shared<Sym>(s->name, s->semtype->to_tactype(), true);

    assert(!s->is_global);
    tacsym->fp_offset = paramframe_size;
    paramframe_size += s->semtype->size();
    table[s] = tacsym;
    return tacsym;
}
std::shared_ptr<Label> Context::get_label()
{
    std::string name = label_prefix + std::to_string(next_label);
    next_label++;
    return std::make_shared<Label>(name);
}
