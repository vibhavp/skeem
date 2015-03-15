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

#include "env.h"
#include "types.h"
#include <stdlib.h>
#include <string.h>

struct env *env_init()
{
  struct env *env = malloc(sizeof(struct env));
  env->size = 0;
  env->symbol = NULL;
  e->value = NULL;
}

void env_free(struct env *env)
{
  int i;
  for (i = 0; i <= env->size; i++) {
    free(env->symbol[i]);
    obj_free(env->value[i]);
  }
  free(env->symbol);
  free(env->val);
  free(env);
}

object_t *env_find(struct env *env, char *sym)
{
  int i;
  for (i = 0; i  <= env->size; i++) {
    if (strcmp(env->sym[i], sym) == 0)
      return env->value[i];
  }
  return NULL;
}

void *env_insert(struct env *env, char *sym, object_t *val)
{
  int i;
  for (i = 0; i <= env->size; i++) {
    if (strcmp(env->symbol[i], sym) == 0 && memcmp(env->value[i], val) != 0) {
      obj_free(env->val[i]);
      env->val[i] = val;
      return;
    }
  }
  
  env->symbol = sym;
  env->value[env->size] = val;
  env->size++;
}
