%{
#include <stdio.h>

#include "parser.h"

extern int yylex(void);
extern int yylineno;

int yyerror(const char* str)
{
     extern char* yytext;
	fprintf(stderr, "[Parse error] %s [%d] : %s\n", str, yylineno, yytext);
	return 0;
}

%}

%union {
	char* str;
}

%start doc

%token<str>			TITLE
%token<str>			CODE_LINE
%token<str>         LABELED_CODE_LINE
%token<str>         CODE_NUM
%token<str>			FIGURE_NUM
%token<str>         QUOTE_BLOCK
%token<str>         BULLET_LIST
%token<str>			FIGURE
%token<str>         WORD
%token<str>         WORD_DOT
%token<str>         REF_LINE
%token<str>         REF_CODE
%token<str>         REF_FIGURE
%token<str>         NEW_LINE

%token              SPACE
%token				CODE_QUOTE
%token				MD_CODE_QUOTE

%type<str>          sentence_list
%type<str>          sentence

%%

doc  : REF_LINE
     | TITLE					     { write_header($1); }
     | CODE_LINE 		     	     { write_code_line($1); }
     | CODE_NUM                      { write_code_num($1); }
     | FIGURE_NUM                    { write_figure_num($1); }
     | LABELED_CODE_LINE			 { write_replace_code_line($1); }
	 | MD_CODE_QUOTE      		     { write_quote(); }
     | CODE_QUOTE                    { write_start_code_line(); }
     | QUOTE_BLOCK                   { write_plain($1); }
     | BULLET_LIST                   { write_plain($1); }
     | FIGURE		                 { write_plain($1); write_plain("\n");}
     | sentence_list			     { write_text($1); }
     ;

sentence_list :                      { $$ = NULL; }
              | sentence NEW_LINE                
              | sentence SPACE sentence_list
              ;

sentence : WORD_DOT
         | REF_LINE SPACE sentence
         | REF_CODE sentence
         | REF_FIGURE sentence
         | WORD sentence
         ;
%%
