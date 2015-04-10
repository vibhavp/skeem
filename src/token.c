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
  
  if (head_tok->next == NULL) {
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

inline bool token_is_list()
{
  return head_tok->type == TOK_PAREN_OPEN;
}

cons_t *token_to_cons(token_t *tok)
{
  cons_t *cell = cons_init(), *cur_cell;
  token_t *cur_tok = tok->next;
  object_t *obj = token_to_obj(cur_tok);
  cell->car = obj;
  cur_cell = cell->cdr;

  /*Empty lists are handled by token_to_obj, this function will never get one.*/
  while (true) {
    obj = token_to_obj(cur_tok);
    if (obj == NULL)
      return cell;
    cur_cell = cons_init();
    cur_cell->car = obj;
  }
}

object_t *token_to_obj(token_t *tok)
{
  object_t *obj;
  no_gc = false;
  
  switch (tok->type) {
    case TOK_INT:
      obj = obj_init(INTEGER);
      obj->integer = head_tok->integer;
      break;
    case TOK_FLOAT:
      obj = obj_init(FLOAT);
      obj->flt = head_tok->flt;
      break;
    case TOK_STRING:
      obj = obj_init(STRING);
      obj->string = head_tok->string;
    case TOK_SYMBOL:
      obj = obj_init(SYMBOL);
      obj->string = head_tok->string;
    case TOK_PAREN_OPEN:
      if (tok->next->type == TOK_PAREN_CLOSE)
        return EMPTY_LIST;
      
      obj = obj_init(LIST);
      obj->cell = token_to_cons(tok);
    default: /*TOK_PAREN_CLOSE*/
      return NULL;
  }
}
