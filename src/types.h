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

#ifndef TYPES_H
#define TYPES_H
#include <stdint.h>
#include <stdbool.h>

/*Object types.*/
typedef enum types {
  INTEGER,
  FLOAT,
  CHAR,
  STRING,
  SYMBOL,
  LIST,
  BOOLEAN,
  BUILTIN,
  OPERATOR,
  PREDICATE
} type_t;

#define LPAREN BUILTIN+1
#define RPAREN BUILTIN+2

typedef enum {
  AND,
  CAR,
  CDR,
  COND,
  CONS,
  DEFINE,
  LAMBDA,
  NOT,
  OR,
  PRINT,
  QUOTE
} builtin_t;

typedef enum {
  ADD,
  SUBTRACT,
  DIVIDE,
  MULTIPLY
} operator_t;

typedef enum {
  INTEGER_P,
  FLOAT_P,
  NUMBER_P,
  STRING_P,
  SYMBOL_P,
  LIST_P,
  LAMBDA_P
} predicate_t;

struct cons;

typedef struct _object_t {
  type_t type;
  bool quoted;
  union {
    /*A hackish numeric tower*/
    union {
      int64_t integer;
      double flt;
    };
    char *string;
    struct cons *cell;
    union {
      builtin_t builtin;
      operator_t operator;
      predicate_t predicate;
    };
    bool boolean;
  };
} object_t;

typedef struct cons {
  object_t *car;
  struct cons *cdr;
} cons_t;

cons_t *tok_to_cons();
void cons_free(cons_t *cell);
object_t *obj_init();
void obj_free(object_t *obj);
object_t *obj_dup(object_t *obj);
cons_t *dup_cell(cons_t *cell);
int check_arg_type(object_t *obj, int n, ...);
char *strpred(predicate_t pred);
char *strop(operator_t op);

#endif
