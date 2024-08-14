%{
#include <error.h>
#include <stdio.h>
#include <string.h>
#include <parser.y.tab.h>

char* process_escapes(char const* s)
{
    char* a = malloc(strlen(s) + 1);
    int si = 0, ai = 0;
    si++; // skip initial "
    while (s[si] != '"') {
        char c;
        if (s[si] == '\\') {
            si++;
            switch (s[si]) {
                case 'n':
                    c = '\n'; break;
                case 'r':
                    c = '\r'; break;
                case 't':
                    c = '\t'; break;
                case 'a':
                    c = '\a'; break;
                default:
                    c = s[si];
            }
        } else
            c = s[si];
        a[ai] = c;
        si++, ai++;
    }
    a[ai] = '\0';
    return a;
}

void register_strlit(char const* s);
void token_output(char const* token_name, char const* lexeme, int lineno);
#define OUTPUT(token)\
    do {\
        token_output(#token, yytext, yylineno);\
        return token;\
    } while (0)
%}

%%
\/\/[^\n]*\n    yylineno++;
0 {
    yylval.intval = 0;
    OUTPUT(INT_NUM);
}
(0?|[1-9][0-9]*)\.[0-9]* {
    yylval.floatval = strtod(yytext, NULL);
    OUTPUT(FLOAT_NUM);
}
[1-9][0-9]* {
    yylval.intval = strtouq(yytext, NULL, 10);
    OUTPUT(INT_NUM);
}
\"([^\"]|\\.)*\" {
    yylval.strval = process_escapes(yytext);
    register_strlit(yylval.strval);
    OUTPUT(STR_CONST);
}
if          OUTPUT(IF);
else        OUTPUT(ELSE);
do          OUTPUT(DO);
while       OUTPUT(WHILE);
for         OUTPUT(FOR);
break       OUTPUT(BREAK);
continue    OUTPUT(CONTINUE);
print       OUTPUT(WRITE);
return      OUTPUT(RETURN);
read        OUTPUT(READ);
const       OUTPUT(CONST);
void        OUTPUT(VOID);
bool        OUTPUT(BOOL);
int         OUTPUT(INTEGER);
float       OUTPUT(FLOAT);
string      OUTPUT(STRING);
[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.strval = strdup(yytext);
    OUTPUT(NAME);
}
\?          OUTPUT(QUESTION_MARK);
:           OUTPUT(COLON);
\(          OUTPUT(LEFT_ROUND_BRACKET);
\)          OUTPUT(RIGHT_ROUND_BRACKET);
\{          OUTPUT(LEFT_CURLY_BRACKET);
\}          OUTPUT(RIGHT_CURLY_BRACKET);
\[          OUTPUT(LEFT_SQUARE_BRACKET);
\]          OUTPUT(RIGHT_SQUARE_BRACKET);
!=          OUTPUT(NOT_EQUAL);
==          OUTPUT(EQUAL);
\<=         OUTPUT(LESS_THAN_EQUAL);
\<          OUTPUT(LESS_THAN);
>=          OUTPUT(GREATER_THAN_EQUAL);
>           OUTPUT(GREATER_THAN);
\|\|        OUTPUT(OR);
&&          OUTPUT(AND);
&           OUTPUT(AMP);
!           OUTPUT(NOT);
-           OUTPUT(MINUS);
\+          OUTPUT(PLUS);
\*          OUTPUT(MULT);
\/          OUTPUT(DIV);
=           OUTPUT(ASSIGN_OP);
;           OUTPUT(SEMICOLON);
,           OUTPUT(COMMA);
[ \t\r]+
\n {
    yylineno++;
}
.           sclp_error(yylineno, "Unrecognised character '%c'", *yytext);
