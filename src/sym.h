#ifndef SYM_H
#define SYM_H

#include <types.h>
#include <string>
#include <memory>

struct Symbol {
    std::string name;
    SemType const* semtype;

    // only for vars
    bool is_const;
    bool is_global;

    Symbol(std::string name, SemType const* st, bool is_const = false)
        : name(name), semtype(st), is_const(is_const)
    {
    }
};

struct ScopeNode;

class SymbolTable {
private:
    std::shared_ptr<ScopeNode> root;
    std::shared_ptr<ScopeNode> curr;

public:
    SymbolTable();

    void begin_scope();
    void end_scope();

    std::shared_ptr<Symbol> get_symbol(std::string name);
    std::shared_ptr<Symbol> put_symbol(Symbol s);

    std::vector<std::shared_ptr<Symbol>> const& get_global_vars() const;
};

#endif // SYM_H
