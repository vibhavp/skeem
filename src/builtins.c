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
#include "mem.h"
#include "env.h"
#include "builtins.h"

#define _INTEGER_P(n)  ((n)->type == INTEGER)
#define _FLOAT_P(n)    ((n)->type == FLOAT)
#define _NUMBER_P(n)   (_INTEGER_P((n)) || _FLOAT_P((n)))
#define _STRING_P(n)   ((n)->type == STRING)
#define _SYMBOL_P(n)   ((n)->type == SYMBOL)
#define _LIST_P(n)     ((n)->type == LIST)
#define _LAMBDA_P(n)   (_LIST_P((n))                            \
                        && (n)->cell->car->type == BUILTIN      \
                        && (n)->cell->car->builtin == LAMBDA)
#define PRED_BOOL_OBJ(predicate) ((predicate) ? CONST_TRUE : CONST_FALSE)

object_t *add(object_t *n1, object_t *n2)
{
  if (n1 == NULL || n2 == NULL)
    return NULL;
 
  if (check_arg_type(n1, 2, INTEGER, FLOAT) &&
      check_arg_type(n2, 2, INTEGER, FLOAT))
    return NULL;
  
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
  if (n1 == NULL || n2 == NULL)
      return NULL;
    
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

/*(cond                                     _  
  (                                          |
  ((cond1) (body)) <- clauses->car->cell     |
  ((cond2) (body)) <- clauses->cdr->car->cell| <- clauses
  )                                          |
  )                                         -
 */
object_t *cond(cons_t *clauses)
{
  if (clauses == NULL)
    return NULL;
  
  cons_t *curr_cell = clauses, *clause;
  object_t *val;
  int clause_no = 1;
  
  do {
    clause = curr_cell->car->cell;
    val = eval(clause->car);
    /*Evaluate body if clause condition doesnt evaluate to f*/
    if (val->type == BOOLEAN && !val->boolean) {
      if (clause->cdr == NULL) {
        fprintf(stderr, "No consequent for clause %d", clause_no);
        return NULL;
      }
      return eval(clause->cdr->car);
    }
    curr_cell = curr_cell->cdr;
  }while(curr_cell != NULL);
  /*None of conditions are true*/
  fprintf(stderr, "None of the conditions in cond expression are true.");
  return NULL;
}

object_t *cdr(object_t *cell)
{
  if (cell == NULL)
    return NULL;
  
  object_t *obj = obj_init();
   obj->cell = cell->cell->cdr;
  return obj;
}

inline object_t *car(cons_t *cell)
{
  if (cell == NULL)
    return NULL;
  return cell->car;
}

static object_t *call_predicate(object_t *obj, predicate_t pred)
{
  if (obj == NULL)
    return NULL;
  
  switch(pred)
  {
    case INTEGER_P:
      return PRED_BOOL_OBJ(_INTEGER_P(obj));
    case FLOAT_P:
      return PRED_BOOL_OBJ(_FLOAT_P(obj));
    case NUMBER_P:
      return PRED_BOOL_OBJ(_NUMBER_P(obj));
    case STRING_P:
      return PRED_BOOL_OBJ(_STRING_P(obj));
    case SYMBOL_P:
      return PRED_BOOL_OBJ(_SYMBOL_P(obj));
    case LIST_P:
      return PRED_BOOL_OBJ(_LIST_P(obj));
    default: /*LAMBDA_P*/
      return PRED_BOOL_OBJ(_LAMBDA_P(obj));
  }
}

static object_t *call_operator(operator_t op, cons_t *args)
{
  if (args == NULL)
    return NULL;
  
  switch (op) {
    case ADD:
      return add(eval(args->car), eval(args->cdr->car));
    case SUBTRACT:
      return subtract(eval(args->car), eval(args->cdr->car));
    case DIVIDE:
      return divide(eval(args->car), eval(args->cdr->car));
    default: /*MULTIPLY*/
      return multiply(eval(args->car), eval(args->car));
  }
}


/*true if the length of args == params_no. Else, print an error message and
 * return false*/
static bool correct_number_args(char *function, int params_no,
                                cons_t *args)
{
  int len = length(args);
  if (len != params_no) {
    fprintf(stderr,
            "Wrong number of arguments to %s - %d. (Wanted %d)",
            function, len, params_no);
    return false;
  }
  return true;
}

static object_t *call_builtin(builtin_t builtin, cons_t *args)
{
  object_t obj, *rtrn;
  switch(builtin) {
    case AND:
      return correct_number_args("and", 2, args) ?
          and(eval(args->car), eval(args->cdr->car)) : NULL;
    case CAR:
      return correct_number_args("car", 1, args) ?
          eval(args->car) : NULL;
    case CDR:
      if (correct_number_args("cdr", 1, args))
        return NULL;
      obj.type = LIST;
      obj.cell = args;
      rtrn = cdr(eval(&obj));
      return rtrn;
        
    case CONS:
      return correct_number_args("cons", 1, args) ?
          cons(args->car, args->cdr->car) : NULL;
    case DEFINE:
      return correct_number_args("define", 2, args) ?
          define(args->car, eval(args->cdr->car)) : NULL;
    case COND:
      return correct_number_args("cond", 1, args) ?
          cond(args) : NULL;
    case LAMBDA:
      /*TODO*/
      return make_procedure(args->car->cell, args->cdr->car->cell);
    case NOT:
      return correct_number_args("not", 1, args) ?
          not(eval(args->car)) : NULL;
    case OR:
      return correct_number_args("or", 2, args) ?
          or(eval(args->car), eval(args->cdr->car)) : NULL;
    case PRINT:
      return correct_number_args("print", 1, args) ?
          print(eval(args->car)) : NULL;
    default: /*QUOTE*/
      return correct_number_args("quote", 1, args) ?
          quote(args->car) : NULL;
  }
}

/* Call function using args as a list of arguments.
 * A function call in Scheme/Lisp is a list with the car
 * being the lambda/function symbol (an object_t type with cell/string set), and
 * the cdr being the arguments. The cdr is seperated into a different list, and
 * than passed to apply:
 * ((lambda (a b c) (+ a b c)) 1 2 3)
 *  |_______________________|  |___|
 *             car              cdr
 *
 * Which is internally represented as:
 * ((lambda.((a . (b . (c . NULL)). ((ADD . (a . (b . (c . NULL)))) . NULL)))).(1 . (2 . (3 . NULL))))
 *  |         |____________________||_____________________________________| |  |                    |
 *  |              parameters                       body                    |  |                    |
 *  |_______________________________________________________________________|  |____________________|
 *                               function (car)                                      args (cdr)
 */

object_t *apply(object_t *function, cons_t *args)
{
  depth_inc();
  
  switch (function->type) {
    case BUILTIN:
      return call_builtin(function->builtin, args);
    case PREDICATE:
      return correct_number_args(strpred(function->predicate), 1, args) ?
          call_predicate(args->car, function->predicate) : NULL;
    case OPERATOR:
      return correct_number_args(strop(function->operator), 2, args) ?
          call_operator(function->operator, args) : NULL;
    case SYMBOL:
      function = sym_find(function->string);
      if (function == NULL) {
        return NULL;
      }
      return apply(function, args);
    case LIST:
      if (function->cell->car->builtin == LAMBDA) {
        /*Parameters in the lambda's "signature"*/
        cons_t *parameters = function->cell->cdr->car->cell;
        /*Arguments passed to the lambda*/
        cons_t *args_head = args;
        /*The lambda's body */
        cons_t *body = function->cell->cdr->cdr->car->cell;
    
        if (!correct_number_args(repr(function), length(parameters), args))
          return NULL;
        
        while (parameters != NULL) {
          sym_insert(parameters->car->string, args_head->car);
          args_head = args_head->cdr;
          parameters = parameters->cdr;
        }

        while (body->cdr != NULL) {
          if (eval(body->car) == NULL) /*Encountered some error, return to top level*/
            return NULL;
        }
        /*Reached the end of function.*/
        depth_dec();
        return eval(body->car);
      }
    default:
      fprintf(stderr, "Invalid Function: %s.", repr(function));
      return NULL;
  }
}

/* Evaluate object */
object_t *eval(object_t *obj)
{
  
  if (!obj->quoted) {
    object_t *val;
    switch (obj->type)
    {
      case LIST:
        val = apply(obj->cell->car, obj->cell->cdr);
        if (val == NULL) {
          /*Error during function application*/
          goto_top();
          return NULL;
        }
        
      case SYMBOL:
        if ((val = sym_find(obj->string)) == NULL) {
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
  builtin_t i;
  for (i = AND; i <= QUOTE; i++) {
    object_t *obj;
    obj  = obj_init();
    obj->type = BUILTIN;
    obj->builtin = i;
    sym_insert(builtin_func[i], obj);
  }
}
