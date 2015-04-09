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

#include "grammar.h"
#include "builtins.h"
#include "env.h"
#include "mem.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include "types.h"

char *strtype(type_t type)
{
  switch(type)
  {
    case INTEGER:
      return "integer";
    case FLOAT:
      return "float";
    case CHAR:
      return "char";
    case STRING:
      return "string";
    case SYMBOL:
      return "symbol";
    case LIST:
      return "list";
    case BOOLEAN:
      return "boolean";
    default: /*builtin*/
      return "procedure";
  }
}

int length(cons_t *list)
{
  int len = 0;
  cons_t *head = list;

  while (head != 0)
  {
    len++;
    head = head->cdr;
  }
  return len;
}

char *strop(operator_t op)
{
  switch(op)
  {
    case ADD:
      return "+";
    case SUBTRACT:
      return "-";
    case DIVIDE:
      return "/";
    default: /*Multiply*/
      return "*";
  }
}

char *repr(object_t *obj)
{
  return NULL;
}

char *strpred(predicate_t pred)
{
  switch(pred)
  {
    case INTEGER_P:
      return "integer?";
    case FLOAT_P:
      return "float?";
    case NUMBER_P:
      return "number?";
    case STRING_P:
      return "string?";
    case SYMBOL_P:
      return "symbol?";
    case LIST_P:
      return "list?";
    default: /*LAMBDA_P*/
      return "lambda?";
  }
}

/* Convert the array of tokens to a cons cell, start evaluating at index.
 * index is updated to the index of the token at the end of the sexp.
 */
cons_t *tok_to_cons(char **tokens, char *types, int *index)
{
  cons_t *cell = cons_init(), *head = cell;

  while(types[*index] != RPAREN) {
    object_t *obj = obj_init(0);
    switch(types[*index]) {
      case INTEGER:
        obj->integer = atoi(tokens[*index]);
        break;
      case FLOAT:
        obj->flt = atof(tokens[*index]);
        break;
      case SYMBOL:
        obj->string = strdup(tokens[*index]);
        break;
      case LPAREN:
        /* Entering a nested list */
        /* For refernce, (a . (b . ((c . (d . nil)) . nil))) = (a b (c d))
         * docs/nested describes how we represent nested list.
         */
        obj->type = LIST;
        *index += 1;
        obj->cell = tok_to_cons(tokens, types, index);
        break;
      case STRING:
        obj->string = strdup(tokens[*index]);
        break;
    }
    obj->type = types[*index];
    head->car = obj;
    if (types[*index+1] != RPAREN) {
      head->cdr = cons_init();
      head = head->cdr;
    }
    else {
      head->cdr = NULL;
    }
    *index = *index + 1;
  }
  /* Reached last element of current cons cell */
  return cell;
}
