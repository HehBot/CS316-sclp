%{
#include <error.h>
#include <parse.h>
#include <stddef.h>
#include <stdio.h>

extern int yylineno;
int yylex();
void yyerror(struct ParseNode**, char const*);

#define construct_ParseNode_int(X, Y) construct_ParseNode_int(yylineno, X, Y)
#define construct_ParseNode_float(X, Y) construct_ParseNode_float(yylineno, X, Y)
#define construct_ParseNode_str(X, Y) construct_ParseNode_str(yylineno, X, Y)
#define GET_MACRO(_0, _1, _2, _3, _4, NAME, ...) NAME
#define construct_ParseNode(...) GET_MACRO(__VA_ARGS__, construct_ParseNode_4c, construct_ParseNode_3c, construct_ParseNode_2c, construct_ParseNode_1c, construct_ParseNode_0c)(yylineno, __VA_ARGS__)

static int nr_func_decl = 0;

%}

%debug
%define parse.error verbose
%define parse.lac full

%union {
    char* strval;
    size_t intval;
    double floatval;
    struct ParseNode* n;
};

%token LEFT_ROUND_BRACKET RIGHT_ROUND_BRACKET LEFT_CURLY_BRACKET RIGHT_CURLY_BRACKET LEFT_SQUARE_BRACKET RIGHT_SQUARE_BRACKET SEMICOLON COMMA AMP
%token CONST VOID INTEGER FLOAT BOOL STRING WRITE READ
%token IF ELSE DO WHILE FOR BREAK CONTINUE RETURN
%token ASSIGN_OP
%token<intval> INT_NUM
%token<floatval> FLOAT_NUM
%token<strval> NAME STR_CONST

%right QUESTION_MARK COLON
%left OR
%left AND
%nonassoc NOT
%nonassoc NOT_EQUAL EQUAL
%nonassoc LESS_THAN LESS_THAN_EQUAL GREATER_THAN GREATER_THAN_EQUAL
%left PLUS MINUS
%left MULT DIV
%nonassoc UMINUS

%precedence THEN
%precedence ELSE

%start Program
%parse-param {struct ParseNode** ret}

%type<n> PrimType TypeMod ArrayMod FuncMod
%type<n> ConstAsteriskList ArrayNEList TypeModNEList

%type<n> GlobalDeclDefnList FuncDef MultiDecl SingleDecl ConstOptName OptName ParamList ParamNEList

%type<n> StmtList Stmt DeclStmt PrintStmt ReadStmt AssignStmt CompoundStmt IfStmt WhileStmt DoWhileStmt ForStmt BreakStmt ContinueStmt CallStmt ReturnStmt OptionalExpr OptionalAssignStmtKern

%type<n> LValExpr DerefExpr ArrayExpr RValExpr Expr ExprList ExprNEList

%type<n> Name IntLit FloatLit StrLit FuncCall

%%

Program:
    GlobalDeclDefnList YYEOF {
        *ret = construct_ParseNode(Program, $1);
    }
;
GlobalDeclDefnList:
    GlobalDeclDefnList DeclStmt {
        $$ = $1;
        ParseNode_add_child($$, $2);
    }
|   GlobalDeclDefnList FuncDef {
        $$ = $1;
        ParseNode_add_child($$, $2);
    }
|   DeclStmt {
        $$ = construct_ParseNode(GlobalDeclDefnList, $1);
    }
|   FuncDef {
        $$ = construct_ParseNode(GlobalDeclDefnList, $1);
    }
;
FuncDef:
    SingleDecl CompoundStmt {
        $$ = construct_ParseNode(FuncDef, $1, $2);
    }
;

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Declarations ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
MultiDecl:
    PrimType TypeModNEList {
        $$ = construct_ParseNode(MultiDecl, $1, $2);
    }
;
SingleDecl:
    PrimType TypeMod {
        $$ = construct_ParseNode(SingleDecl, $1, $2);
    }
TypeModNEList:
    TypeModNEList COMMA TypeMod {
        $$ = $1;
        ParseNode_add_child($$, $3);
    }
