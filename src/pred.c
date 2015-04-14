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

#include "pred.h"
#include "types.h"
#include "builtins.h"
#include <stdbool.h>
#include <string.h>

#define BOOL_TO_OBJ(predicate) ((predicate) ? CONST_TRUE : CONST_FALSE)

static bool equal(object_t *obj1, object_t *obj2);
static bool eqv(object_t *obj1, object_t *obj2);

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

static bool eqv(object_t *obj1, object_t *obj2)
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

bool eq_cons(cons_t *cell1, cons_t *cell2) {
  if (cell1 && cell2)
    return equal(cell1->car, cell2->car) && eq_cons(cell1->cdr, cell2->cdr);
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
