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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct cons *cons_init()
{
  struct cons *cell = NULL;
  if ((cell = malloc(sizeof(struct cons *))) == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  return cell;
}

void cons_free(struct cons *cell)
{
  obj_free(cell->car);

  if (cell->cdr != NULL)
    cons_free(cell->cdr);
}

object_t *obj_init()
{
  object_t *obj = NULL;
  if ((obj = malloc(sizeof(object_t *))) == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  return obj;
}

void obj_free(object_t *obj)
{
  switch(obj->type) {
    case LIST:
      cons_free(obj->cell);
      break;

    case SYMBOL:
    case STRING:
      free(obj->string);
    default:
      break;
  }
  free(obj);
}
         
/* Convert the array of tokens to a cons cell, start evaluating at index.
 * index is updated to the index of the token at the end of the sexp.
 */
struct cons *tok_to_cons(char **tokens, char *types, int *index)
{
  struct cons *cell = cons_init(), *head = cell;

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
object_t *cons_to_object(struct cons *cell)
{
  object_t *obj = malloc(sizeof(object_t *));
  obj->type = LIST;
  obj->cell = cell;
  return obj;
}
