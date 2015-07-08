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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "types.h"
#include "mem.h"
#include "builtins.h"

#define error(...)                \
  {                               \
    fprintf(stderr, __VA_ARGS__); \
    goto_top();                   \
  }

static object_t *ZERO, *ONE;

char *types[9] = {"integer", "float", "string", "symbol", "list",
                  "boolean", "procedure", "procedure", "closure"};

object_t *eval(object_t *);
object_t *eval_nopush(object_t *);

void add(object_t *n1, object_t *n2, object_t *result) {

  if (_INTEGER_P(n1)) {
    if (_INTEGER_P(n2)) {
      result->type = INTEGER;
      result->integer = n1->integer + n2->integer;
    } else if (_FLOAT_P(n2)) {
      result->type = FLOAT;
      result->flt = n1->integer + n2->flt;
    }
    else error("add: Wrong argument type - %s (Expected number)\n", types[n2->type]);
  }

  else if (_FLOAT_P(n1)) {
    if (_INTEGER_P(n2)) {
      result->type = FLOAT;
      result->flt = n1->flt + n2->integer;
    }
    else if (_FLOAT_P(n2)) {
      result->type = FLOAT;
      result->flt = n1->flt + n2->flt;
    }
    else error("add: Wrong argument type - %s (Expected number)\n", types[n2->type]);
  }

  else error("add: Wrong argument type - %s (Expected number)\n", types[n1->type]);
}

object_t *add_list(cons_t *args)
{
  if (args == NULL) /*no arguments*/
    return ZERO;

  object_t *result = obj_init(INTEGER);
  result->type = INTEGER;
  result->integer = 0;

  while (args != NULL) {
    add(result, eval(args->car), result);
    args = args->cdr;
  }

  return result;
}

#define NUMBER(n) (((n)->type == INTEGER) ? n->integer : n->flt)

void subtract(object_t *n1, object_t *n2, object_t *result) {
  if (_FLOAT_P(n2))
    n2->flt = -n2->flt;
  else if (_INTEGER_P(n2))
    n2->integer = -n2->integer;

  add(n1, n2, result);

  if (_FLOAT_P(n2))
    n2->flt = -n2->flt;
  else if (_INTEGER_P(n2))
    n2->integer = -n2->integer;
}

object_t *subtract_list(cons_t *args)
{
  if (args == NULL)
    return ZERO;

  object_t *result = obj_init(args->car->type);

  if (_INTEGER_P(args->car))
    result->integer = args->car->integer;
  else if (_FLOAT_P(args->car))
    result->flt = args->car->flt;

  while (args != NULL) {
    subtract(result, args->car, result);
    args = args->cdr;
  }
  return result;
}

void divide(object_t *n1, object_t *n2, object_t *result) {
  if (_NUMBER_P(n1) && _NUMBER_P(n2)) {
    if (NUMBER(n1) == 0 || NUMBER(n2) == 0) {
      error("divide: Division by zero.\n");
    }
    result->flt = NUMBER(n1) / NUMBER(n2);
  }

  error("divide: Wrong argument type(s)\n");
}

object_t *divide_list(cons_t *args) {
  if (args == NULL)
    return ONE;

  object_t *result = obj_init(FLOAT);
  result->flt = 1.0;

  while (args != NULL) {
    divide(args->car, result, result);
    args = args->cdr;
  }
  return result;
}

object_t *multiply(object_t *n1, object_t *n2, object_t *result) {
  if (_INTEGER_P(n1)) {
    if (_INTEGER_P(n2)) {
      result->type = INTEGER;
      result->integer = n1->integer * n2->integer;
    } else if (_FLOAT_P(n2)) {
      result->type = FLOAT;
      result->flt = n1->integer * n2->flt;
    } else error("Wrong argument type - %s (Wanted number)\n", types[n2->type])
  }

  else if (_FLOAT_P(n1)) {
    result->type = FLOAT;

    if (_INTEGER_P(n2))
      result->flt = n1->flt * n2->integer;
    else if (_FLOAT_P(n2))
      result->flt = n1->flt * n2->flt;
    else error("Wrong argument type - %s (Wanted number)\n", types[n2->type]);
  }

  else error("Wrong argument type - %s (Wanted number)\n", types[n2->type]);

  return result;
}


