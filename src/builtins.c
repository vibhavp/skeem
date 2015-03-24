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
    result->integer = n1->integer + n2->flt;
    result->type = INTEGER;
  }
  else if (n1->type == INTEGER || n2->type == FLOAT) {
    result->flt = n1->integer + n2->flt;
    result->type = FLOAT;
  }
  else if (n1->type == FLOAT || n2->type == INTEGER) {
    result->flt = n1->flt + n2->integer;
    result->type = FLOAT;
  }
  else {
    result->flt = n1->flt + n2->flt;
    result->type = FLOAT;
  }
  return result;
}

object_t *subtract(object_t *n1, object_t *n2)
{
  if (n2->type == FLOAT)
    n2->flt = -n2->flt;
  else if (n2->type == INTEGER)
    n2->integer = -n2->integer;
  
  object_t *result = add(n1, n2);

  if (n2->type == FLOAT)
    n2->flt = -n2->flt;
  else if (n2->type == INTEGER)
    n2->integer = -n2->integer;
  
  return result;
}

/* If cond yield non-nil, return consequent, else alternate*/
object_t *ifelse(object_t *cond, object_t *consequent, object_t *alternate)
{
  if (cond->type == BOOLEAN && !cond->boolean) {
    return eval(alternate);
  }
  
  return eval(consequent);
}

    
/* Call function using args as a list of arguments.
 * A function call in Scheme/Lisp is a list with the car
 * being the lambda/function symbol (an object_t type with cell/string set), and
 * the cdr being the arguments. The cdr is seperated into a different list, and
 * than passed to apply:
 * ((lambda (a b c) (+ 1 2 3)) 1 2 3)
 *  |_______________________|  |___|
 *             car              cdr
 *
 * Which is internally represented as:
 * ((lambda.((a . (b . (c . NULL)). ((ADD . (a . (b . (c . NULL)))) . NULL)))).(1 . (2 . (3 . NULL))))
 *  |         |____________________||_____________________________________| |  |                    |
 *  |              parameters                       body                    |  |                    |
 *  |_______________________________________________________________________|  |____________________|
 *                               function                                               args
 */
 object_t *apply(object_t *function, struct cons *args)
{
  object_t *obj;
  if (function->type == BUILTIN) {
    switch (function->builtin) {
      case AND:
        return and(eval(args->car), eval(args->cdr->car));
      case CAR:
        return eval(args->car);
      case CDR:
        obj = obj_init();
        obj->type = LIST;
        obj->cell = args->cdr;
        return obj;
      case CONS:
        return cons(args->car, args->cdr->car);
      case DEFINE:
        return define(args->car, args->cdr->car);
      case IF:
        return ifelse(eval(args->car),
                      args->cdr->car,
                      args->cdr->cdr->car);
      case LAMBDA:
        /*TODO*/
        return args;
      case NOT:
        return not(eval(args->car));
      case OR:
        return or(eval(args->car), eval(args->cdr->car));
      case PRINT:
        return print(eval(args->car));
      case QUOTE:
        return quote(args->car);
      case OPERATOR:

        switch (function->operator) {
          case ADD:
            return add(eval(args->car), eval(args->cdr->car));
          case SUBTRACT:
            return subtract(eval(args->car), eval(args->cdr->car));
          case DIVIDE:
            return divide(eval(args->car), eval(args->cdr->car));
          case MULTIPLY:
            return multiply(eval(args->car), eval(args->car));
        }
    }
  }
  if (function->type == SYMBOL) {
    function = sym_find(function->string);
    if (function == NULL) {
      return NULL;
    }
  }
  if (function->type == LIST && function->cell->car->builtin == LAMBDA) {

    /*Parameters in the function's "signature"*/
    struct cons *parameters = function->cell->cdr->car->cell;
    /*Actual parameters passed to the function*/
    struct cons *args_head = args;
    /*The function's body */
    struct cons *body = function->cell->cdr->cdr;
    
    if (_length(parameters) != _length(args)) {
      fprintf(stderr, "%s: wrong number of arguments.", repr(function));
      return NULL;
    }
    
    depth_inc();
    while (parameters != NULL) {
      sym_insert(parameters->car->string, args_head->car);
      args_head = args_head->cdr;
      parameters = parameters->cdr;
    }
    if (body->cdr != NULL)
      (void)eval(body->car);

    /*TODO: Check this thing, looks buggy*/
    /*Reached end of body, return this value*/
    return eval(body->cdr->car);
    depth_dec();
  }
  
  fprintf(stderr, "Invalid Function: %s.", repr(function));
  return NULL;
}

/* Evaluate object */
object_t *eval(object_t *obj)
{
  object_t *val;
  
  if (!obj->quoted) {
    switch (obj->type)
    {
      case LIST:
        /* Doing a function call, the first car of the cell should either be a
         * procedure, or a symbol pointing to a procedure. */
        if (obj->cell->car->type != PROCEDURE ||
            obj->cell->car->type != SYMBOL) {
          fprintf(stderr, "Invalid Function");
          /* Return to top level, haven't figured out how to do that yet.
           */
          return NULL;
        }
        depth_inc();
        val = 
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
  /*Quoted object*/
  return obj;
}

void init_globals()
{
  char *builtin_func[] = {"and", "car", "cdr", "cons", "define", "eval",
                          "if", "lambda", "not", "or", "print", "quote"};
  object_t *obj;
  builtin_t i;
  for (i = AND; i <= QUOTE; i++) {
    obj  = obj_init();
    obj->type = BUILTIN;
    obj->builtin = i;
    sym_insert(builtin_func[i], obj);
  }
}
