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
#include "builtins.h"
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>

/*Variables for GC purposes*/
struct obj_list {
  object_t *val;
  struct obj_list *next;
  struct obj_list *prev;
};

static unsigned int max_obj = INIT_GC_THRESHOLD, num_obj;

/*Stores all allocated objects. Used by sweep()*/
static struct obj_list *heap = NULL, *heap_head = NULL;

/*Pinned objects get marked every GC cycle*/
static struct obj_list *pinned = NULL, *pin_head = NULL;

void *ERR_MALLOC(size_t bytes)
{
  void *ptr = malloc(bytes);

  if (ptr == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  return ptr;
}

void heap_init()
{
  heap = ERR_MALLOC(sizeof(struct obj_list));
  heap_head = heap;
}

void root_env_init()
{
  root_env = ERR_MALLOC(sizeof(object_t));
  root_env->type = ENVIRONMENT;
  root_env->env = ERR_MALLOC(sizeof(env_t));

  root_env->env->size = 0;
  root_env->env->prev = NULL;

  env_head = root_env;
}

struct obj_list *obj_list_init()
{
  struct obj_list *n = ERR_MALLOC(sizeof(struct obj_list));
  n->next = NULL;
  n->prev = NULL;
  return n;
}

binding_t *bind_init()
{
  binding_t *bind = ERR_MALLOC(sizeof(binding_t));
  return bind;
}

void pin(object_t *obj)
{

#ifdef DEBUG
  printf("Pinned object\n");
#endif
  if (pinned == NULL) {
    pinned = obj_list_init();
    pin_head = pinned;
  }
  else {
    pin_head->next = obj_list_init();
    pin_head->next->prev = pin_head;
    pin_head = pin_head->next;
  }

  pin_head->val = obj;
}

void unpin_head()
{
#ifdef DEBUG
  printf("Unpinned head.\n");
#endif
  pin_head->val->marked = false;
  if (pin_head != pinned) {
    pin_head = pin_head->prev;
    free(pin_head->next);
  }

  else { /*The only pinned object is unpinned*/
    free(pinned);
    pin_head = NULL;
    pinned = NULL;
  }
}

cons_t *cons_init()
{
  cons_t *cell = ERR_MALLOC(sizeof(cons_t));
  cell->car = NULL;
  cell->cdr = NULL;

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

/*Initialize the heap, root environment, and pinned list*/
void init_mem()
{
  heap = ERR_MALLOC(sizeof(struct obj_list));
  heap_head = heap;

  root_env = ERR_MALLOC(sizeof(object_t));
  root_env->type = ENVIRONMENT;
  root_env->env = ERR_MALLOC(sizeof(env_t));
  root_env->env->size = 0;
  root_env->env->prev = NULL;
  env_head = root_env;

  pinned = obj_list_init();
  pin_head = pinned;
}

object_t *obj_init(type_t type)
{
  if (num_obj == max_obj)
    gc();
  
  object_t *obj = ERR_MALLOC(sizeof(object_t));
  obj->type = type;
  obj->marked = false;
  
  /*if heap_head->val == NULL, this is the first object in the heap*/
  if (heap_head != NULL) {
    heap_head->next = obj_list_init();
    heap_head->next->prev = heap_head;
    heap_head = heap_head->next;
  }

  else
    heap_init();

  heap_head->val = obj;
#ifdef DEBUG
  printf("Allocated object type %s\n", strtype(type));
#endif
  num_obj += !no_gc;
  if (type == ENVIRONMENT) {
    obj->env = ERR_MALLOC(sizeof(env_t));
  }
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
  while (cur_env != NULL) { /*Mark all bindings*/  
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
  
  /*mark all pinned objects*/
  struct obj_list *cur_pin = pinned;
  while (cur_pin != NULL) {
    mark(cur_pin->val);
    cur_pin = cur_pin->next;
  }
}

void sweep()
{
  struct obj_list *curr = heap, *prev = NULL;

  while (curr != NULL) {

    if (!curr->val->marked) {
      obj_free(curr->val);
      if (prev != NULL) {
        prev->next = curr->next;
        free(curr);
        curr = prev->next;
      }
      else {
        /*the first object on the heap is being freed*/
        heap = heap->next;
        free(curr);
        curr = heap;
      }
      num_obj--;
    }
    else {
      prev = curr;
      curr = curr->next;
    }
  }
}

void gc()
{
#ifdef DEBUG
  printf("Started GC cycle\n");
#endif
  
  mark_all();
  sweep();
  max_obj = num_obj * 2;
}



void print_obj_list(struct obj_list *list)
{
  struct obj_list *curr = list;

  while (curr != NULL) {
    object_t *obj = curr->val;
    switch(obj->type) {
      case STRING:
        printf("<string>\t\"%s\" \n", obj->string);
        break;
      case SYMBOL:
        printf("<symbol>\t%s\n", obj->string);
        break;
      case INTEGER:
        printf("<integer>\t%ld\n", obj->integer);
        break;
      case FLOAT:
        printf("<float>\t%f\n", obj->flt);
        break;
      case ENVIRONMENT:
        printf("<environment>\n");
        break;
      case LIST:
        printf("cons cell\n");
        break;
      case CHAR:
        /* printf("<character>\t"); */
        /* putchar(obj->character); */
        /* printf("\n"); */
        printf("<character> '%c'\n", obj->character);
    }
    curr = curr->next;
  }
}

void print_heap()
{
  printf("Current heap: \n");
  print_obj_list(heap);
}

void print_pinned()
{
  printf("Pinned objects: \n");
  print_obj_list(pinned);
}

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

extern jmp_buf err;
__attribute__((noreturn))
void goto_top()
{
  while (env_head->env->prev != NULL)
    env_pop();
  while (pinned != NULL)
    unpin_head();
  
  longjmp(err, 1);
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

  env->binding[env->size] = malloc(sizeof(binding_t));
  env->binding[env->size]->sym = sym;
  env->binding[env->size]->val = val;
  env->size++;
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
  goto_top();
}
