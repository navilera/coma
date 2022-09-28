%{

#include "y.tab.h"
#include "parser.h"

int yywrap(void)
{
	return 1;
}

%}

REG_WORD	[a-zA-Z가-힣,\-\<\>\(\)\[\]0-9\|\!\'\"\%\#_\*&\/]

%%

^"%%%\n"		return CODE_QUOTE;
^"```\n"		return MD_CODE_QUOTE;
" "				return SPACE;
"\n"			return NEW_LINE;

^"#"+" ".+\n {
	yylval.str = yytext;
	return TITLE;
}

^"*"+" ".+\n {
	yylval.str = yytext;
	return BULLET_LIST;
}

^"> ".+\n {
	yylval.str = yytext;
	return QUOTE_BLOCK;
}

^[a-zA-Z][a-zA-Z]"|"\t.*\n {
	yylval.str = yytext;
	return LABELED_CODE_LINE;
}

^\t.*\n {
	yylval.str = yytext;
	return CODE_LINE;
}

^"code{"[a-zA-Z][a-zA-Z][a-zA-Z]"}"\n {
	yylval.str = yytext;
	return CODE_NUM;
}

^"figure{"[0-9a-zA-Z][0-9a-zA-Z][0-9a-zA-Z]"}"\n {
	yylval.str = yytext;
	return FIGURE_NUM;
}

^"![".*"](".+")"\n {
	yylval.str = yytext;
	return FIGURE;
}

"line{".."}" {
	uint32_t line_num = get_val_as_2key(yytext[5], yytext[6]);
	sprintf(yytext, "%02d", line_num);						// NN
	yytext[2] = 0xEB; yytext[3] = 0xB2; yytext[4] = 0x88;	// 번
	yytext[5] = 0xEC; yytext[6] = 0xA7; yytext[7] = 0xB8;	// 째
	yylval.str = yytext;
	return REF_LINE;
}

"cd{"..."}" {
	uint32_t code_num = get_val_as_3key(yytext[3], yytext[4], yytext[5]);
	sprintf(yytext, "[%02d-%02d]", (code_num / 100), (code_num % 100));
	yylval.str = yytext;
	return REF_CODE;
}

"fg{"..."}" {
	uint32_t fg_num = get_val_as_3key(yytext[3], yytext[4], yytext[5]);
	sprintf(yytext, "[%02d-%02d]", (fg_num / 100), (fg_num % 100));
	yylval.str = yytext;
	return REF_FIGURE;
}

{REG_WORD}+" " {
	yylval.str = yytext;
	return WORD;
}

{REG_WORD}+[\.\?] {
	yylval.str = yytext;
	return WORD_DOT;
}

%%
