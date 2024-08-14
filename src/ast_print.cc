#include <string>
#include <ast.h>
#include <iostream>
#include <iomanip>

using namespace AST;

void FuncDefn::print(std::ostream& o) const
{
    o << "**PROCEDURE: " << func->name << "\n";
    o << "	Return Type: <";
    func->semtype->get_ret_type()->print(o);
    o << ">\n";
    o << "	Formal Parameters:\n";
    for (std::shared_ptr<Symbol> param : params) {
        o << "		" << param->name << "  Type:<";
        param->semtype->print(o);
        o << ">\n";
    }
    o << "**BEGIN: Abstract Syntax Tree ";
    body->print(o, "         ");
    o << "\n";
    o << "**END: Abstract Syntax Tree \n";
}

void AssignStmt::print(std::ostream& o, std::string indent) const
{
    o << "\n";
    o << indent << "Asgn:" << "\n";
    o << indent << "  LHS (";
    lhs->print(o, indent + "    ");
    o << ")\n";
    o << indent << "  RHS (";
    rhs->print(o, indent + "    ");
    o << ")";
}

void IntLit::print(std::ostream& o, std::string) const
{
    o << "Num : " << val << "<int>";
}

void FloatLit::print(std::ostream& o, std::string) const
{
    o << "Num : " << val << "<float>";
}

void print_string_escapes(std::string, std::ostream&);
void StrLit::print(std::ostream& o, std::string) const
{
    o << "String : \"";
    print_string_escapes(val, o);
    o << "\"<string>";
}

void Sym::print(std::ostream& o, std::string) const
{
    o << "Name : " << sym->name << "<";
    semtype->print(o);
    o << ">";
}

void ReadStmt::print(std::ostream& o, std::string indent) const
{
    o << "\n";
    o << indent << "Read: ";
    arg->print(o, indent + "  ");
}

void PrintStmt::print(std::ostream& o, std::string indent) const
{
    o << "\n";
    o << indent << "Write: ";
    arg->print(o, indent + "  ");
}

void CompoundStmt::print(std::ostream& o, std::string indent) const
{
    for (auto child : stmt_list)
        child->print(o, indent);
}

void IfStmt::print(std::ostream& o, std::string indent) const
{
    o << "\n";
    o << indent << "If: \n";
    o << indent + "  " << "Condition (";
    cond->print(o, indent + "    ");
    o << ")\n";
    o << indent + "  " << "Then (";
    body->print(o, indent + "    ");
    o << ")";
}

void IfElseStmt::print(std::ostream& o, std::string indent) const
{
    o << "\n";
    o << indent << "If: \n";
    o << indent + "  " << "Condition (";
    cond->print(o, indent + "    ");
    o << ")\n";
    o << indent + "  " << "Then (";
    body->print(o, indent + "    ");
    o << ")";

    o << "\n";
    o << indent + "  " << "Else (";
    else_body->print(o, indent + "    ");
    o << ")";
}

void WhileStmt::print(std::ostream& o, std::string indent) const
{
    o << "\n";
    o << indent << "While: \n";
    o << indent + "  " << "Condition (";
    cond->print(o, indent + "    ");
    if (body != nullptr) {
        o << ")\n";
        o << indent + "  " << "Body (";
        body->print(o, indent + "    ");
    }
    o << ")";
}

void DoWhileStmt::print(std::ostream& o, std::string indent) const
{
    o << "\n";
    o << indent << "Do:\n";
    o << indent + "  " << "Body (";
    body->print(o, indent + "    ");
    o << ")\n";
    o << indent + "  " << "While Condition (";
    cond->print(o, indent + "    ");
    o << ")";
}

void ForStmt::print(std::ostream& o, std::string indent) const
{
    o << "\n";
    o << indent << "For:";
    if (pre_stmt != nullptr) {
        o << "\n";
        o << indent + "  " << "Pre (";
        pre_stmt->print(o, indent + "  ");
        o << ")";
    }
    if (cond != nullptr) {
        o << "\n";
        o << indent + "  " << "Condition (";
        cond->print(o, indent + "  ");
        o << ")";
    }
    if (inc_stmt != nullptr) {
        o << "\n";
        o << indent + "  " << "Inc (";
        inc_stmt->print(o, indent + "  ");
        o << ")";
    }
    if (body != nullptr) {
        o << "\n";
        o << indent + "  " << "Body (";
        body->print(o, indent + "  ");
        o << ")";
    }
}

void BreakStmt::print(std::ostream& o, std::string indent) const
{
    o << "\n";
    o << indent << "break";
}

void ContinueStmt::print(std::ostream& o, std::string indent) const
{
    o << "\n";
    o << indent << "continue";
}
void CallStmt::print(std::ostream& o, std::string indent) const
{
    fc->print(o, indent);
}
void ReturnStmt::print(std::ostream& o, std::string indent) const
{
    o << "\n";
    o << indent << "Return";
    if (ret != nullptr) {
        o << ": ";
        ret->print(o, indent + "  ");
    }
}

void AddExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n" << indent << "Arith: Plus<";
    semtype->print(o);

    o << ">\n" << indent << "  L_Opd (";
    lhs->print(o, indent + "    ");
    o << ")\n";

    o << indent << "  R_Opd (";
    rhs->print(o, indent + "    ");
    o << ")";
}

void MulExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n" << indent << "Arith: Mult<";
    semtype->print(o);
    o << ">\n" << indent << "  L_Opd (";
    lhs->print(o, indent + "    ");
    o << ")\n";

    o << indent << "  R_Opd (";
    rhs->print(o, indent + "    ");
    o << ")";
}

void DivExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n" << indent << "Arith: Div<";
    semtype->print(o);
    o << ">\n" << indent << "  L_Opd (";
    lhs->print(o, indent + "    ");
    o << ")\n";

    o << indent << "  R_Opd (";
    rhs->print(o, indent + "    ");
    o << ")";
}

void SubExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n" << indent << "Arith: Minus<";
        semtype->print(o);
    o << ">\n" << indent << "  L_Opd (";
        lhs->print(o, indent + "    ");
    o << ")\n";

    o << indent << "  R_Opd (";
    rhs->print(o, indent + "    ");
    o << ")";
}

void NegExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n" << indent << "Arith: Uminus<";
    semtype->print(o);
    o << ">\n" << indent << "  L_Opd (";
    lhs->print(o, indent + "    ");
    o << ")";
}

void OrExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n" << indent << "Condition: OR<";
    semtype->print(o);
    o << ">\n" << indent << "  L_Opd (";
    lhs->print(o, indent + "    ");
    o << ")\n";

    o << indent << "  R_Opd (";
    rhs->print(o, indent + "    ");
    o << ")";
}

void AndExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n" << indent << "Condition: AND<";
    semtype->print(o);
    o << ">\n" << indent << "  L_Opd (";
    lhs->print(o, indent + "    ");
    o << ")\n";

    o << indent << "  R_Opd (";
    rhs->print(o, indent + "    ");
    o << ")";
}

void NotEqualExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n" << indent << "Condition: NE<";
    semtype->print(o);
    o << ">\n" << indent << "  L_Opd (";
    lhs->print(o, indent + "    ");
    o << ")\n";

    o << indent << "  R_Opd (";
    rhs->print(o, indent + "    ");
    o << ")";
}

void EqualExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n" << indent << "Condition: EQ<";
    semtype->print(o);
    o << ">\n" << indent << "  L_Opd (";
    lhs->print(o, indent + "    ");
    o << ")\n";

    o << indent << "  R_Opd (";
    rhs->print(o, indent + "    ");
    o << ")";
}

void GreaterExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n" << indent << "Condition: GT<";
    semtype->print(o);
    o << ">\n" << indent << "  L_Opd (";
    lhs->print(o, indent + "    ");
    o << ")\n";

    o << indent << "  R_Opd (";
    rhs->print(o, indent + "    ");
    o << ")";
}

void GreaterEqualExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n" << indent << "Condition: GE<";
    semtype->print(o);
    o << ">\n" << indent << "  L_Opd (";
    lhs->print(o, indent + "    ");
    o << ")\n";

    o << indent << "  R_Opd (";
    rhs->print(o, indent + "    ");
    o << ")";
}

void LessExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n" << indent << "Condition: LT<";
    semtype->print(o);
    o << ">\n" << indent << "  L_Opd (";
    lhs->print(o, indent + "    ");
    o << ")\n";

    o << indent << "  R_Opd (";
    rhs->print(o, indent + "    ");
    o << ")";
}

void LessEqualExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n" << indent << "Condition: LE<";
    semtype->print(o);
    o << ">\n" << indent << "  L_Opd (";
    lhs->print(o, indent + "    ");
    o << ")\n";

    o << indent << "  R_Opd (";
    rhs->print(o, indent + "    ");
    o << ")";
}

void ArrayExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n" << indent << "ArrayAccess<";
    semtype->print(o);
    o << ">\n" << indent << "  Array (";
    base->print(o, indent + "    ");
    o << ")\n";

    o << indent << "  Index (";
    index->print(o, indent + "    ");
    o << ")";
}

void NotExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n" << indent << "Condition: NOT<";
    semtype->print(o);
    o << ">\n" << indent << "  L_Opd (";
    lhs->print(o, indent + "    ");
    o << ")";
}

void DerefExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n" << indent << "Dereference<";
    semtype->print(o);
    o << ">\n" << indent << "  L_Opd (";
    lhs->print(o, indent + "    ");
    o << ")";
}

void AddrExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n" << indent << "Addr<";
    semtype->print(o);
    o << ">\n" << indent << "  L_Opd (";
    lhs->print(o, indent + "    ");
    o << ")";
}

void TernaryExpr::print(std::ostream& o, std::string indent) const
{
    cond->print(o, indent + "    ");
    o << "\n" << indent << "    True_Part (";
    true_part->print(o, indent + "      ");
    o << ")\n" << indent << "    False_Part (";
    false_part->print(o, indent + "      ");
    o << ")";
}

void FuncCallExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n";
    o << indent << "FN CALL: " << func->name << "(";
    for (auto p : params) {
        o << "\n" << indent;
        p->print(o, indent + "  ");
    }
    o << ")";
}
void FuncPtrCallExpr::print(std::ostream& o, std::string indent) const
{
    o << "\n";
    o << indent << "Indirect FN CALL: ";
    func_ptr->print(o, indent);
    o << "\n" << indent << "(";
    for (auto p : params) {
        o << "\n" << indent;
        p->print(o, indent);
    }
    o << ")";
}
