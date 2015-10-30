%{

#include <stdio.h>
#include "lex.yy.h"

%}

%union
{
    int num;
    char *str;
}

%token  NOUN
%token  VERB
%token  ADJECTIVE
%token  PUNCTUATION

%%

text:
        sentence  {   printf("Reduction: text <- sentence\n"); }
        | text sentence {   printf("Reduction: text <- text sentence\n"); }
        ;
sentence:
        subject_clause predicate object_clause PUNCTUATION  {   printf("Reduction: sentence <- subject_clause predicate object_clause %s\n", yylval.str); }
        ;

subject_clause:
        attribute subject   {   printf("Reduction: subject_clause <- attribute subject\n"); }
        ;

object_clause:
        attribute object    {   printf("Reduction: object_clause <- attribute object\n"); }
        ;

subject:
        NOUN    {   printf("Reduction: subject <- %s\n", yylval.str); }
        ;
object:
        NOUN    {   printf("Reduction: object <- %s\n", yylval.str); }
        ;
predicate:
        VERB    {   printf("Reduction: predicate <- %s\n", yylval.str); }
        ;
attribute:
        ADJECTIVE   {   printf("Reduction: attribute <- %s\n", yylval.str); }
        ;
%%

void yyerror(char *s) {
    fprintf(stderr, "%s\n", s);
}


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

    yyparse();
    return 0;
}
