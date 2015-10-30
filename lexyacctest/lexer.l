/*** Definition section ***/

%{
/* C code to be copied verbatim */
#include <stdio.h>

#define IMENICA     1001
#define GLAGOL      1002
#define PRIDJEV     1003
#define TOCKA       1004
%}

/* This tells flex to read only one input file */
/* %option noyywrap */

%%

djevojke	|
macke		{   printf("Token: %s, length: %d, id: %d\n", yytext, yyleng, IMENICA);
                //return IMENICA;
            }

gledaju		|
promatraju	{   printf("Token: %s, length: %d, id: %d\n", yytext, yyleng, GLAGOL);
                //return GLAGOL;
            }

zbunjene	|
uplasene	{   printf("Token: %s, length: %d, id: %d\n", yytext, yyleng, PRIDJEV);
			    //return PRIDJEV;
		    }

"."		    {   printf("Token: %s, length: %d, id: %d\n", yytext, yyleng, TOCKA);
                //return TOCKA;
            }

[\t\n\r ]   {}

.		    { printf("ERROR: %s\n", yytext); }

%%

int file_index = 1;
int gargc;
char **gargv;


int yywrap() {
    FILE *next_file;
    // move a file pointer to the next input file
    file_index++;
    // open the next input file if there is one
    if (file_index < gargc) {
        next_file = fopen(gargv[file_index], "r");
        if (!next_file) {
            printf("Cannot open input file: %s\n", gargv[file_index]);
            // if we don't have next file for processing, return 1 to notify end of processing
            return 1;
        }
    } 
    else {
        // if all input files have been processed, return 1 to notify end of processing
        return 1;
    }
    // if we have next file for processing, go ahead by returning 0
    yyin = next_file;
    return 0;
}


int main(int argc, char *argv[]) {
    FILE *init_file;
    // save command line aruments to global variables to be accessible from yywrap
    gargc = argc;
    gargv = malloc(sizeof(char*) * argc);
    for (int i=0; i<argc; i++) {
        gargv[i] = argv[i];
    }
    // open the first input file
    if (argc > 1) {
        init_file = fopen(argv[file_index], "r");
        if (!init_file) {
            printf("Cannot open input file: %s\n", argv[file_index]);
            return -1;
        }
    }
    else {
        printf("No input file specified.\nUsage: execprog INPUT_FILE_1 [INPUT_FILE_2 ... INPUT_FILE_N]\n");
        return -1;
    }

    yyin = init_file;

    yylex();
    return 0;
}

