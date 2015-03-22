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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "env.h"
#include "builtins.h"

static inline void check_number(object_t *obj)
{
  if (obj->type != INTEGER || obj->type != FLOAT) {
    /* fprintf(stderr, "Expected int/float, got %s", strtype(obj->type));   */
  }
}

object_t *add(object_t *n1, object_t *n2)
{
  /* There has to be a better way to do this */
  check_number(n1);
  check_number(n2);
  object_t *result = obj_init();

  if (n1->type == INTEGER || n2->type == INTEGER) {
    result->val = malloc(sizeof(int *));
    cast_int(result->val) = cast_int(n1->val) + cast_int(n2->val);
    result->type = INTEGER;
  }
  else if (n1->type == INTEGER || n2->type == FLOAT) {
    result->val = malloc(sizeof(float *));
    cast_float(result->val) = cast_int(n1->val) + cast_float(n2->val);
    result->type = FLOAT;
  }
  else if (n1->type == FLOAT || n2->type == INTEGER) {
    result->val = malloc(sizeof(float *));
    cast_float(result->val) = cast_float(n1->val) + cast_int(n2->val);
    result->type = FLOAT;
  }
  else {
    result->val = malloc(sizeof(float *));
    cast_float(result->val) = cast_float(n1->val) + cast_float(n2->val);
    result->type = FLOAT;
  }
  return result;
}

object_t *subtract(object_t *n1, object_t *n2)
{
  cast_int(n2->val) = -1 * cast_int(n2->val);
  object_t *result = add(n1, n2);
  cast_int(n2->val) = -1 * cast_int(n2->val);

  return result;
}

object_t *quote(object_t *object) 
{
  object_t *quoted = obj_init();
  quoted->type = QUOTED;
  quoted->val = object;

  return quoted;
}

/* If cond yield non-nil, return consequent, else alternate*/
object_t *ifelse(object_t *cond, object_t *consequent, object_t *alternate)
{
  if (cond->type == SYMBOL && strcmp(cond->val, "nil") == 0) {
    return eval(alternate);
  }
  
  return eval(consequent);
}

    
/* Call function using args as a list of arguments*/
object_t *apply(object_t *function, struct cons *args)
{
  if (function->type == SYMBOL) {
    /* Check for builtins */
    char *sym = (char *)function->val;
    if (strcmp(sym, "+"))
      return add(eval(args->car), eval(args->cdr->car));
    
    else if (strcmp(sym, "-"))
      return subtract(eval(args->car), eval(args->cdr->car));
    
    else if (strcmp(sym, "if"))
      /* Cannot eval consequents, might have side effects */
      return ifelse(eval(args->car), args->car, args->cdr->car);

    else if (strcmp(sym, "car"))
      return args->car;

    else if (strcmp(sym, "cdr")) {
      object_t *cdr = obj_init();
      cdr->type = LIST;
      cdr->val = args->cdr;
      return cdr;
    }

    /* else if (strcmp(sym, "define")) */
    /*   return define() */

    else {
      object_t *l = sym_find(sym);
      if (l == NULL)
        return NULL;
      return apply(l, args);
    }
  }
  /* The car of function->val stores the argument list,
   * while the cdr stores the function body.
   */
  if (check_args_n(function, args))
    return NULL;
  
  /*(lambda (a b c) (body))*/
  struct cons *body = cast_cons(function->val)->cdr->cdr;
  
  if (body->cdr != NULL)
    (void)eval(body->car);
  
  return eval(body->car);
}


/* Evaluate object */
object_t *eval(object_t *obj)
{
  object_t *val;
  
  switch (obj->type) {
    case LIST:
      /* Doing a function call, the first car of the cell should either be a
       * procedure, or a symbol pointing to a procedure. */
      if (cast_cons(obj->val)->car->type != PROCEDURE ||
          cast_cons(obj->val)->car->type != SYMBOL) {
        fprintf(stderr, "Invalid Function");
        /* Return to top level, haven't figured out how to do that yet.
         */
        return NULL;
      }
      depth_inc();
      
      val = apply(cast_cons(obj->val)->car, cast_cons(obj->val)->cdr);
      depth_dec();
      return val;
      
    case SYMBOL:
      if ((val = sym_find((char *)obj->val)) == NULL) {
        return NULL;
      }
      return eval(val);

    default:
      return obj;
  }
}
