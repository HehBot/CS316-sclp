#ifndef OPT_H
#define OPT_H

#include <string>
#include <cstdio>
#include <iostream>

enum class Stage {
    TOKEN, PARSE, AST, TAC, RTL, ASM
};
struct Options {
    FILE* input;
    std::string input_filename;

    Stage stage;
    std::ostream* token_output;
    std::ostream* ast_output;
    std::ostream* tac_output;
    std::ostream* rtl_output;
    std::ostream* asm_output;

    Options()
        : input(NULL), input_filename(""), stage(Stage::AST), token_output(nullptr), ast_output(nullptr), tac_output(nullptr), rtl_output(nullptr), asm_output(nullptr)
    {
    }
    Options(int argc, char** argv);

    Options(Options const&) = delete;
    Options(Options&& o)
        : input(o.input), input_filename(o.input_filename), stage(o.stage), token_output(o.token_output), ast_output(o.ast_output), tac_output(o.tac_output), rtl_output(o.rtl_output), asm_output(o.asm_output)
    {
        o.input = NULL;
        o.token_output = o.ast_output = o.tac_output = o.rtl_output = o.asm_output = nullptr;
    }
    Options& operator=(Options const&) = delete;
    Options& operator=(Options&& o)
    {
        this->~Options();
        input = o.input;
        input_filename = o.input_filename;
        stage = o.stage;
        token_output = o.token_output;
        ast_output = o.ast_output;
        tac_output = o.tac_output;
        rtl_output = o.rtl_output;
        asm_output = o.asm_output;

        o.input = NULL;
        o.token_output = o.ast_output = o.tac_output = o.rtl_output = o.asm_output = nullptr;
        return *this;
    }
    ~Options()
    {
        if (input != NULL) {
            fclose(input);
            input = NULL;
        }
        if (token_output != nullptr && token_output != &std::cout) {
            delete token_output;
            token_output = nullptr;
        }
        if (ast_output != nullptr && ast_output != &std::cout) {
            delete ast_output;
            ast_output = nullptr;
        }
        if (tac_output != nullptr && tac_output != &std::cout) {
            delete tac_output;
            tac_output = nullptr;
        }
        if (rtl_output != nullptr && rtl_output != &std::cout) {
            delete rtl_output;
            rtl_output = nullptr;
        }
        if (asm_output != nullptr && asm_output != &std::cout) {
            delete asm_output;
            asm_output = nullptr;
        }
    }
};

#endif // OPT_H
