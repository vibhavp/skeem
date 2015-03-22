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

enum types {
  INTEGER,
  FLOAT,
  CHAR,
  STRING,
  SYMBOL,
  LIST,
  LPAREN,
  RPAREN,
  QUOTED
};

struct numeric {
  char type;
  void *num;
};

typedef struct _object_t {
  enum types type;
  void *val;
} object_t;

struct cons {
  object_t *car;
  struct cons *cdr;
};

#define cast_int(x) (*((int *)(x)))
#define cast_float(x) (*((float *)(x)))
#define cast_cons(x) ((struct cons *)(x))
#define cast_obj(x) (*(object_t *)(x))
struct cons *tok_to_cons();
void cons_free(struct cons *cell);
object_t *obj_init();
void obj_free(object_t *obj);

#endif