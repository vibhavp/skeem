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


#ifndef PRED_H
#define PRED_H
#include "types.h"

#define _INTEGER_P(n)  ((n)->type == INTEGER)
#define _FLOAT_P(n)    ((n)->type == FLOAT)
#define _NUMBER_P(n)   (_INTEGER_P((n)) || _FLOAT_P((n)))
#define _STRING_P(n)   ((n)->type == STRING)
#define _SYMBOL_P(n)   ((n)->type == SYMBOL)
#define _LIST_P(n)     ((n)->type == LIST)
#define _LAMBDA_P(n)   (_LIST_P((n))                            \
                        && (n)->cell->car->type == BUILTIN      \
                        && (n)->cell->car->builtin == LAMBDA)
#define _BOOLEAN_P(n) ((n)->type == BOOLEAN)

extern object_t *call_predicate(cons_t *obj, predicate_t pred);
#endif
