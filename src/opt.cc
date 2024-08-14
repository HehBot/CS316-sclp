#include <argp.h>
#include <cstdio>
#include <error.h>
#include <opt.h>
#include <iomanip>
#include <iostream>
#include <fstream>

/*
      --sa-scan              Stop after scanning
      --sa-parse             Stop after parsing
      --sa-ast               Stop after constructing Abstract Syntax Tree
                             (AST)
      --sa-tac               Stop after constructing Three Address Code (TAC)
      --sa-rtl               Stop after constructing Register Transfer Language
                             (RTL) code
      --show-tokens          Show the tokens in FILE.toks (or out.toks)
      --show-ast             Show abstract syntax trees in FILE.ast (or
                             out.ast)
      --show-tac             Show the Three Address Code in FILE.tac (or
                             out.tac)
      --show-rtl             Show the Register Transfer Language code in
                             FILE.rtl (or out.rtl)
      --show-symtab          Show the symbol table after RTL construction (when
                             offsets are allocated) in FILE.sym, (or out.sym)
      --show-asm             Generate the assembly program in FILE.spim (or
                             out.spim). This is the default action and is
                             suppressed only if a valid `sa-...' option is
                             given to stop the compilation after some earlier
                             phase.
      --show-json-ast        Show the Abstract Syntax Tree in JSON format
      --show-json-tac        Show the Three Address Code in JSON format
      --show-json-rtl        Show the Register Transfer Language code in JSON
                             format
      --read-json-ast        Use the input file (in JSON format) to generate
                             the AST and skip all the phases from scanning to
                             parsing
      --read-json-tac        Use the input file (in JSON format) to generate
                             the TAC and skip all the phases from scanning to
                             TAC generation
      --read-json-rtl        Use the input file (in JSON format) to generate
                             the RTL code and skip all the phases from scanning
                             to RTL generation
  -d, --demo                 Demo version. Use stdout for the output instead of
                             files
      --gen-temp-symb-table  Populate Symbol Table For Temporaries
  -e, --single-stmt-bb       Flag to construct single statement basic blocks
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version
*/
static char const* doc = "Sclp - A language processor for C-like language";
char const* argp_program_version = "Sclp Version: A1";
static char const* args_doc = "FILE";
static struct argp_option options[] = {
    { "sa-scan", 1, NULL, 0, "Stop after scanning" },
    { "sa-parse", 2, NULL, 0, "Stop after parsing" },
    { "sa-ast", 3, NULL, 0, "Stop after constructing Abstract Syntax Tree (AST)" },
    { "sa-tac", 4, NULL, 0, "Stop after constructing Three Address Code (TAC)" },
    { "sa-rtl", 5, NULL, 0, "Stop after constructing Register Transfer Language (RTL) code" },
    { "show-tokens", 6, NULL, 0, "Show the tokens in FILE.toks (or out.toks)" },
    { "show-ast", 7, NULL, 0, "Show abstract syntax trees in FILE.ast (or out.ast)" },
    { "show-tac", 8, NULL, 0, "Show the Three Address Code in FILE.tac (or out.tac)" },
    { "show-rtl", 9, NULL, 0, "Show the Register Transfer Language code in FILE.rtl (or out.rtl)" },
    { "show-symtab", 10, NULL, 0, "Show the symbol table after RTL construction (when offsets are allocated) in FILE.sym, (or out.sym)" },
    { "show-asm", 11, NULL, 0, "Generate the assembly program in FILE.spim (or out.spim). This is the default action and is suppressed only if a valid `sa-...' option is given to stop the compilation after some earlier phase." },
    { "show-json-ast", 12, NULL, 0, "Show the Abstract Syntax Tree in JSON format" },
    { "show-json-tac", 13, NULL, 0, "Show the Three Address Code in JSON format" },
    { "show-json-rtl", 14, NULL, 0, "Show the Register Transfer Language code in JSON format" },
    { "read-json-ast", 15, NULL, 0, "Use the input file (in JSON format) to generate the AST and skip all the phases from scanning to parsing" },
    { "read-json-tac", 16, NULL, 0, "Use the input file (in JSON format) to generate the TAC and skip all the phases from scanning to TAC generation" },
    { "read-json-rtl", 17, NULL, 0, "Use the input file (in JSON format) to generate the RTL code and skip all the phases from scanning to RTL generation" },
    { "demo", 'd', NULL, 0, "Demo version. Use stdout for the output instead of files" },
    { "gen-temp-symb-table", 18, NULL, 0, "Populate Symbol Table For Temporaries" },
    { "single-stmt-bb", 'e', NULL, 0, "Flag to construct single statement basic blocks" },
    { 0 }
};