|   TypeMod {
        $$ = construct_ParseNode(TypeModNEList);
        ParseNode_add_child($$, $1);
    }
;
TypeMod:
    ArrayMod {
        $$ = construct_ParseNode(TypeMod, $1);
    }
|   FuncMod {
        $$ = construct_ParseNode(TypeMod, $1);
    }
|   ConstAsteriskList LEFT_ROUND_BRACKET TypeMod RIGHT_ROUND_BRACKET {
        $$ = construct_ParseNode(TypeMod, $1, $3);
    }
|   ConstAsteriskList ConstOptName {
        $$ = construct_ParseNode(TypeMod, $1, $2);
    }
|   ConstOptName {
        $$ = construct_ParseNode(TypeMod, $1);
    }
;
ArrayMod:
    ConstAsteriskList LEFT_ROUND_BRACKET TypeMod RIGHT_ROUND_BRACKET ArrayNEList {
        $$ = construct_ParseNode(ArrayMod, $1, $3, $5);
    }
|   LEFT_ROUND_BRACKET TypeMod RIGHT_ROUND_BRACKET ArrayNEList {
        $$ = construct_ParseNode(ArrayMod, $2, $4);
    }
|   ConstAsteriskList ConstOptName ArrayNEList {
        $$ = construct_ParseNode(ArrayMod, $1, $2, $3);
    }
|   ConstOptName ArrayNEList {
        $$ = construct_ParseNode(ArrayMod, $1, $2);
    }
;
FuncMod:
    ConstAsteriskList LEFT_ROUND_BRACKET TypeMod RIGHT_ROUND_BRACKET LEFT_ROUND_BRACKET ParamList RIGHT_ROUND_BRACKET {
        $$ = construct_ParseNode(FuncMod, $1, $3, $6);
    }
|   LEFT_ROUND_BRACKET TypeMod RIGHT_ROUND_BRACKET LEFT_ROUND_BRACKET ParamList RIGHT_ROUND_BRACKET {
        $$ = construct_ParseNode(FuncMod, $2, $5);
    }
|   ConstAsteriskList Name LEFT_ROUND_BRACKET ParamList RIGHT_ROUND_BRACKET {
        $$ = construct_ParseNode(FuncMod, $1, $2, $4);
    }
|   Name LEFT_ROUND_BRACKET ParamList RIGHT_ROUND_BRACKET {
        $$ = construct_ParseNode(FuncMod, $1, $3);
    }
;
ConstAsteriskList:
    ConstAsteriskList CONST MULT {
        $$ = $1;
        ParseNode_add_child($$, construct_ParseNode(Asterisk));
        ParseNode_add_child($$, construct_ParseNode(Const));
    }
|   ConstAsteriskList MULT {
        $$ = $1;
        ParseNode_add_child($$, construct_ParseNode(Asterisk));
    }
|   MULT {
        $$ = construct_ParseNode(ConstAsteriskList);
        ParseNode_add_child($$, construct_ParseNode(Asterisk));
    }
;
ArrayNEList:
    LEFT_SQUARE_BRACKET IntLit RIGHT_SQUARE_BRACKET ArrayNEList {
        $$ = $4;
        ParseNode_add_child($$, $2);
    }
|   LEFT_SQUARE_BRACKET IntLit RIGHT_SQUARE_BRACKET {
        $$ = construct_ParseNode(ArrayNEList);
        ParseNode_add_child($$, $2);
    }
;

PrimType:
    VOID {
        $$ = construct_ParseNode(PrimType, construct_ParseNode(Void));
    }
|   BOOL {
        $$ = construct_ParseNode(PrimType, construct_ParseNode(Bool));
    }
|   INTEGER {
        $$ = construct_ParseNode(PrimType, construct_ParseNode(Int));
    }
|   FLOAT {
        $$ = construct_ParseNode(PrimType, construct_ParseNode(Float));
    }
|   STRING {
        $$ = construct_ParseNode(PrimType, construct_ParseNode(String));
    }
;

ParamList:
    ParamNEList {
        $$ = $1;
    }
|   %empty {
        $$ = construct_ParseNode(ParamList);
    }
