/*** Definition section ***/

%{
/* C code to be copied verbatim */
#include <stdio.h>

#define IMENICA 1
#define GLAGOL  2
#define PRIDJEV 3
#define TOCKA   4
%}

/* This tells flex to read only one input file */
%option noyywrap

%%

zbunjene	|
uplasene	{	printf("Token: %s, id: %d\n", yytext, PRIDJEV);
			    //return PRIDJEV;
		    }

djevojke	|
macke		{   printf("Token: %s, id: %d\n", yytext, IMENICA);
                //return IMENICA;
            }

gledaju		|
promatraju	{   printf("Token: %s, id: %d\n", yytext, GLAGOL);
                //return GLAGOL;
            }

"."		    {   printf("Token: %s, id: %d\n", yytext, TOCKA);
                //return TOCKA;
            }

[\t\n\r ]   {}

.		    { printf("ERROR: %s\n", yytext); }

%%

int main(int argc, char *argv[])
{
    FILE *myfile = fopen(argv[1], "r");
    if (!myfile) {
        printf("Cannot open input file: %s\n", argv[1]);
        return -1;
    }

    // set lex to read from it instead of defaulting to STDIN:
    yyin = myfile;

    yylex();
    return 0;
}

