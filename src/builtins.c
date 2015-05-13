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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "types.h"
#include "mem.h"
#include "builtins.h"

object_t *add(object_t *n1, object_t *n2)
{
  object_t *result;

  if (_INTEGER_P(n1) || _INTEGER_P(n2)) {
    result = obj_init(INTEGER);
    result->integer = n1->integer + n2->integer;
  }
  else if (_INTEGER_P(n1) || _FLOAT_P(n2)) {
    result = obj_init(FLOAT);
    result->flt = n1->integer + n2->flt;
  }
  else if (_FLOAT_P(n1) || _INTEGER_P(n2)) {
    result = obj_init(FLOAT);
    result->flt = n1->flt + n2->integer;
  }
  else if (_FLOAT_P(n1) || _FLOAT_P(n2)){
    result = obj_init(FLOAT);
    result->flt = n1->flt + n2->flt;
  }
  else {
    fprintf(stderr, "add: Wrong argument type(s).");
    goto_top();
  }

  return result;
}

object_t *subtract(object_t *n1, object_t *n2)
{

  if (_FLOAT_P(n2))
    n2->flt = -n2->flt;
  else if (_INTEGER_P(n2))
    n2->integer = -n2->integer;

  object_t *result = add(n1, n2);

  if (_FLOAT_P(n2))
    n2->flt = -n2->flt;
  else if (_INTEGER_P(n2))
    n2->integer = -n2->integer;

  return result;
}

#define NUMBER(n) (((n)->type == INTEGER) ? n->integer : n->flt)

object_t *divide(object_t *n1, object_t *n2)
{
  if (_NUMBER_P(n1) && _NUMBER_P(n2)) {
    if (NUMBER(n1) == 0 || NUMBER(n2) == 0) {
      fprintf(stderr, "divide: Division by zero.\n");
      goto_top();
    }
      
    object_t *result = obj_init(FLOAT);
    result->flt = NUMBER(n1) / NUMBER(n2);

    return result;
  }

  fprintf(stderr, "divide: Wrong argument type(s)\n");
  goto_top();
}

object_t *multiply(object_t *n1, object_t *n2)
{
  object_t *result;

  if (_INTEGER_P(n1) || _INTEGER_P(n2)) {
    result = obj_init(INTEGER);
    result->integer = n1->integer * n2->integer;
  }
  else if (_INTEGER_P(n1) || _FLOAT_P(n2)) {
    result = obj_init(FLOAT);
    result->flt = n1->integer * n2->flt;
  }
  else if (_FLOAT_P(n1) || _INTEGER_P(n2)) {
    result = obj_init(FLOAT);
    result->flt = n1->flt * n2->integer;
  }
  else if (_FLOAT_P(n1) || _FLOAT_P(n2)){
    result = obj_init(FLOAT);
    result->flt = n1->flt * n2->flt;
  }
  else {
    fprintf(stderr, "add: Wrong argument type(s).");
    goto_top();
  }

  return result;
}

#define IS_FALSE(val) (_BOOLEAN_P((val)) && !(val)->boolean)
#define IS_TRUE(val) (!IS_FALSE((val)))

object_t *and(object_t *test1, object_t *test2)
{
  object_t *val1 = eval(test1);
  if (IS_FALSE(val1))
    return val1;

  return eval(test2);

}

object_t *or(object_t *test1, object_t *test2)
{
  object_t *val1 = eval(test1);
  if (IS_TRUE(val1))
    return val1;

  return eval(test2);
}

inline object_t *not(object_t *obj)
{
  return IS_TRUE(eval(obj)) ? CONST_TRUE : CONST_FALSE;
}

object_t *print(object_t *obj)
{
  switch (obj->type) {
    case INTEGER:
      printf("%ld", obj->integer);
      break;
    case FLOAT:
      printf("%f", obj->flt);
    case STRING:
      printf("%s", obj->string);
      break;
    case CHAR:
      printf("%c", obj->character);
      break;
    case BOOLEAN:
      printf("%d", obj->boolean);
      break;
    default:
      fprintf(stderr, "Type %s isn't printable.\n", strtype(obj->type));
      goto_top();
  }

  return obj;
}