;
ParamNEList:
    ParamNEList COMMA SingleDecl {
        $$ = $1;
        ParseNode_add_child($$, $3);
    }
|   SingleDecl {
        $$ = construct_ParseNode(ParamList);
        ParseNode_add_child($$, $1);
    }
;

ConstOptName:
    OptName {
        $$ = construct_ParseNode(ConstOptName, construct_ParseNode(NotConst, $1));
    }
|   CONST OptName {
        $$ = construct_ParseNode(ConstOptName, construct_ParseNode(Const, $2));
    }
;

OptName:
    Name {
        $$ = construct_ParseNode(OptName, $1);
    }
|   %empty {
        $$ = construct_ParseNode(OptName);
    }
;

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// Statements ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
Stmt:
    DeclStmt {
        $$ = construct_ParseNode(Stmt, $1);
    }
|   PrintStmt {
        $$ = construct_ParseNode(Stmt, $1);
    }
|   ReadStmt {
        $$ = construct_ParseNode(Stmt, $1);
    }
|   AssignStmt {
        $$ = construct_ParseNode(Stmt, $1);
    }
|   CompoundStmt {
        $$ = construct_ParseNode(Stmt, $1);
    }
|   IfStmt {
        $$ = construct_ParseNode(Stmt, $1);
    }
|   WhileStmt {
        $$ = construct_ParseNode(Stmt, $1);
    }
|   DoWhileStmt {
        $$ = construct_ParseNode(Stmt, $1);
    }
|   ForStmt {
        $$ = construct_ParseNode(Stmt, $1);
    }
|   BreakStmt {
        $$ = construct_ParseNode(Stmt, $1);
    }
|   ContinueStmt {
        $$ = construct_ParseNode(Stmt, $1);
    }
|   CallStmt {
        $$ = construct_ParseNode(Stmt, $1);
    }
|   ReturnStmt {
        $$ = construct_ParseNode(Stmt, $1);
    }
;

DeclStmt:
    MultiDecl SEMICOLON {
        $$ = construct_ParseNode(DeclStmt, $1);
    }
;
PrintStmt:
    WRITE Expr SEMICOLON {
        $$ = construct_ParseNode(PrintStmt, $2);
    }
;
ReadStmt:
    READ LValExpr SEMICOLON {
        $$ = construct_ParseNode(ReadStmt, $2);
    }
;
AssignStmt:
    LValExpr ASSIGN_OP Expr SEMICOLON {
        $$ = construct_ParseNode(AssignStmt, $1, $3);
    }
|   LValExpr ASSIGN_OP FuncCall SEMICOLON {
        $$ = construct_ParseNode(AssignStmt, $1, construct_ParseNode(Expr, $3));
    }
;
CompoundStmt:
    LEFT_CURLY_BRACKET StmtList RIGHT_CURLY_BRACKET {
        $$ = construct_ParseNode(CompoundStmt, $2);
    }
;
IfStmt:
    IF LEFT_ROUND_BRACKET Expr RIGHT_ROUND_BRACKET Stmt %prec THEN {
        $$ = construct_ParseNode(IfStmt, $3, $5);
    }
|   IF LEFT_ROUND_BRACKET Expr RIGHT_ROUND_BRACKET Stmt ELSE Stmt {
        $$ = construct_ParseNode(IfElseStmt, $3, $5, $7);
    }
;
WhileStmt:
    WHILE LEFT_ROUND_BRACKET Expr RIGHT_ROUND_BRACKET Stmt {
        $$ = construct_ParseNode(WhileStmt, $3, $5);
    }
|   WHILE LEFT_ROUND_BRACKET Expr RIGHT_ROUND_BRACKET SEMICOLON {
        $$ = construct_ParseNode(WhileStmt, $3, NULL);
    }
;
DoWhileStmt:
    DO Stmt WHILE LEFT_ROUND_BRACKET Expr RIGHT_ROUND_BRACKET SEMICOLON {
        $$ = construct_ParseNode(DoWhileStmt, $2, $5);
    }
