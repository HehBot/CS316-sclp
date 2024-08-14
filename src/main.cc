#include <ast.h>
#include <opt.h>
#include <parse.h>
#include <sym.h>
#include <rtl.h>
#include <tac.h>

#include <algorithm>
#include <cstdarg>
#include <cstring>
#include <memory>
#include <iostream>

static Options options;

std::string func_under_processing_name;

extern "C" {
    int yylex();
    int yylex_destroy();
    int yyparse(struct ParseNode**);
    void token_output(char const*, char const*, int);
    void init_instrument();
    void sclp_error(size_t line, char const* format, ...)
    {
        fprintf(stderr, "sclp error:");
        if (line > 0)
            fprintf(stderr, " %s:%lu", options.input_filename.c_str(), line);
        fprintf(stderr, "\n");
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        fprintf(stderr, "\n");
        exit(1);
    }
    void register_strlit(char const* s)
    {
        extern RTL::Context ctx;
        ctx.get_string_id(std::string(s));
    }
}
std::string aux_error_msg;
void sclp_error(size_t line, std::string s)
{
    std::cerr << "sclp error:";
    if (line > 0)
        std::cerr << " " << options.input_filename << ":" << line;
    std::cerr << '\n' << s;
    if (aux_error_msg.length() > 0)
        std::cerr << ": " << aux_error_msg;
    std::cerr << '\n';
    exit(1);
}

void token_output(char const* token_name, char const* lexeme, int lineno)
{
    (*options.token_output) << "\tToken Name: " << token_name << " \tLexeme: " << lexeme << " \t Lineno: " << lineno << "\n";
}

void print_string_escapes(std::string s, std::ostream& o)
{
    for (char c : s) {
        switch (c) {
        case '\n':
            o << "\\n"; break;
        case '\r':
            o << "\\r"; break;
        case '\t':
            o << "\\t"; break;
        case '\a':
            o << "\\a"; break;
        case '"':
            o << "\\\""; break;
        case '\\':
            o << "\\\\"; break;
        default:
            o << c;
        }
    }
}

int main(int argc, char** argv)
{
    init_instrument();

    options = Options(argc, argv);
    {
        extern FILE* yyin;
        yyin = options.input;
    }

    if (options.stage == Stage::TOKEN)
        while (yylex());
    else {
        ParseNode* parse_tree = nullptr;
        if (options.stage >= Stage::PARSE)
            yyparse(&parse_tree);

        std::vector<AST::FuncDefn> ast;
        SymbolTable symtab;
        if (options.stage >= Stage::AST)
            ast = AST::from_parse_tree(parse_tree, symtab);

        if (parse_tree != nullptr)
            delete parse_tree;

        if (options.stage >= Stage::TAC)
            for (auto& a : ast)
                a.make_tac();

        if (options.stage >= Stage::RTL)
            for (auto& a : ast) {
                RTL::reset();
                for(auto t : a.tac)
                    t->gen_rtl(a.rtl);
            }

        if (options.stage >= Stage::ASM)
            for (auto& a : ast) {
                func_under_processing_name = a.func->name;
                for(auto r : a.rtl)
                    r->gen_asm(a.mips_asm);
            }


        if (options.stage >= Stage::AST)
            for (auto const& a : ast)
                a.print(*options.ast_output);
        if (options.stage >= Stage::TAC) {
            for (auto const& a : ast)
                if (a.tac.size() > 0) {
                    (*options.tac_output) << "**PROCEDURE: " << a.func->name << "\n";
                    (*options.tac_output) << "**BEGIN: Three Address Code Statements\n";
                    for (auto z : a.tac)
                        z->print(*options.tac_output);
                    (*options.tac_output) << "**END: Three Address Code Statements\n";
                }
        }
        if (options.stage >= Stage::RTL) {
            for (auto const& a : ast)
                if (a.rtl.size() > 0) {
                    (*options.rtl_output) << "**PROCEDURE: " << a.func->name << "\n";
                    (*options.rtl_output) << "**BEGIN: RTL Statements\n";
                    for(auto const& r : a.rtl)
                        r->print(*options.rtl_output);
                    (*options.rtl_output) << "**END: RTL Statements\n";
                }
        }
        if (options.stage >= Stage::ASM) {
            extern RTL::Context ctx;
            std::vector<std::shared_ptr<Symbol>> const& gv = symtab.get_global_vars();

            if (ctx.string_store.size() > 0 || gv.size() > 0) {
                (*options.asm_output) << "\n\t.data\n";
                for (auto s : gv)
                    (*options.asm_output) << s->name << ":\t" << (s->semtype->to_tactype() == TAC::Type::FLOAT ? ".double 0.0" : ".word 0") << '\n';
                for (size_t i = 0; i < ctx.string_store.size(); ++i) {
                    (*options.asm_output) << "_str_" << i << ": .asciiz \"";
                    print_string_escapes(ctx.string_store[i], *options.asm_output);
                    (*options.asm_output) << "\"\n";
                }
            }

            for(auto const& a : ast) {
                    func_under_processing_name = a.func->name;

                    (*options.asm_output) << "\t.text\n";
                    (*options.asm_output) << "\t.globl " << a.func->name << "\n";
                    (*options.asm_output) << a.func->name << ":\n";
                    (*options.asm_output) << "\tsw $ra, 0($sp)\n";
                    (*options.asm_output) << "\tsw $fp, -4($sp)\n";
                    (*options.asm_output) << "\tsub $fp, $sp, 4\n";

                    size_t sps = a.stackframe_size + 4;

                    (*options.asm_output) << "\tsub $sp, $sp, " << sps << "\n";

                    for(auto const& as : a.mips_asm)
                        as->print(*options.asm_output);

                    (*options.asm_output) << "epilogue_" << a.func->name << ":\n";
                    (*options.asm_output) << "\tadd $sp, $sp, " << sps << "\n";
                    (*options.asm_output) << "\tlw $fp, -4($sp)\n";
                    (*options.asm_output) << "\tlw $ra, 0($sp)\n";
                    (*options.asm_output) << "\tjr $ra\n";
            }
        }
    }

    yylex_destroy();

    return 0;
}
