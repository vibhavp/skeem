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

#define PRED_BOOL_OBJ(predicate) ((predicate) ? CONST_TRUE : CONST_FALSE)

object_t *call_predicate(object_t *obj, predicate_t pred)
{  
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
