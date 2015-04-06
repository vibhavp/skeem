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

#include "env.h"
#include "mem.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void env_push()
{
  object_t *prev_env = env_head;
  env_head->env->next = obj_init(ENVIRONMENT);
  env_head = env_head->env->next;
  env_head->env->prev = prev_env;
}

void env_pop()
{
  env_head = env_head->env->prev;
  env_head->env->next = NULL;
}

void env_insert(object_t *sym, object_t *val)
{
  int i;
  env_t *env = env_head->env;
  env->binding = realloc(env->binding, sizeof(binding_t) * env->size
                         + sizeof(binding_t));
  if (env->binding == NULL) {
    perror("realloc");
    exit(EXIT_FAILURE);
  }
  /*Check if the symbol is already bound*/
  for (i = 0; i < env->size; i++) {
    if (strcmp(env->binding[i]->sym->string, sym->string) == 0) {
      env->binding[i]->val = val;
      return;
    }
  }
  
  env->binding[env->size++]->sym = sym;
  env->binding[env->size]->val = val;
}

object_t *env_lookup(object_t *sym)
{
  env_t *cur_env = env_head->env;
  
  while (cur_env != NULL) {
    int i;
    for (i = 0; i < cur_env->size; i++) {
      if (strcmp(cur_env->binding[i]->sym->string, sym->string) == 0)
        return cur_env->binding[i]->val;
    }
  }
  fprintf(stderr, "Unbound variable: %s", sym->string);
  return NULL;
}
