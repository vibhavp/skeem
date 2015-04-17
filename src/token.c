/* Copyright © 2015 Vibhav Pant <vibhavp@gmail.com>

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "token.h"
#include "types.h"
#include "mem.h"
#include "builtins.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static char *builtin_syms[22] =  {"and", "car", "cdr", "cond", "cons", "define",
                                  "eval", "lambda", "not", "or", "print", "quote",
                                  "integer_p", "float_p", "number_p", "string_p",
                                  "symbol_p", "list_p", "lambda_p", "boolean_p",
                                  "eqv_p", "equal_p"};
enum tok_type {
  TOK_INT,
  TOK_FLOAT,
  /* TOK_CHAR, */
  TOK_STRING,
  TOK_SYMBOL,
  TOK_OPERATOR_PLUS,
  TOK_OPERATOR_MINUS,
  TOK_OPERATOR_DIVIDE,
  TOK_OPERATOR_MULTIPLY,
  TOK_PAREN_OPEN,
  TOK_PAREN_CLOSE,
  TOK_INCOMPLETE,
  TOK_ERROR
};

typedef struct _token {
  enum tok_type type;
  union {
    char *string;
    char character;
    int64_t integer;
    double flt;
  };

  struct _token *next;
} token_t;

static token_t *tokens, *head_tok;

object_t *token_to_obj(token_t *tok);

void add_token(enum tok_type type, char *str)
{
  if (tokens == NULL) {
    tokens = malloc(sizeof(token_t));
    head_tok = tokens;
  }
  else {
    head_tok->next = malloc(sizeof(token_t));
    head_tok = head_tok->next;
  }

  if (head_tok == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  head_tok->type = type;

  switch (type) {
    case TOK_INT:
      head_tok->integer = atoi(str);
      break;
    case TOK_FLOAT:
      head_tok->flt = atof(str);
      break;
    /* case TOK_CHAR: */
    /*   head_tok->character = str[0]; */
    /*   break; */
    case TOK_STRING:
    case TOK_SYMBOL:
      head_tok->string = str;
      break;
    default:
      break;
  }
}

cons_t *token_to_cons(token_t *tok)
{
  cons_t *cell;
  object_t *val = token_to_obj(tok->next);

  if (val == NULL)
    return NULL;

  cell = cons_init();
  cell->car = val;
  cell->cdr = token_to_cons(tok->next);

  return cell;
}

object_t *token_to_obj(token_t *tok)
{
  object_t *obj;
  no_gc = false;

  switch (tok->type) {
    case TOK_INT:
      obj = obj_init(INTEGER);
      obj->integer = tok->integer;
      return obj;
    case TOK_FLOAT:
      obj = obj_init(FLOAT);
      obj->flt = tok->flt;
      return obj;
    case TOK_STRING:
      obj = obj_init(STRING);
      obj->string = tok->string;
      return obj;
    case TOK_SYMBOL:
      /*Check if symbol is a builtin*/
      for (int i = 0; i < 22; i++) {
        if (strcmp(tok->string, builtin_syms[i]) == 0)
          return builtins[i];
      }

      obj = obj_init(SYMBOL);
      obj->string = tok->string;
      return obj;
    case TOK_PAREN_OPEN:
      if (tok->next->type == TOK_PAREN_CLOSE)
        return EMPTY_LIST;
      depth++;
      obj = obj_init(LIST);
      obj->cell = token_to_cons(tok);
      return obj;
    case TOK_PAREN_CLOSE:
      depth--;
      return NULL;
    case TOK_OPERATOR_PLUS:
      return builtins[OPERATOR(ADD)];
    case TOK_OPERATOR_MINUS:
      return builtins[OPERATOR(SUBTRACT)];
    case TOK_OPERATOR_MULTIPLY:
      return builtins[OPERATOR(DIVIDE)];
    case TOK_OPERATOR_DIVIDE:
      return builtins[OPERATOR(DIVIDE)];
  }
}

enum tok_type type(char *word, unsigned long start)
{
  enum tok_type sub;

  switch(word[start]) {
    case '+':
      if (word[start+1] != '\0') {
        sub = type(word, start+1);
        return sub == TOK_FLOAT || sub == TOK_INT ? sub : TOK_SYMBOL;
      }
      return TOK_OPERATOR_PLUS;
    case '-':
      if (word[start+1] != '\0') {
        sub = type(word, start+1);  
          return sub == TOK_FLOAT || sub == TOK_INT ? sub : TOK_SYMBOL;
      }
      return TOK_OPERATOR_MINUS;
    case '*':
      if (word[start+1] != '\0')
        return TOK_SYMBOL;
      return TOK_OPERATOR_MULTIPLY;
    case'/':
      if (word[start+1] != '\0')
        return TOK_SYMBOL;
      return TOK_OPERATOR_DIVIDE;
    case '0'...'9':
      for (unsigned long i = start; i < strlen(word); i++) {

        if (!(word[i] >= '0' && word[i] <= '9')) {
          if (word[i] == '.' && type(word, i+1) == TOK_INT)
            return TOK_FLOAT;
          return TOK_SYMBOL;
        }
      }
    case '\"':
        if (word[strlen(word)-1] == '"')
          return TOK_STRING;
        return TOK_INCOMPLETE;
    case '(':
      return TOK_PAREN_OPEN;
    case ')':
      return TOK_PAREN_CLOSE;
    default:
      return TOK_SYMBOL;
  }
}

token_t *str_to_tok(char *word)
{
  token_t *tok = malloc(sizeof(token_t));
  tok->type = type(word, 0);

  switch(tok->type)
  {
    case TOK_INT:
      tok->integer = atoi(word);
      return tok;
    case TOK_FLOAT:
      tok->flt = atof(word);
      return tok;
    case TOK_STRING:
    case TOK_SYMBOL:
      tok->string = word;
    case TOK_PAREN_OPEN:
    case TOK_PAREN_CLOSE:
      return tok;      
  }
  return NULL;
}

token_t **scan(char *str)
{
  char word[100];
  unsigned long index = 0, tok_size = 0, cur_tok = 0;

  for (unsigned long i = 0; i < strlen(str); i++) {
    if (str[i] == ' ' && str[i] != ' ') {
      tokens = realloc(tokens, sizeof(token_t *)*(++tok_size));
      if (tokens == NULL) {
        perror("realloc");
        exit(EXIT_FAILURE);
      }
      
    }
  }
}