object_t *multiply_list(cons_t *args)
{
  if (args == NULL)
    return ONE;

  object_t *result = obj_init(INTEGER);
  result->integer = 1;

  while (args != NULL) {
    multiply(result, args->car, result);
    args = args->cdr;
  }
  return result;
}

#define BOOL_TO_OBJ(predicate) ((predicate) ? CONST_TRUE : CONST_FALSE)

/*true if the length of args == params_no. Else, print an error message and
 * return false*/
void correct_number_args(const char *function, int params_no, cons_t *args) {
  int len = length(args);
  if (len != params_no) {
    error("Wrong number of arguments to %s (Got %d, Wanted %d)\n", function,
          len, params_no);
  }
}

#define assert_arity(ar) correct_number_args(__func__, (ar), args)
object_t *greater(cons_t *args)
{
  assert_arity(2);
  object_t *n1 = eval(args->car);
  object_t *n2 = eval(args->cdr->car);

  if (_INTEGER_P(n1)) {
    if (_FLOAT_P(n2)) return BOOL_TO_OBJ(n1->integer > n2->flt);
    if (_INTEGER_P(n2)) return BOOL_TO_OBJ(n1->integer > n2->integer);
    else goto n1err;
  }
  if (_FLOAT_P(n1)) {
    if (_FLOAT_P(n2)) return BOOL_TO_OBJ(n1->flt > n2->flt);
    if (_INTEGER_P(n2)) return BOOL_TO_OBJ(n1->flt > n2->integer);
    else goto n2err;
  }
  /*n1 is neither int or float*/
n1err:
  error("greater: Wrong argument type - %s (Wanted number)\n", types[n1->type]);
n2err:
  error("greater: Wrong argument type - %s (Wanted number)\n", types[n2->type]);
}

object_t *lesser(cons_t *args)
{
  assert_arity(2);
  object_t *n1 = eval(args->car);
  object_t *n2 = eval(args->cdr->car);

  if (_INTEGER_P(n1)) {
    if (_FLOAT_P(n2)) return BOOL_TO_OBJ(n1->integer < n2->flt);
    if (_INTEGER_P(n2)) return BOOL_TO_OBJ(n1->integer < n2->integer);
    else goto n1err;
  }
  if (_FLOAT_P(n1)) {
    if (_FLOAT_P(n2)) return BOOL_TO_OBJ(n1->flt < n2->flt);
    if (_INTEGER_P(n2)) return BOOL_TO_OBJ(n1->flt < n2->integer);
    else goto n2err;
  }
  /*n1 is neither int or float*/
n1err:
  error("lesser: Wrong argument type - %s (Wanted number)\n", types[n1->type]);
n2err:
  error("lesser: Wrong argument type - %s (Wanted number)\n", types[n2->type]);
}

#define IS_FALSE(val) (_BOOLEAN_P((val)) && !(val)->boolean)
#define IS_TRUE(val) (!IS_FALSE((val)))

object_t *and(cons_t *args)
{
  if (args == NULL) return CONST_TRUE;

  object_t *val;
  while (args != NULL) {
    val = eval(args->car);
    if (IS_FALSE(val)) return val;
    args = args->cdr;
  }
  return val;
}

object_t *or(cons_t *args)
{
  if (args == NULL) return CONST_FALSE;

  object_t *val;
  while (args != NULL) {
    val = eval(args->car);
    if (IS_TRUE(val)) return val;
    args = args->cdr;
  }
  return val;

}

object_t *not(cons_t *args) {
  assert_arity(1);
  return IS_TRUE(args->car) ? CONST_FALSE : CONST_TRUE;
}

