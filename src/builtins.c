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

  if (_INTEGER_P(n1)) {
      if (_INTEGER_P(n2)) {
        result = obj_init(INTEGER);
        result->integer = n1->integer + n2->integer;
      }
      else if (_FLOAT_P(n2)) {
        result = obj_init(FLOAT);
        result->flt = n1->integer + n2->flt;
      }
      else
        goto err;
    }

  else if (_FLOAT_P(n1)) {
    result = obj_init(FLOAT);

    if (_INTEGER_P(n2))
      result->flt = n1->flt + n2->integer;
    else if (_FLOAT_P(n2))
      result->flt = n1->flt + n2->flt;  
    else
      goto err;
  }

  else
    goto err;

  return result;

err:
  fprintf(stderr,"add: Wrong argument type(s)\n");
  goto_top();
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

  if (_INTEGER_P(n1)) {
      if (_INTEGER_P(n2)) {
        result = obj_init(INTEGER);
        result->integer = n1->integer * n2->integer;
      }
      else if (_FLOAT_P(n2)) {
        result = obj_init(FLOAT);
        result->flt = n1->integer * n2->flt;
      }
      else
        goto err;
    }

  else if (_FLOAT_P(n1)) {
    result = obj_init(FLOAT);

    if (_INTEGER_P(n2))
      result->flt = n1->flt * n2->integer;
    else if (_FLOAT_P(n2))
      result->flt = n1->flt * n2->flt;  
    else
      goto err;
  }

  else
    goto err;

  return result;

err:
  fprintf(stderr,"multiply: Wrong argument type(s)\n");
  goto_top();
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
  return IS_TRUE(eval(obj)) ? CONST_FALSE : CONST_TRUE;
}

