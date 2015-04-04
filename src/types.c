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

#include "types.h"
#include "grammar.h"
#include "env.h"
#include "mem.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

         
static char *strtype(type_t type)
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

int check_arg_type(object_t *obj, int n, ...)
{
  va_list args;
  type_t type;
  int i;
  
  va_start(args, n);
  for (i = 0; i < n; i++) {
    type = va_arg(args, type_t);
    if (obj->type != type) {
      fprintf(stderr, "Wrong argument type - %s (wanted %s).",
              strtype(obj->type),
              strtype(type));
      va_end(args);
      return 1;
    }
  }
  va_end(args);
  return 0;
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

bool obj_eq(object_t *ob1, object_t *ob2)
{
  if (ob1->type == ob2->type) {
    cons_t *curr1, *curr2;
    switch(ob1->type) {
      case INTEGER:
        return ob1->integer == ob2->integer;
      case FLOAT:
        return ob1->flt == ob2->flt;
      case STRING:
        return strcmp(ob1->string, ob2->string) == 0;
      case CHAR:
        return ob1->character == ob2->character;
      case SYMBOL:
        return obj_eq(sym_find(ob1->string),
                      sym_find(ob2->string));
      case LIST:
        if (length(ob1->cell) != length(ob2->cell))
          return false;
        curr1 = ob1->cell;
        curr2 = ob2->cell;
        
        while (curr1 != NULL)
        {
          if (!obj_eq(curr1->car, curr2->car))
            return false;
          curr1 = curr1->cdr;
          curr2 = curr2->cdr;
        }
        return true;
      case BOOLEAN:
        return ob1->boolean == ob2->boolean;
      case OPERATOR:
        return ob1->operator == ob2->operator;
      case BUILTIN:
        return ob1->builtin == ob2->builtin;
      default: /*PREDICATE*/
        return ob1->predicate == ob2->predicate;
    }
  }
  return false;
}

/* Convert the array of tokens to a cons cell, start evaluating at index.
 * index is updated to the index of the token at the end of the sexp.
 */
cons_t *tok_to_cons(char **tokens, char *types, int *index)
{
  cons_t *cell = cons_init(), *head = cell;

  while(types[*index] != RPAREN) {
    object_t *obj = obj_init();
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

/* Convert a cons cell to an object */
object_t *cons_to_object(cons_t *cell)
{
  object_t *obj = malloc(sizeof(object_t));
  obj->type = LIST;
  obj->cell = cell;
  return obj;
}