/*Execute EXP while pred evaluates to true*/
object_t *loop_while(cons_t *args) {
  assert_arity(2);
  object_t *pred = eval(args->car);
  object_t *exp = eval(args->cdr->car);
  object_t *last = CONST_FALSE;

  while (IS_TRUE(eval(pred))) {
    last = eval(exp);
  }

  return last;
}

object_t *cons(cons_t *args) {
  assert_arity(2);
  object_t *obj1 = eval(args->car);
  object_t *obj2 = eval(args->cdr->car);
  object_t *cons = obj_init(LIST);

  cons->cell = cons_init();
  cons->cell->car = obj1;
  cons->cell->cdr = cons_init();
  cons->cell->cdr->car = obj2;

  return cons;
}

object_t *if_else(cons_t *args) {
  assert_arity(3);
  if (IS_TRUE(eval(args->car))) return eval(args->cdr->car);
  return eval(args->cdr->cdr->car);
}

object_t *set(cons_t *args) {
  assert_arity(2);
  object_t *sym = args->car;
  object_t *val = eval(args->cdr->car);

  if (_SYMBOL_P(sym)) {
    struct bind_tree *bind = env_lookup_node(sym);

    if (bind == NULL) {
      error("Unbound variable: %s\n", sym->string);

    }
    bind->val = val;
  }

  return val;
}

static object_t *make_procedure(char *name, cons_t *params, object_t *body)
{
  object_t *proc = obj_init(PROCEDURE);
  proc->procedure->name = name;
  proc->procedure->params = params;
  proc->procedure->body = body;
  return proc;
}

object_t *define(cons_t *args) {
  assert_arity(2);
  object_t *sym = args->car;
  object_t *val = args->cdr->car;

  if (_SYMBOL_P(sym))
    env_insert(sym, eval(val));

  else if (_LIST_P(sym))
    env_insert(sym->cell->car,
               make_procedure(sym->cell->car->string,
                              args->car->cell->cdr, args->cdr->car));
  else
    error("Wrong argument type - %s (needed symbol)\n", types[sym->type]);

  return val;
}

object_t *quote(cons_t *args)
{
  assert_arity(1);
  return args->car;
}

bool _eqv(object_t *obj1, object_t *obj2)
{
  if (obj1->type == obj2->type) {
    switch (obj1->type) {
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
      case PRIMITIVE:
        return obj1->primitive == obj2->primitive;
    }
  }
  return false;
}

object_t *eqv(cons_t *args)
{
  assert_arity(2);
  return BOOL_TO_OBJ(_eqv(args->car, args->cdr->car));
}

bool _equal(object_t *obj1, object_t *obj2);

bool eq_cons(cons_t *cell1, cons_t *cell2) {
  if (cell1 && cell2) {
    if (_equal(cell1->car, cell2->car)) /*Using && wouldn't result in TCO*/
      return eq_cons(cell1->cdr, cell2->cdr);

    return false;
  }
  /*If false returned, both lists are of unequal length*/
  return cell1 == NULL && cell2 == NULL;
}

/*Compares lists/strings recursively*/
bool _equal(object_t *obj1, object_t *obj2) {
  if (obj1->type == obj2->type) {
    switch (obj1->type) {
      case LIST:
        return eq_cons(obj1->cell, obj2->cell);
      case STRING:
        return strcmp(obj1->string, obj2->string) == 0;
      default:
        return _eqv(obj1, obj2);
    }
  }
  return false;
}

object_t *equal(cons_t *args)
{
  assert_arity(2);
  return BOOL_TO_OBJ(_equal(args->car, args->cdr->car));
}

object_t *car(cons_t *args)
{
  assert_arity(1);

  if (args->car->type != LIST)
    error("Wrong argument type - %s. (Expected list)\n", types[args->car->type]);

  return args->car->cell->car;
}