;
ForStmt:
    FOR LEFT_ROUND_BRACKET OptionalAssignStmtKern SEMICOLON OptionalExpr SEMICOLON OptionalAssignStmtKern RIGHT_ROUND_BRACKET Stmt {
        $$ = construct_ParseNode(ForStmt, $3, $5, $7, $9);
    }
|   FOR LEFT_ROUND_BRACKET OptionalAssignStmtKern SEMICOLON OptionalExpr SEMICOLON OptionalAssignStmtKern RIGHT_ROUND_BRACKET SEMICOLON {
        $$ = construct_ParseNode(ForStmt, $3, $5, $7, NULL);
    }
;
BreakStmt:
    BREAK SEMICOLON {
        $$ = construct_ParseNode(BreakStmt);
    }
;
ContinueStmt:
    CONTINUE SEMICOLON {
        $$ = construct_ParseNode(ContinueStmt);
    }
;
CallStmt:
    FuncCall SEMICOLON {
        $$ = construct_ParseNode(CallStmt, $1);
    }
;
ReturnStmt:
    RETURN Expr SEMICOLON {
        $$ = construct_ParseNode(ReturnStmt, $2);
    }
|   RETURN SEMICOLON {
        $$ = construct_ParseNode(ReturnStmt);
    }
;

StmtList:
    StmtList Stmt {
        $$ = $1;
        ParseNode_add_child($$, $2);
    }
|   %empty {
        $$ = construct_ParseNode(StmtList);
    }
;
OptionalExpr:
    Expr {
        $$ = $1;
    }
|   %empty {
        $$ = NULL;
    }
;
OptionalAssignStmtKern:
    LValExpr ASSIGN_OP Expr {
        $$ = construct_ParseNode(AssignStmt, $1, $3);
    }
|   LValExpr ASSIGN_OP FuncCall {
        $$ = construct_ParseNode(AssignStmt, $1, construct_ParseNode(Expr, $3));
    }
|   %empty {
        $$ = NULL;
    }
;

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Expressions ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
Expr:
    LValExpr {
        $$ = $1;
    }
|   RValExpr {
        $$ = $1;
    }
;
LValExpr: // L Value expressions
    Name {
        $$ = construct_ParseNode(Expr, $1);
    }
|   LEFT_ROUND_BRACKET LValExpr RIGHT_ROUND_BRACKET {
        $$ = $2;
    }
|   DerefExpr {
        $$ = construct_ParseNode(Expr, $1);
    }
|   ArrayExpr {
        $$ = construct_ParseNode(Expr, $1);
    }
;
DerefExpr:
    MULT Expr {
        $$ = construct_ParseNode(Expr_DEREF, $2);
    }
;
ArrayExpr:
    Name LEFT_SQUARE_BRACKET Expr RIGHT_SQUARE_BRACKET {
        $$ = construct_ParseNode(Expr_ARRAY, construct_ParseNode(Expr, $1), $3);
    }
|   LEFT_ROUND_BRACKET LValExpr RIGHT_ROUND_BRACKET LEFT_SQUARE_BRACKET Expr RIGHT_SQUARE_BRACKET {
        $$ = construct_ParseNode(Expr_ARRAY, $2, $5);
    }
|   LEFT_ROUND_BRACKET RValExpr RIGHT_ROUND_BRACKET LEFT_SQUARE_BRACKET Expr RIGHT_SQUARE_BRACKET {
        $$ = construct_ParseNode(Expr_ARRAY, $2, $5);
    }
|   ArrayExpr LEFT_SQUARE_BRACKET Expr RIGHT_SQUARE_BRACKET {
        $$ = construct_ParseNode(Expr_ARRAY, construct_ParseNode(Expr, $1), $3);
    }
;
RValExpr: // R Value expressions
    IntLit {
        $$ = construct_ParseNode(Expr, $1);
    }
|   FloatLit {
        $$ = construct_ParseNode(Expr, $1);
    }
|   StrLit {
        $$ = construct_ParseNode(Expr, $1);
    }
|   Expr QUESTION_MARK Expr COLON Expr {
        $$ = construct_ParseNode(Expr, construct_ParseNode(Expr_TERNARY, $1, $3, $5));
    }
