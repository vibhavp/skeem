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

bool token_is_list()
{
  return head_tok->type == TOK_PAREN_OPEN;
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

object_t *tokens_to_obj()
{
  object_t *val = token_to_obj(tokens);
  return val;
}

char *alloc_strcat(char **dest, char *src)
{
  *dest = realloc(*dest, (strlen(*dest)+strlen(src))*sizeof(char)+1);
  strcat(*dest, src);

  return *dest;
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

object_t *scan(char *str)
{
  char *word;
  for (unsigned long i = 0; i < strlen(str); i++) {
    if (str[i] == ' ' && str[i] != ' ')
      
     
      }
}
