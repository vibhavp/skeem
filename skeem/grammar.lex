/* Copyright © 2015 Vibhav Pant <vibhavp@gmail.com>

 *Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 *associated documentation files (the “Software”), to deal in the Software without restriction,
 *including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 *subject to the following conditions:

 *The above copyright notice and this permission notice shall be included in all copies or substantial
 *portions of the Software.

 *THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 *LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *LIABILITY,WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

%{
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include "types.h"
#include "grammar.h"

  static char **tokens = NULL, *types = NULL;
  static int depth = 0, size = 0;
  void token_add(char *str, char type)
  {
    /* if (size == 0) { */
    /*   tokens = malloc(sizeof(char *) * 10); */
    /*   types = malloc(sizeof(char) * 10); */
    /*   tokens[0] = str; */
    /*   types[0] = type; */
    /*   size++; */
    /* } */
    /* else { */
    /*   if (index > size) { */
    /*     tokens = realloc(tokens, sizeof(tokens) + sizeof(char *)); */
    /*     types = realloc(types, sizeof(types) + sizeof(char)); */
    /*     tokens[index] = */
    /*   } */
    /* } */
    char **new_tokens, *new_types;
    if ((new_tokens  = realloc(tokens, sizeof(tokens)*(size) + sizeof(char)*strlen(str))) == NULL) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }
    printf("Allocated token #%d\n", size);
    tokens = new_tokens;
    if ((new_types = realloc(types, sizeof(types)*(size) + sizeof(char))) == NULL) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }
    types = new_types;
    tokens[size] = str;
    types[size] = type;
    size++;
  }
  
%}

%%
[ \t] ;
-*[0-9]+           {token_add(strdup(yytext), NUMBER);}
-*[0-9]+\.[0-9]+   {token_add(strdup(yytext), NUMBER_FLT);}
\"([^\\\"]|\\.)*\" {token_add(strdup(yytext), STRING);};
[a-zA-Z0-9]+       {token_add(strdup(yytext), SYMBOL);}
\(                 {token_add(strdup("("), LPAREN); depth++;}
\)                 {token_add(strdup(")"), RPAREN);depth--; unsigned int index = 1; if (depth == 0)repr(cons_to_object(tok_to_cons(tokens, types, &index)));}
%%

main(int argc, char *argv[]) {
  yylex();
}