struct Args {
    std::string input_filename;
    Stage stage = Stage::ASM;
    bool show_tokens = false, show_ast = false, show_tac = false, show_rtl = false, show_asm = true;
    bool demo = false;
};

static error_t parse_opt(int key, char* arg, struct argp_state* state)
{
    Args* args = static_cast<Args*>(state->input);
    switch (key) {
        case 1:
            args->stage = Stage::TOKEN;
            break;
        case 2:
            if (args->stage > Stage::PARSE)
                args->stage = Stage::PARSE;
            break;
        case 3:
            if (args->stage > Stage::AST)
                args->stage = Stage::AST;
            break;
        case 4:
            if (args->stage > Stage::TAC)
                args->stage = Stage::TAC;
            break;
        case 5:
            if (args->stage > Stage::RTL)
                args->stage = Stage::RTL;
            break;
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
            sclp_error(0, "The JSON related options are not supported.");
        case 10:
        case 18:
            sclp_error(0, "The symtab related options are not supported.");
        case 6:
            args->show_tokens = true;
            break;
        case 7:
            args->show_ast = true;
            break;
        case 8:
            args->show_tac = true;
            break;
        case 9:
            args->show_rtl = true;
            break;
        case 11:
            args->show_asm = true;
            break;
        case 'd':
            args->demo = true;
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 2)
                argp_usage(state);
            args->input_filename = std::string(arg);
            break;
        case ARGP_KEY_END:
            if (state->arg_num < 1)
                argp_usage(state);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

// see https://stackoverflow.com/a/11826666/18230146
class NullBuffer : public std::streambuf {
    static NullBuffer singleton;

public:
    static NullBuffer* get()
    {
        return &singleton;
    }
    int overflow(int c) override
    {
        return c;
    }
};
NullBuffer NullBuffer::singleton;

Options::Options(int argc, char** argv)
{
    Args args;
    struct argp a = { options, parse_opt, args_doc, doc };
    argp_parse(&a, argc, argv, 0, 0, &args);

    input_filename = args.input_filename;
    stage = args.stage;
    input = fopen(args.input_filename.c_str(), "r");

    if (input == NULL)
        sclp_error(0, std::string("Unable to open file ") + input_filename);

    if (args.show_tokens && stage >= Stage::TOKEN) {
        if (args.demo)
            token_output = &std::cout;
        else
            token_output = new std::ofstream((args.input_filename + ".toks").c_str());
    } else
        token_output = new std::ostream(NullBuffer::get());

    if (args.show_ast && stage >= Stage::AST) {
        if (args.demo)
            ast_output = &std::cout;
        else
            ast_output = new std::ofstream((args.input_filename + ".ast").c_str());
    } else
        ast_output = new std::ostream(NullBuffer::get());

    if (args.show_tac && stage >= Stage::TAC) {
        if (args.demo)
            tac_output = &std::cout;
        else
            tac_output = new std::ofstream((args.input_filename + ".tac").c_str());
    } else
        tac_output = new std::ostream(NullBuffer::get());

    if (args.show_rtl && stage >= Stage::RTL) {
        if (args.demo)
            rtl_output = &std::cout;
        else
            rtl_output = new std::ofstream((args.input_filename + ".rtl").c_str());
    } else
        rtl_output = new std::ostream(NullBuffer::get());

    if (args.show_asm && stage == Stage::ASM) {
        if (args.demo)
            asm_output = &std::cout;
        else
            asm_output = new std::ofstream((args.input_filename + ".spim").c_str());
    } else
        asm_output = new std::ostream(NullBuffer::get());

    (*ast_output) << std::fixed << std::showpoint << std::setprecision(2);
    (*tac_output) << std::fixed << std::showpoint << std::setprecision(2);
    (*rtl_output) << std::fixed << std::showpoint << std::setprecision(2);
    (*asm_output) << std::fixed << std::showpoint << std::setprecision(2);
}