|   Expr OR Expr {
        $$ = construct_ParseNode(Expr, construct_ParseNode(Expr_OR, $1, $3));
    }
|   Expr AND Expr {
        $$ = construct_ParseNode(Expr, construct_ParseNode(Expr_AND, $1, $3));
    }
|   NOT Expr {
        $$ = construct_ParseNode(Expr, construct_ParseNode(Expr_NOT, $2));
    }
|   Expr NOT_EQUAL Expr {
        $$ = construct_ParseNode(Expr, construct_ParseNode(Expr_NOT_EQUAL, $1, $3));
    }
|   Expr EQUAL Expr {
        $$ = construct_ParseNode(Expr, construct_ParseNode(Expr_EQUAL, $1, $3));
    }
|   Expr LESS_THAN Expr {
        $$ = construct_ParseNode(Expr, construct_ParseNode(Expr_LESS_THAN, $1, $3));
    }
|   Expr LESS_THAN_EQUAL Expr {
        $$ = construct_ParseNode(Expr, construct_ParseNode(Expr_LESS_THAN_EQUAL, $1, $3));
    }
|   Expr GREATER_THAN Expr {
        $$ = construct_ParseNode(Expr, construct_ParseNode(Expr_GREATER_THAN, $1, $3));
    }
|   Expr GREATER_THAN_EQUAL Expr {
        $$ = construct_ParseNode(Expr, construct_ParseNode(Expr_GREATER_THAN_EQUAL, $1, $3));
    }
|   Expr PLUS Expr {
        $$ = construct_ParseNode(Expr, construct_ParseNode(Expr_PLUS, $1, $3));
    }
|   Expr MINUS Expr {
        $$ = construct_ParseNode(Expr, construct_ParseNode(Expr_MINUS, $1, $3));
    }
|   Expr MULT Expr {
        $$ = construct_ParseNode(Expr, construct_ParseNode(Expr_MULT, $1, $3));
    }
|   Expr DIV Expr {
        $$ = construct_ParseNode(Expr, construct_ParseNode(Expr_DIV, $1, $3));
    }
|   MINUS Expr %prec UMINUS {
        $$ = construct_ParseNode(Expr, construct_ParseNode(Expr_UMINUS, $2));
    }
|   LEFT_ROUND_BRACKET RValExpr RIGHT_ROUND_BRACKET {
        $$ = $2;
    }
|   AMP LValExpr {
        $$ = construct_ParseNode(Expr, construct_ParseNode(Expr_ADDR, $2));
    }
// |   FuncCall {  // fuck sclp
//         $$ = construct_ParseNode(Expr, $1);
//     }
;
FuncCall:
    Name LEFT_ROUND_BRACKET ExprList RIGHT_ROUND_BRACKET {
        $$ = construct_ParseNode(Expr_FUNC_CALL, $1, $3);
    }
|   LEFT_ROUND_BRACKET LValExpr RIGHT_ROUND_BRACKET LEFT_ROUND_BRACKET ExprList RIGHT_ROUND_BRACKET {
        $$ = construct_ParseNode(Expr_FUNC_CALL, $2, $5);
    }
;

ExprList:
    %empty {
        $$ = construct_ParseNode(ExprList);
    }
|   ExprNEList {
        $$ = $1;
    }
;
ExprNEList:
    Expr {
        $$ = construct_ParseNode(ExprList, $1);
    }
|   ExprNEList COMMA Expr {
        $$ = $1;
        ParseNode_add_child($$, $3);
    }
;

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// Terminals /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
Name:
    NAME {
        $$ = construct_ParseNode_str(Name, $1);
        free($1);
    }
;
IntLit:
    INT_NUM {
        $$ = construct_ParseNode_int(IntLit, $1);
    }
;
FloatLit:
    FLOAT_NUM {
        $$ = construct_ParseNode_float(FloatLit, $1);
    }
;
StrLit:
    STR_CONST {
        $$ = construct_ParseNode_str(StrLit, $1);
        free($1);
    }
;

%%

void yyerror(struct ParseNode**, char const* msg)
{
    sclp_error(yylineno, msg);
}
