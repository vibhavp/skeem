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

#include "mem.h"
#include "env.h"
#include <stdio.h>
#include <stdlib.h>

/*Variables for GC purposes*/
struct obj_list {
  object_t *val;
  struct obj_list *next;
  struct obj_list *prev;
};

/*Stores all allocated objects. Used by sweep()*/
static struct obj_list *heap, *heap_head = NULL;

void heap_init()
{
  heap = malloc(sizeof(struct obj_list));
  if (heap == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  heap_head = heap;
}

void root_env_init()
{
  root_env = obj_init(ENVIRONMENT);
  root_env->env = malloc(sizeof(env_t));

  if (root_env->env == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  
  root_env->env->size = 0;
  root_env->env->prev = NULL;
  
  env_head = root_env;
}

struct obj_list *obj_list_init()
{
  struct obj_list *n = malloc(sizeof(struct obj_list));
  if (n == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  n->next = NULL;
  n->prev = NULL;
  return n;
}

binding_t *bind_init()
{
  binding_t *bind = malloc(sizeof(binding_t));

  if (bind == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  
  return bind;
}

cons_t *cons_init()
{
  cons_t *cell = NULL;
  if ((cell = malloc(sizeof(cons_t))) == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  return cell;
}

void cons_free(cons_t *cell)
{
  cons_t *curr = cell;

  while (curr != NULL)
  {
    obj_free(curr->car);
    curr = curr->cdr;
  }
}

object_t *obj_init(type_t type)
{
  object_t *obj;
  if ((obj = malloc(sizeof(object_t))) == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  obj->type = type;
  obj->marked = false;

  if (heap->val != NULL) {
    heap_head->next = obj_list_init();
    heap_head = heap_head->next;
  }
  
  heap_head->val = obj;

  return obj;
}

void obj_free(object_t *obj)
{
  switch(obj->type) {
    case ENVIRONMENT:
      free(obj->env->binding);
      break;
    case STRING:
    case SYMBOL:
      free(obj->string);
      break;
    case LIST:
      free(obj->cell);
      break;
    default:
      break;
  }
  free(obj);
}


void mark(object_t *obj)
{
  if (obj->marked)
    return;
  
  obj->marked = true;

  if (obj->type == LIST) {
    cons_t *head = obj->cell->cdr;
    mark(obj->cell->car);
    
    if (head != NULL) {
      mark(head->car);
      head = head->cdr;
    }
  }
}

static inline void mark_symbol(binding_t *sym)
{
  mark(sym->sym);
  mark(sym->val);
}

void mark_all()
{
  object_t *cur_env = root_env;
  int i;
  while (cur_env != NULL) {
    
    for (i = 0; i < cur_env->env->size; i++) {
      mark_symbol(cur_env->env->binding[i]);
    }
    cur_env = cur_env->env->next;
  }
  /*mark all environment objects*/
  object_t *cur = root_env->env->next;
  while (cur != NULL) {
    cur->marked = true;
    cur = cur->env->next;
  }
}

void sweep()
{
  struct obj_list *curr = heap->next, *prev = NULL;
  while (curr != NULL) {
    if (!curr->val->marked)
    {
      obj_free(curr->val);
      if (prev != NULL) {
        prev->next = curr->next;
        free(curr);
      }
    }
    prev = curr;
    curr = curr->next;
  }
}

void gc()
{
  mark_all();
  sweep();
}

#define DEBUG
#ifdef DEBUG
void print_heap()
{
  struct obj_list *curr = heap;

  while (curr != NULL) {
    object_t *obj = curr->val;
    switch(obj->type) {
      case STRING:
        printf("\"%s\" \n", obj->string);
        break;
      case SYMBOL:
        printf("<symbol> %s\n", obj->string);
        break;
      case INTEGER:
        printf("%ld\n", obj->integer);
        break;
      case FLOAT:
        printf("%f\n", obj->flt);
        break;
      case ENVIRONMENT:
        printf("<environment>\n");
        break;
      case LIST:
        printf("cons cell\n");
        break;
    }
    curr = curr->next;
  }
}
#endif
