%{

#include <stdio.h>
#include "y.tab.h"

extern YYSTYPE yylval;
char *get_class_desc(int);

%}

/* This tells flex to read only one input file */
/* %option noyywrap */

%%

girls   |
cats		{   printf("Token: %s, length: %d, class: %s\n", yytext, yyleng, get_class_desc(NOUN));
                yylval.str = strdup(yytext);
                return NOUN;
            }

watch	|
observe	    {   printf("Token: %s, length: %d, class: %s\n", yytext, yyleng, get_class_desc(VERB));
                yylval.str = strdup(yytext);
                return VERB;
            }

afraid  |
confused    {   printf("Token: %s, length: %d, class: %s\n", yytext, yyleng, get_class_desc(ADJECTIVE));
                yylval.str = strdup(yytext);
			    return ADJECTIVE;
		    }

"." |
"?" |
"!"		    {   printf("Token: %s, length: %d, class: %s\n", yytext, yyleng, get_class_desc(PUNCTUATION));
                yylval.str = strdup(yytext);
                return PUNCTUATION;
            }

[\t\n\r ]   {}

.		    { printf("ERROR - unrecognized character(s): %s, length: %d\n", yytext, yyleng); }

%%

char *get_class_desc(int token) {
    switch(token) {
        case NOUN:
            return "NOUN";
        case VERB:
            return "VERB";
        case ADJECTIVE:
            return "ADJECTIVE";
        case PUNCTUATION:
            return "PUNCTUATION";
        default:
            return "N/A";
    }
}