/*Execute EXP while pred evaluates to true*/
object_t *loop_while(object_t *pred, object_t *exp)
{
  object_t *last = CONST_FALSE;

  while (IS_TRUE(eval(pred))) {
    last = eval(exp);
  }

  return last;
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

  return CONST_TRUE;
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

object_t *if_else(object_t *pred, object_t *exp1, object_t *exp2)
{
  if (IS_TRUE(eval(pred)))
    return eval(exp1);

  return eval(exp2);
}

object_t *set(object_t *sym, object_t *val)
{
  if (_SYMBOL_P(sym)) {
    struct bind_tree *bind = env_lookup_node(sym);

    if (bind == NULL) {
      fprintf(stderr, "Unbound variable: %s\n", sym->string);
      goto_top();
    }
    bind->val = val;
  }

  return val;
}

object_t *define(object_t *sym, object_t *val)
{
  if (_SYMBOL_P(sym))
    env_insert(sym, val);
  else {
    fprintf(stderr, "Wrong argument type - %s (needed symbol)\n", strtype(sym->type));
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
      return multiply(eval(args->car), eval(args->cdr->car));
  }
}

#define BOOL_TO_OBJ(predicate) ((predicate) ? CONST_TRUE : CONST_FALSE)

/*true if the length of args == params_no. Else, print an error message and
 * return false*/
static void correct_number_args(char *function, int params_no,
                                cons_t *args)
{
  int len = length(args);
  if (len != params_no) {
    fprintf(stderr,
            "Wrong number of arguments to %s (Got %d, Wanted %d)\n",
            function, len, params_no);
    goto_top();
  }
}

object_t *call_predicate(cons_t *obj, predicate_t pred)
{ 
  switch(pred)
  {
    case INTEGER_P:
      correct_number_args(strpred(pred), 1, obj);
      return BOOL_TO_OBJ(_INTEGER_P(eval(obj->car)));
    case FLOAT_P:
      correct_number_args(strpred(pred), 1, obj);
      return BOOL_TO_OBJ(_FLOAT_P(eval(obj->car)));
    case NUMBER_P:
      correct_number_args(strpred(pred), 1, obj);
      return BOOL_TO_OBJ(_NUMBER_P(eval(obj->car)));
    case STRING_P:
      correct_number_args(strpred(pred), 1, obj);
      return BOOL_TO_OBJ(_STRING_P(eval(obj->car)));
    case SYMBOL_P:
      correct_number_args(strpred(pred), 1, obj);
      return BOOL_TO_OBJ(_SYMBOL_P(obj->car));
    case LIST_P:
      correct_number_args(strpred(pred), 1, obj);
      return BOOL_TO_OBJ(_LIST_P(eval(obj->car)));
    case LAMBDA_P:
      correct_number_args(strpred(pred), 1, obj);
      return BOOL_TO_OBJ(_LAMBDA_P(eval(obj->car)));
    case BOOLEAN_P:
      correct_number_args(strpred(pred), 1, obj);
      return BOOL_TO_OBJ(_BOOLEAN_P(eval(obj->car)));
    case EQV_P:
      correct_number_args(strpred(pred), 2, obj);
      return BOOL_TO_OBJ(eqv(eval(obj->car), eval(obj->cdr->car)));
    case EQUAL_P:
      correct_number_args(strpred(pred), 2, obj);
      return BOOL_TO_OBJ(equal(eval(obj->car), eval(obj->cdr->car)));
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

object_t *car(object_t *obj)
{
  if (obj->type != LIST) {
    fprintf(stderr, "Wrong argument type - %s. (Expected list)\n",
            strtype(obj->type));
    goto_top();
  }
  return obj->cell->car;
}

object_t *cdr(object_t *obj)
{
  if(obj->type != LIST) {
    fprintf(stderr, "Wrong argument type - %s. (Expected list)\n",
            strtype(obj->type));
    goto_top();
  }
  object_t *o = obj_init(LIST);
  o->cell = obj->cell->cdr;
  return o->cell == NULL ? EMPTY_LIST : o;
}

object_t *obj_len(object_t *obj)
{
  if (obj->type != LIST) {
    fprintf(stderr, "Wrong argument type - %s. (Expected list)\n",
            strtype(obj->type));
    goto_top();
  }

  object_t *len = obj_init(INTEGER);
  len->integer = length(obj->cell);

  return len;
}

static object_t *call_builtin(builtin_t builtin, cons_t *args)
{
  switch(builtin) {
    case AND:
      correct_number_args("and", 2, args);
      return and(args->car, args->cdr->car);
    case CAR:
      correct_number_args("car", 1, args);
      return car(eval(args->car));
    case CDR:
      correct_number_args("cdr", 1, args);
      return cdr(eval(args->car));
    case CONS:
      correct_number_args("cons", 2, args);
      return cons(args->car, args->cdr->car);
    case DEFINE:
      correct_number_args("define", 2, args);
      return define(args->car, eval(args->cdr->car));
    case EVAL:
      correct_number_args("eval", 1, args);
      return eval(args->car);
    case EXIT:
      correct_number_args("exit", 0, args);
      printf("Exiting\n");
      exit(EXIT_SUCCESS);
    case GC:
      correct_number_args("garbage-collect", 0, args);
      gc();
      return CONST_TRUE;
    case IF:
      if (length(args) == 2)
        return if_else(args->car, args->cdr->car, CONST_FALSE);
      correct_number_args("cond", 3, args);
      return if_else(args->car, args->cdr->car, args->cdr->cdr->car);
    case LENGTH:
      correct_number_args("length", 1 , args);
      return obj_len(eval(args->car));
    case NOT:
      correct_number_args("not", 1, args);
      return not(eval(args->car));
    case OR:
      correct_number_args("or", 2, args);
      return or(args->car, args->cdr->car);
    case PRINT:
      correct_number_args("print", 1, args);
      return print(eval(args->car));
    case SET:
      correct_number_args("set", 2, args);
      return set(args->car, eval(args->cdr->car));
    case WHILE:
      correct_number_args("while", 2, args);
      return loop_while(eval(args->car), args->cdr->car);
    default: /*QUOTE*/
      correct_number_args("quote", 1, args);
      return quote(args->car);
  }
}

/* Call function using args as a list of arguments.
 * ((lambda (a b c) (+ a b c)) 1 2 3)
 *  |_______________________|  |___|
 *             car              cdr
 */

object_t *apply(object_t *function, cons_t *args)
{
  switch (function->type) {
    case BUILTIN:
      return call_builtin(function->builtin, args);
    case PREDICATE:
      return call_predicate(args, function->predicate);
    case OPERATOR:
      correct_number_args(strop(function->operator), 2, args);
      return call_operator(function->operator, args);
    case SYMBOL:
      function = eval(function);
      return apply(function, args);
    case LIST:
      env_push();
      if (function->cell->car->builtin == LAMBDA) {
        /*Parameters in the lambda's "signature"*/
        cons_t *parameters = function->cell->cdr->car->cell;
        /*Arguments passed to the lambda*/
        cons_t *args_head = args;
        /*The lambda's body */
        cons_t *body = function->cell->cdr->cdr;

        if (_LIST_P(body->car) && _LIST_P(body->car->cell->car))
          body = body->car->cell;

        correct_number_args("lambda", length(parameters), args);

        while (parameters != NULL) {
          env_insert(parameters->car, eval(args_head->car));
          args_head = args_head->cdr;
          parameters = parameters->cdr;        }

        while (body->cdr != NULL) {
          eval(body->car);
          body = body->cdr;
        }
        /*Reached the end of function.*/
        env_pop();
        return eval(body->car);
      }
    default:
      fprintf(stderr, "Invalid Function: ");
      print_obj(function, stderr);
      goto_top();
  }
}

/* Evaluate object */
object_t *eval(object_t *obj)
{
  if (obj == NULL)
    return NULL;

  switch (obj->type)
  {
    case LIST:
      {
        obj->marked = true;
        
        if (obj->cell->car->type == BUILTIN && obj->cell->car->builtin == LAMBDA)
          return correct_number_args("lambda", 2, obj->cell->cdr), obj;
        env_push();

        object_t *val = apply(obj->cell->car, obj->cell->cdr);
        env_pop();

        obj->marked = false;
        return val;
      }
    case SYMBOL:
      {
        object_t *result = env_lookup(obj);

        if (result == NULL) {
          fprintf(stderr, "Unbound variable: %s\n", obj->string);
          goto_top();
        }

        while (result->type == SYMBOL)
          result = env_lookup(obj);
        
        return result;
      }
    default:
      return obj;
  }
}

char *builtin_syms[BUILTIN_LEN];

/*Initialize all builtins, including procedures, predicates, and operators*/
void builtins_init()
{
  char *tmp[BUILTIN_LEN] =  {"and", "car", "cdr",
                             "cons", "define", "eval",
                             "exit", "garbage-collect", "if",
                             "lambda", "length", "not",
                             "or", "print", "quote", "set", "while",
                             "integer?", "float?",
                             "number?", "string?", "symbol?",
                             "list?", "lambda?", "boolean?",
                             "eqv?", "equal?"};
  for (int i = 0; i < BUILTIN_LEN; i++)
    builtin_syms[i] = tmp[i];

  for (builtin_t i = AND; i <= WHILE; i++) {
    builtins[i] = ERR_MALLOC(sizeof(object_t));
    builtins[i]->type = BUILTIN;
    builtins[i]->builtin = i;
  }

  for(predicate_t i = INTEGER_P; i <= EQUAL_P; i++) {
    int index = WHILE+i+1;

    builtins[index] = ERR_MALLOC(sizeof(object_t));
    builtins[index]->type = PREDICATE;
    builtins[index]->predicate = i;
  }

  for (operator_t i = ADD; i <= MULTIPLY; i++) {
    int index = WHILE+EQUAL_P+i+2;

    builtins[index] = ERR_MALLOC(sizeof(object_t));
    builtins[index]->type = OPERATOR;
    builtins[index]->operator = i;
  }

  CONST_TRUE = ERR_MALLOC(sizeof(object_t));
  CONST_TRUE->type = BOOLEAN;
  CONST_TRUE->boolean = true;

  CONST_FALSE = ERR_MALLOC(sizeof(object_t));
  CONST_FALSE->type = BOOLEAN;
  CONST_FALSE->boolean = false;

  EMPTY_LIST = ERR_MALLOC(sizeof(object_t));

#ifdef DEBUG
  printf("Initialised builtins\n");
#endif
}

/* Local Variables:  */
/* mode: c           */
/* flycheck-gcc-args: ("-std=gnu11") */
/* End:              */