object_t *cdr(cons_t *args)
{
  assert_arity(1);

  if (args->car->type != LIST)
    error("Wrong argument type - %s. (Expected list)\n", types[args->car->type]);
  object_t *o = obj_init(LIST);
  o->cell = args->car->cell->cdr;
  return o->cell == NULL ? EMPTY_LIST : o;
}

object_t *lambda(cons_t *args)
{
  assert_arity(2);

  if (!_LIST_P(args->car))
    error("Wrong argument type - %s (Expected list)\n", types[args->car->type]);

  return make_procedure(strdup("lambda"), args->car->cell, args->cdr->car);
}

#if GCC_VERSION >= 40700
_Noreturn
#endif
object_t *exit_status(cons_t *args)
{
  assert_arity(1);
  if (_INTEGER_P(args->car)) exit(args->car->integer);

  error("Wrong argument type - %s. (Expected integer)\n", types[args->car->type]);
}

object_t *apply_procedure(procedure_t *procedure, cons_t *args)
{
  cons_t *cur_param = procedure->params;
  cons_t *cur_arg = args;
  object_t *body = procedure->body;
  
  if (body == EMPTY_LIST)
    return EMPTY_LIST;
  
  cons_t *cur_exp = procedure->body->cell;
  
  if (_LIST_P(cur_exp->car) && _LIST_P(cur_exp->car->cell->car))
    cur_exp = cur_exp->car->cell;
  
  correct_number_args(procedure->name,
                      length(procedure->params),
                      args);
  while (cur_param != NULL) {
    arg_insert(cur_param->car, eval(cur_arg->car));
    cur_arg = cur_arg->cdr;
    cur_param = cur_param->cdr;
  }
  
  object_t *last;
  if (!_LIST_P(body) || !_LIST_P(body->cell->car)) {
    last = eval_nopush(body);
    goto closure_check;
  }
  
  while (cur_exp->cdr != NULL) {
    eval(cur_exp->car);
    cur_exp = cur_exp->cdr;
  }
  
  last = eval(cur_exp->car);

closure_check:
  /*closure*/
  if (last->type == PROCEDURE) {
    object_t *cl = obj_init(CLOSURE);
    cl->closure->env = env_head;
    cl->closure->proc = last; 
    return cl;
  }
  return last;
}

object_t *apply(object_t *function, cons_t *args)
{
  switch (function->type) {
    case PRIMITIVE:
      return function->primitive(args);
    case SYMBOL:
      function = eval(function);
      return apply(function, args);
    case PROCEDURE:
      return apply_procedure(function->procedure, args);
    case CLOSURE:
      function->closure->env->env->prev = env_head;
      env_head->env->next = function->closure->env;
      env_head = env_head->env->next;
      object_t *val = apply_procedure(function->closure->proc->procedure, args);
      env_pop();
      return val;
    default:
      fprintf(stderr, "Invalid Function: ");
      print_obj(function, stderr);
      fprintf(stderr, "\n");
      goto_top();
  }
}

/* Evaluate object */
object_t *_eval(object_t *obj, bool push)
{
  if (obj == NULL) return NULL;

  switch (obj->type) {
    case LIST:
      {
        if (obj == EMPTY_LIST) return EMPTY_LIST;
        
        mark(obj);
        if (push) env_push();
        object_t *val = apply(obj->cell->car, obj->cell->cdr);
        if (push) env_pop();
        
        return val;
      }
    case SYMBOL: {
      object_t *result = env_lookup(obj);

      if (result == NULL) {
        error("Unbound variable: %s\n", obj->string);
      }

      while (result->type == SYMBOL) result = env_lookup(obj);

      return result;
    }
    default:
      return obj;
  }
}

inline object_t *eval(object_t *obj)
{
  return _eval(obj, true);
}

inline object_t *eval_nopush(object_t *obj)
{
  return _eval(obj, false);
}

void add_primitive(char *name, primitive_t function)
{
  object_t *p = ERR_MALLOC(sizeof(object_t));
  p->type = PRIMITIVE;
  p->primitive = function;
  object_t *n = ERR_MALLOC(sizeof(object_t));
  n->type = SYMBOL;
  n->string = strdup(name);
  arg_insert(n, p);
}