object_t *cons(object_t *obj1, object_t *obj2)
{
  object_t *cons = obj_init(LIST);

  cons->cell = cons_init();
  cons->cell->car = obj1;
  cons->cell->cdr = cons_init();
  cons->cell->cdr->car = obj2;

  return cons;
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
  cons_t *curr_cell = clauses, *clause;
  object_t *val;
  int clause_no = 1;

  do {
    clause = curr_cell->car->cell;
    val = eval(clause->car);
    /*Evaluate body if clause condition doesnt evaluate to #f*/
    if (IS_FALSE(val)) {
      if (clause->cdr == NULL) {
        fprintf(stderr, "No consequent for clause %d", clause_no);
        goto_top();
      }
      return eval(clause->cdr->car);
    }
    curr_cell = curr_cell->cdr;
  }while(curr_cell != NULL);

  /*None of conditions are true*/
  fprintf(stderr, "None of the conditions in cond expression are true.");
  goto_top();
}

object_t *cdr(object_t *cell)
{
  object_t *obj = obj_init(LIST);
   obj->cell = cell->cell->cdr;
  return obj;
}

inline object_t *car(cons_t *cell)
{
  return cell->car;
}

object_t *define(object_t *sym, object_t *val)
{
  if (_SYMBOL_P(sym))
    env_insert(sym, val);
  else {
    fprintf(stderr, "Wrong argument type - %s (needed symbol)", strtype(sym->type));
    goto_top();
  }

  return val;
}

inline object_t *quote(object_t *obj)
{
  return obj;
}

