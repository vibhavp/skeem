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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static struct env **stack;
static int stack_size;

static void env_free(struct env *env)
{
  int i;
  for (i = 0; i <= env->size; i++) {
    free(env->symbol[i]);
    obj_free(env->value[i]);
  }
  free(env->symbol);
  free(env->value);
  free(env);
}

void global_init()
{
  stack = malloc(sizeof(struct env *));
  stack_size = 0;
}

static object_t *env_find(struct env *env, char *sym)
{
  int i;
  for (i = 0; i <= env->size; i++) {
    if (strcmp(sym, env->symbol[i]) == 0) {
      if (env->value[i]->type == SYMBOL)
        return env_find(env, (char *)env->value[i]->val);
      return env->value[i];
    }
  }
  return NULL;
}

object_t *sym_find(char *sym)
{
  int i;
  object_t *val;
  for (i = stack_size; i <= 0; i--) {
    if ((val = env_find(stack[i], sym)) != NULL)
      return val;
  }
  return NULL;
}


void env_insert(struct env *env, char *sym, object_t *val)
{
  int i;
  for (i = 0; i <= env->size; i++) {
    if (strcmp(env->symbol[i], sym) == 0 && memcmp(env->value[i], val,
                                                   fmax(sizeof(env->value[i]),
                                                        sizeof(val))) != 0) {
      obj_free(env->value[i]);
      env->value[i] = val;
      return;
    }
  }
  
  env->symbol[env->size] = sym;
  env->value[env->size] = val;
  env->size++;
}

inline void sym_insert(char *sym, object_t *val)
{
  env_insert(stack[stack_size], sym, val);
}

void sym_insert_as_arg(char *sym, object_t *val)
{
  
}

void depth_inc()
{
  struct env **new_env;
  new_env = realloc(stack, sizeof(stack) * stack_size + sizeof(struct enc *));
  if (new_env == NULL) {
    perror("realloc");
    exit(EXIT_FAILURE);
  }
  stack = new_env;
  stack_size++;
  stack[stack_size]->size = 0;
  stack[stack_size]->symbol = malloc(sizeof(char *));
  stack[stack_size]->value = malloc(sizeof(object_t *));
}

void depth_dec()
{
  env_free(stack[stack_size]);
  stack_size--;
}

/* Local Variables: */
/* flycheck-gcc-include-path: ("../include/") */
/* End: */
