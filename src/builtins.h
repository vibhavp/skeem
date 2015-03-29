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

#include "types.h"
#ifndef BUILTINS_H
#define BUILTINS_H

object_t *quote(object_t *object);
object_t *cond(struct cons *clauses);
object_t *and(object_t *cond1, object_t *cond2);
object_t *or(object_t *cond1, object_t *cond2);
object_t *not(object_t *cond);
object_t *quote(object_t *obj);
object_t *print(object_t *obj);
object_t *apply(object_t *function, cons_t *args);
object_t *eval(object_t *obj);
object_t *define(object_t *sym, object_t *val);
object_t *dup_obj(object_t *obj);
object_t *divide(object_t *n1, object_t *n2);
object_t *multiply(object_t *n1, object_t *n2);
object_t *cons(object_t *car, object_t *cdr);
char *repr(object_t *obj);

extern object_t *builtins[12];
extern object_t *CONST_TRUE;
extern object_t *CONST_FALSE;

#endif