static object_t *call_operator(operator_t op, cons_t *args)
{
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

#define BOOL_TO_OBJ(predicate) ((predicate) ? CONST_TRUE : CONST_FALSE)

object_t *call_predicate(cons_t *obj, predicate_t pred)
{
  switch(pred)
  {
    case INTEGER_P:
      return BOOL_TO_OBJ(_INTEGER_P(obj->car));
    case FLOAT_P:
      return BOOL_TO_OBJ(_FLOAT_P(obj->car));
    case NUMBER_P:
      return BOOL_TO_OBJ(_NUMBER_P(obj->car));
    case STRING_P:
      return BOOL_TO_OBJ(_STRING_P(obj->car));
    case SYMBOL_P:
      return BOOL_TO_OBJ(_SYMBOL_P(obj->car));
    case LIST_P:
      return BOOL_TO_OBJ(_LIST_P(obj->car));
    case LAMBDA_P:
      return BOOL_TO_OBJ(_LAMBDA_P(obj->car));
    case BOOLEAN_P:
      return BOOL_TO_OBJ(_BOOLEAN_P(obj->car));
    case EQV_P:
      return BOOL_TO_OBJ(eqv(obj->car, obj->cdr->car));
    case EQUAL_P:
      return BOOL_TO_OBJ(equal(obj->car, obj->cdr->car));
  }
}

bool eqv(object_t *obj1, object_t *obj2)
{
  if (obj1->type == obj2->type) {
    switch(obj1->type) {
      case BOOLEAN:
        return obj1->boolean == obj2->boolean;
      case SYMBOL:
        return strcmp(obj1->string, obj2->string) == 0;
      case INTEGER:
        return obj1->integer == obj2->integer;
      case FLOAT:
        return obj1->flt == obj2->flt;
      case CHAR:
        return obj1->character == obj2->character;
      case LIST:
        /*Only returns true if both point to the same _location_, as per R5RS*/
        return obj1->cell == obj2->cell;
      case STRING:
        return obj1->string == obj2->string;
      case BUILTIN:
        return obj1->builtin == obj2->builtin;
      case OPERATOR:
        return obj1->operator == obj2->operator;
      default: /*PREDICATE, environment types are hidden from the user*/
        return obj1->predicate == obj2->predicate;
    }
  }
  return false;
}

bool eq_cons(cons_t *cell1, cons_t *cell2)
{
  if (cell1 && cell2) {
    if (equal(cell1->car, cell2->car)) /*Using && wouldn't result in TCO*/
      return eq_cons(cell1->cdr, cell2->cdr);

    return false;
  }
  /*If false returned, both lists are of unequal length*/
  return cell1 == NULL && cell2 == NULL;
}

/*Compares lists/strings recursively*/
bool equal(object_t *obj1, object_t *obj2)
{
  if (obj1->type == obj2->type) {
    switch(obj1->type) {
      case LIST:
        return eq_cons(obj1->cell, obj2->cell);
      case STRING:
        return strcmp(obj1->string, obj2->string) == 0;
      default:
        return eqv(obj1, obj2);
    }
  }
  return false;
}

/*true if the length of args == params_no. Else, print an error message and
 * return false*/
static void correct_number_args(char *function, int params_no,
                                cons_t *args)
{
  int len = length(args);
  if (len != params_no) {
    fprintf(stderr,
            "Wrong number of arguments to %s - %d. (Wanted %d)\n",
            function, len, params_no);
    goto_top();
  }
}

static object_t *call_builtin(builtin_t builtin, cons_t *args)
{
  object_t obj, *rtrn;
  switch(builtin) {
    case AND:
      correct_number_args("and", 2, args);
      return and(args->car, args->cdr->car);
    case CAR:
      correct_number_args("car", 1, args);
      return eval(args->car);
    case CDR:
      correct_number_args("cdr", 1, args);
      obj.type = LIST;
      obj.cell = args;
      rtrn = cdr(eval(&obj));
      return rtrn;

    case CONS:
      correct_number_args("cons", 1, args);
      return cons(args->car, args->cdr->car);
    case DEFINE:
      correct_number_args("define", 2, args);
      return define(args->car, eval(args->cdr->car));
    case COND:
      correct_number_args("cond", 1, args);
      return cond(args);
    case LAMBDA:
      /*TODO*/
      /* return make_procedure(args->car->cell, args->cdr->car->cell); */
    case NOT:
      correct_number_args("not", 1, args);
      return not(eval(args->car));
    case OR:
      correct_number_args("or", 2, args);
      return or(args->car, args->cdr->car);
    case PRINT:
      correct_number_args("print", 1, args);
      return print(eval(args->car));
    default: /*QUOTE*/
      correct_number_args("quote", 1, args);
      return quote(args->car);
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
  switch (function->type) {
    case BUILTIN:
      return call_builtin(function->builtin, args);
    case PREDICATE:
      correct_number_args(strpred(function->predicate), 1, args);
      return call_predicate(args, function->predicate);
    case OPERATOR:
      correct_number_args(strop(function->operator), 2, args);
      return call_operator(function->operator, args);
    case SYMBOL:
      function = env_lookup(function);
      return apply(function, args);
    case LIST:
      if (function->cell->car->builtin == LAMBDA) {
        /*Parameters in the lambda's "signature"*/
        cons_t *parameters = function->cell->cdr->car->cell;
        /*Arguments passed to the lambda*/
        cons_t *args_head = args;
        /*The lambda's body */
        cons_t *body = function->cell->cdr->cdr->car->cell;

        correct_number_args("lambda", length(parameters), args);

        while (parameters != NULL) {
          env_insert(parameters->car, args_head->car);
          args_head = args_head->cdr;
          parameters = parameters->cdr;
        }

        while (body->cdr != NULL) {
          eval(body->car);
        }
        /*Reached the end of function.*/
        return eval(body->car);
      }
    default:
      fprintf(stderr, "Invalid Function: ");
      /* print_obj(function, stderr); */
      goto_top();
  }
}

/* Evaluate object */
__attribute__((hot))
object_t *eval(object_t *obj)
{
  no_gc = false;
  switch (obj->type)
  {
    case LIST:
      env_push();
      cons_t *cur = obj->cell->cdr;

      while (cur != NULL) {
        pin(cur->car);
        cur = cur->cdr;
      }
      object_t *val = apply(obj->cell->car, obj->cell->cdr);

      for (int i = 0; i < length(obj->cell->cdr); i++)
        unpin_head();

      env_pop();
      return val;
    case SYMBOL:
      return eval(env_lookup(obj));

    default:
      return obj;
  }
}

/*Initialize all builtins, including procedures, predicates, and operators*/
void builtins_init()
{
  for (builtin_t i = AND; i <= QUOTE; i++) {
    builtins[i] = ERR_MALLOC(sizeof(object_t));
    builtins[i]->type = BUILTIN;
    builtins[i]->builtin = i;
  }

  for(predicate_t i = INTEGER_P; i <= EQUAL_P; i++) {
    int index = QUOTE+i+1;

    builtins[index] = ERR_MALLOC(sizeof(object_t));
    builtins[index]->type = PREDICATE;
    builtins[index]->predicate = i;
  }

  for (operator_t i = ADD; i <= MULTIPLY; i++) {
    int index = EQUAL_P+i+2;

    builtins[index] = ERR_MALLOC(sizeof(object_t));
    builtins[index]->type = OPERATOR;
    builtins[index]->operator = i;
  }



  CONST_TRUE = ERR_MALLOC(sizeof(object_t));
  CONST_TRUE->type = BOOLEAN;
  CONST_TRUE->boolean = true;

  CONST_FALSE = ERR_MALLOC(sizeof(object_t));
  EMPTY_LIST = ERR_MALLOC(sizeof(object_t));

#ifdef DEBUG
  printf("Initialised builtins\n");
#endif
}

/* Local Variables:  */
/* mode: c           */
/* flycheck-gcc-args: ("-std=gnu11") */
/* End:              */