object_t *garbage_collect(cons_t *args)
{
  assert_arity(0);
  gc();
  return CONST_TRUE;
}

object_t *integer_p(cons_t *args)
{
  assert_arity(1);
  return BOOL_TO_OBJ(_INTEGER_P(args->car));
}

object_t *float_p(cons_t *args)
{
  assert_arity(1);
  return BOOL_TO_OBJ(_FLOAT_P(args->car));
}

object_t *number_p(cons_t *args)
{
  assert_arity(1);
  return BOOL_TO_OBJ(_NUMBER_P(args->car));
}

object_t *string_p(cons_t *args)
{
  assert_arity(1);
  return BOOL_TO_OBJ(_STRING_P(args->car));
}

object_t *symbol_p(cons_t *args)
{
  assert_arity(1);
  return BOOL_TO_OBJ(_SYMBOL_P(args->car));
}

object_t *list_p(cons_t *args)
{
  assert_arity(1);
  return BOOL_TO_OBJ(_LIST_P(args->car));
}

object_t *procedure_p(cons_t *args)
{
  assert_arity(1);
  return BOOL_TO_OBJ(_PROCEDURE_P(args->car));
}

object_t *boolean_p(cons_t *args)
{
  assert_arity(1);
  return BOOL_TO_OBJ(_BOOLEAN_P(args->car));
}

object_t *closure_p(cons_t *args)
{
  assert_arity(1);
  return BOOL_TO_OBJ(_CLOSURE_P(args->car));
}

object_t *print(cons_t *args)
{
  assert_arity(1);
  print_obj(args->car, stdout);
  return CONST_TRUE;
}

/*Initialize all builtin primitives and constants*/
void builtins_init()
{
  add_primitive("+", add_list);
  add_primitive("-", subtract_list);
  add_primitive("*", multiply_list);
  add_primitive("/", divide_list);
  add_primitive(">", greater);
  add_primitive("<", lesser);
  add_primitive("and", and);
  add_primitive("or", or);
  add_primitive("not", not);
  add_primitive("while", loop_while);
  add_primitive("cons", cons);
  add_primitive("if", if_else);
  add_primitive("set!", set);
  add_primitive("define", define);
  add_primitive("quote", quote);
  add_primitive("eqv?", eqv);
  add_primitive("equal?", equal);
  add_primitive("car", car);
  add_primitive("cdr", cdr);
  add_primitive("lambda", lambda);
  add_primitive("exit", exit_status);
  add_primitive("garbage-collect", garbage_collect);
  add_primitive("print", print);
  /*Predicates*/
  add_primitive("integer?", integer_p);
  add_primitive("float?", float_p);
  add_primitive("number?", number_p);
  add_primitive("string?", string_p);
  add_primitive("symbol?", symbol_p);
  add_primitive("list?", list_p);
  add_primitive("procedure?", procedure_p);
  add_primitive("boolean?", boolean_p);
  add_primitive("closure?", closure_p);
  
  CONST_TRUE = ERR_MALLOC(sizeof(object_t));
  CONST_TRUE->type = BOOLEAN;
  CONST_TRUE->boolean = true;

  CONST_FALSE = ERR_MALLOC(sizeof(object_t));
  CONST_FALSE->type = BOOLEAN;
  CONST_FALSE->boolean = false;

  EMPTY_LIST = ERR_MALLOC(sizeof(object_t));
  EMPTY_LIST->type = LIST;

  ZERO = ERR_MALLOC(sizeof(object_t));
  ZERO->type = INTEGER;
  ZERO->integer = 0;

  ONE = ERR_MALLOC(sizeof(object_t));
  ONE->type = INTEGER;
  ONE->integer = 1;
}

/* Local Variables:  */
/* mode: c           */
/* flycheck-gcc-args: ("-std=gnu11") */
/* End:              */
