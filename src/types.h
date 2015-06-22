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

#ifndef TYPES_H
#define TYPES_H
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define GCC_VERSION (__GNUC__ * 10000                 \
                     + __GNUC_MINOR__ * 100           \
                     + __GNUC_PATCHLEVEL__)

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
  PREDICATE,
  ENVIRONMENT
} type_t;

typedef enum {
  AND,
  CAR,
  CDR,
  CONS,
  DEFINE,
  EVAL,
  EXIT,
  GC,
  IF,
  LAMBDA,
  LENGTH,
  NOT,
  OR,
  PRINT,
  QUOTE,
  SET,
  WHILE
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
  LAMBDA_P,
  BOOLEAN_P,
  EQV_P,
  EQUAL_P
} predicate_t;

#define BUILTIN_LEN 27
extern char *builtin_syms[];
struct cons;

typedef struct _object_t {
  type_t type;
  bool marked;
  union {
    int64_t integer;
    double flt;
    
    char *string;
    char character;
    struct cons *cell;
    bool boolean;
    
    builtin_t builtin;
    operator_t operator;
    predicate_t predicate;

    /*This allows environments to be GC'd*/
    struct env *env;
  };
} object_t;

typedef struct cons {
  object_t *car;
  struct cons *cdr;
} cons_t;

extern cons_t *tok_to_cons(char **tokens, char *types, int *index);
extern char *strpred(predicate_t pred);
extern char *strop(operator_t op);
extern int length(cons_t *list);
extern char *strtype(type_t type);
extern char *repr(object_t *obj);
extern void print_obj(object_t *obj, FILE *stream);

#endif
