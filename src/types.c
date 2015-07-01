/* Copyright (c) 2015 Vibhav Pant <vibhavp@gmail.com>

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

int length(cons_t *list) {
  if (list == NULL) return 0;

  int len = 0;
  cons_t *head = list;

  while (head != NULL) {
    len++;
    head = head->cdr;
  }
  return len;
}


void print_obj(object_t *obj, FILE *stream) {
  switch (obj->type) {
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
      fprintf(stream, obj->boolean ? "#t" : "#f");
      break;
    case LIST: {
      if (obj == EMPTY_LIST) {
        fprintf(stream, "()");
        return;
      }

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
    case PRIMITIVE:
      fprintf(stream, "<builtin procedure>");
      break;
    case PROCEDURE:
      fprintf(stream, "<procedure %s>", obj->procedure->name);
  }
}
