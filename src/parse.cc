#include "parse.h"

void ParseNode::print_helper(std::string prefix) const
{
    static char const* ParseNodeTypeName[ParseNodeTypeNr] = {
        "Program",
        "GlobalDeclDefnList",
        "FuncDefn",

        "MultiDecl", "SingleDecl",
        "TypeModNEList",
        "TypeMod",
        "ArrayMod",
        "FuncMod",
        "ConstAsteriskList",
        "ArrayNEList",
        "PrimType",
        "ParamList",
        "ConstOptName", "Const", "NotConst", "OptName",

        "Stmt",
        "DeclStmt",
        "PrintStmt",
        "ReadStmt",
        "AssignStmt",
        "CompoundStmt",
        "IfStmt",
        "IfElseStmt",
        "WhileStmt",
        "DoWhileStmt",
        "ForStmt",
        "BreakStmt",
        "ContinueStmt",
        "CallStmt",
        "ReturnStmt",
        "StmtList",

        "Expr",
        "Expr_TERNARY",
        "Expr_OR",
        "Expr_AND",
        "Expr_NOT",
        "Expr_NOT_EQUAL",
        "Expr_EQUAL",
        "Expr_LESS_THAN",
        "Expr_LESS_THAN_EQUAL",
        "Expr_GREATER_THAN",
        "Expr_GREATER_THAN_EQUAL",
        "Expr_PLUS",
        "Expr_MINUS",
        "Expr_MULT",
        "Expr_DIV",
        "Expr_UMINUS",
        "Expr_ADDR",
        "Expr_DEREF",
        "Expr_ARRAY",
        "Expr_FUNC_CALL",
        "ExprCommaList",

        "",

        "Void", "Bool", "Int", "Float", "String",
        "Name",
        "IntLit",
        "FloatLit",
        "StrLit",
        "Asterisk",
    };

    std::cout << prefix << ParseNodeTypeName[type] << '[' << line << ']';
    switch (type) {
    case Name:
    case StrLit:
        std::cout << ": " << strval << '\n';
        break;
    case IntLit:
        std::cout << ": " << intval << '\n';
        break;
    case FloatLit:
        std::cout << ": " << floatval << '\n';
        break;
    default:
        std::cout << '\n';
        for (auto c : children) {
            if (c != nullptr)
                c->print_helper(prefix + "    ");
        }
    }
}

void ParseNode::free_tree()
{
    using std::string, std::vector;
    if (type < Terminals) {
        for (auto c : children)
            delete c;
        (&children)->~vector<ParseNode*>();
    } else if (type == StrLit || type == Name)
        (&strval)->string::~string();
}

extern "C" {
    ParseNode* construct_ParseNode_0c(size_t line, ParseNodeType type)
    {
        return new ParseNode(line, type);
    }
    ParseNode* construct_ParseNode_1c(size_t line, ParseNodeType type, ParseNode* c1)
    {
        return new ParseNode(line, type, std::initializer_list<ParseNode*>{ c1 });
    }
    ParseNode* construct_ParseNode_2c(size_t line, ParseNodeType type, ParseNode* c1, ParseNode* c2)
    {
        return new ParseNode(line, type, std::initializer_list<ParseNode*>{ c1, c2 });
    }
    ParseNode* construct_ParseNode_3c(size_t line, ParseNodeType type, ParseNode* c1, ParseNode* c2, ParseNode* c3)
    {
        return new ParseNode(line, type, std::initializer_list<ParseNode*>{ c1, c2, c3 });
    }
    ParseNode* construct_ParseNode_4c(size_t line, ParseNodeType type, ParseNode* c1, ParseNode* c2, ParseNode* c3, ParseNode* c4)
    {
        return new ParseNode(line, type, std::initializer_list<ParseNode*>{ c1, c2, c3, c4 });
    }
    ParseNode* construct_ParseNode_int(size_t line, ParseNodeType type, size_t intval)
    {
        return new ParseNode(line, type, intval);
    }
    ParseNode* construct_ParseNode_float(size_t line, ParseNodeType type, double floatval)
    {
        return new ParseNode(line, type, floatval);
    }
    ParseNode* construct_ParseNode_str(size_t line, ParseNodeType type, char const* strval)
    {
        return new ParseNode(line, type, std::string(strval));
    }
    void ParseNode_add_child(ParseNode* p, ParseNode* c)
    {
        p->add_child(c);
    }
}
