#include <algorithm>
#include <cassert>
#include <memory>
#include <sym.h>
#include <tac.h>
#include <unordered_map>

struct ScopeNode {
    std::shared_ptr<ScopeNode> parent;

    std::unordered_map<std::string, size_t> var_map;
    std::vector<std::shared_ptr<Symbol>> var_list;

    std::unordered_map<std::string, size_t> func_map;
    std::vector<std::shared_ptr<Symbol>> func_list;

    ScopeNode(std::shared_ptr<ScopeNode> parent)
        : parent(parent)
    {
    }
    bool is_global() const
    {
        return parent == nullptr;
    }
};

SymbolTable::SymbolTable()
{
    root = std::make_shared<ScopeNode>(nullptr);
    curr = root;
}

void SymbolTable::begin_scope()
{
    std::shared_ptr<ScopeNode> child = std::make_shared<ScopeNode>(curr);
    curr = child;
}
void SymbolTable::end_scope()
{
    assert(curr->parent != nullptr);
    curr = curr->parent;
}

std::shared_ptr<Symbol> SymbolTable::get_symbol(std::string name)
{
    std::shared_ptr<ScopeNode> traversing = curr;
    while (traversing != nullptr) {
        auto it = traversing->var_map.find(name);
        if (it != traversing->var_map.end())
            return traversing->var_list[it->second];
        it = traversing->func_map.find(name);
        if (it != traversing->func_map.end())
            return traversing->func_list[it->second];
        traversing = traversing->parent;
    }
    return nullptr;
}

std::shared_ptr<Symbol> SymbolTable::put_symbol(Symbol s)
{
    if (s.semtype->is_func()) {
        // functions can't be declared with same name as vars in current scope
        auto it1 = curr->var_map.find(s.name);
        if (it1 != curr->var_map.end())
            return nullptr;

        // same function can be declared multiple times in the same scope
        // provided type is same
        // each corresponds to the same function
        auto hit = get_symbol(s.name);
        if (hit != nullptr) {
            if (hit->semtype != s.semtype)
                return nullptr;
            else
                return hit;
        }

        s.is_global = true;
        curr->func_map[s.name] = curr->func_list.size();

        std::shared_ptr<Symbol> new_entry = std::make_shared<Symbol>(s);
        curr->func_list.push_back(new_entry);
        return new_entry;
    } else {
        auto it1 = curr->var_map.find(s.name);
        if (it1 != curr->var_map.end())
            return nullptr;
        auto it2 = curr->func_map.find(s.name);
        if (it2 != curr->func_map.end())
            return nullptr;
        s.is_global = (curr->is_global());
        curr->var_map[s.name] = curr->var_list.size();
        std::shared_ptr<Symbol> new_entry = std::make_shared<Symbol>(s);
        curr->var_list.push_back(new_entry);
        return new_entry;
    }
}

std::vector<std::shared_ptr<Symbol>> const& SymbolTable::get_global_vars() const
{
    return root->var_list;
}
