#ifndef PARSE_H
#define PARSE_H

typedef enum {
    Program,
    GlobalDeclDefnList,
    FuncDef,

    MultiDecl, SingleDecl,
    TypeModNEList,
    TypeMod,
    ArrayMod,
    FuncMod,
    ConstAsteriskList,
    ArrayNEList,
    PrimType,
    ParamList,
    ConstOptName, Const, NotConst, OptName,

    Stmt,
    DeclStmt,
    PrintStmt,
    ReadStmt,
    AssignStmt,
    CompoundStmt,
    IfStmt,
    IfElseStmt,
    WhileStmt,
    DoWhileStmt,
    ForStmt,
    BreakStmt,
    ContinueStmt,
    CallStmt,
    ReturnStmt,
    StmtList,

    Expr,
    Expr_TERNARY,
    Expr_OR,
    Expr_AND,
    Expr_NOT,
    Expr_NOT_EQUAL,
    Expr_EQUAL,
    Expr_LESS_THAN,
    Expr_LESS_THAN_EQUAL,
    Expr_GREATER_THAN,
    Expr_GREATER_THAN_EQUAL,
    Expr_PLUS,
    Expr_MINUS,
    Expr_MULT,
    Expr_DIV,
    Expr_UMINUS,
    Expr_ADDR,
    Expr_DEREF,
    Expr_ARRAY,
    Expr_FUNC_CALL,
    ExprList,

    Terminals,

    Void, Bool, Int, Float, String,
    Name,
    IntLit,
    FloatLit,
    StrLit,
    Asterisk,
    ParseNodeTypeNr
} ParseNodeType;

#ifdef __cplusplus

#include <iostream>
#include <string>
#include <vector>

struct ParseNode {
private:
    void print_helper(std::string prefix) const;
    void free_tree();

public:
    size_t line;
    ParseNodeType const type;
    union {
        std::vector<ParseNode*> children;
        std::size_t const intval;
        double const floatval;
        std::string const strval;
    };

    ParseNode(size_t line, ParseNodeType type)
        : line(line), type(type), children{}
    {
    }
    ParseNode(size_t line, ParseNodeType type, std::initializer_list<ParseNode*> const& children)
        : line(line), type(type), children{children}
    {
    }
    ParseNode(size_t line, ParseNodeType type, double floatval)
        : line(line), type(type), floatval(floatval)
    {
    }
    ParseNode(size_t line, ParseNodeType type, std::size_t intval)
        : line(line), type(type), intval(intval)
    {
    }
    ParseNode(size_t line, ParseNodeType type, std::string strval)
        : line(line), type(type), strval(strval)
    {
    }
    ~ParseNode()
    {
        free_tree();
    }

    void add_child(ParseNode* c)
    {
        children.push_back(c);
    }

    void print() const
    {
        print_helper("");
    }
};
#else

#include <stddef.h>
struct ParseNode;
struct ParseNode* construct_ParseNode_0c(size_t, ParseNodeType);
struct ParseNode* construct_ParseNode_1c(size_t, ParseNodeType, struct ParseNode*);
struct ParseNode* construct_ParseNode_2c(size_t, ParseNodeType, struct ParseNode*, struct ParseNode*);
struct ParseNode* construct_ParseNode_3c(size_t, ParseNodeType, struct ParseNode*, struct ParseNode*, struct ParseNode*);
struct ParseNode* construct_ParseNode_4c(size_t, ParseNodeType, struct ParseNode*, struct ParseNode*, struct ParseNode*, struct ParseNode*);
struct ParseNode* construct_ParseNode_int(size_t line, ParseNodeType type, size_t intval);
struct ParseNode* construct_ParseNode_float(size_t line, ParseNodeType type, double floatval);
struct ParseNode* construct_ParseNode_str(size_t line, ParseNodeType type, char const* str);
void ParseNode_add_child(struct ParseNode* p, struct ParseNode* c);
#endif

#endif // PARSE_H
