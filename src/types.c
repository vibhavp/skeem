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

#include "builtins.h"
#include "mem.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <math.h>
#include "types.h"
#include "token.h"

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

void print_obj(object_t *obj, FILE *stream)
{
  switch(obj->type)
  {
    case INTEGER:
      fprintf(stream, "%ld", obj->integer);
      break;
    case FLOAT:
      fprintf(stream, "%f", obj->flt);
      break;
    case CHAR:
      fprintf(stream, "%c", obj->character);
      break;
    case STRING:
      fprintf(stream, "\"%s\"", obj->string);
      break;
    case SYMBOL:
      fprintf(stream, "%s", obj->string);
      break;
    case BOOLEAN:
      fprintf(stream, obj->boolean ? "true" : "false");
      break;
    case LIST:
      {
        fprintf(stream, "(");
        cons_t *cur = obj->cell;

        while (cur->cdr != NULL) {
          print_obj(cur->car, stream);
          fputs(" ", stream);
          cur = cur->cdr;
        }
        print_obj(cur->car, stream);
        fprintf(stream, ")");
      }
      break;
    case BUILTIN:
      fprintf(stream, "%s", builtin_syms[obj->builtin]);
      break;
    case PREDICATE:
      fprintf(stream, "%s", builtin_syms[PREDICATE(obj->predicate)]);
      break;
    case OPERATOR:
      fprintf(stream, "%s", builtin_syms[OPERATOR(obj->operator)]);
      break;
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
