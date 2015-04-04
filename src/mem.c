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

#include "mem.h"
#include "env.h"
#include <stdio.h>
#include <stdlib.h>

/*Variables for GC purposes*/

/*Stores all allocated objects. Used by sweep()*/
static object_t **heap = NULL;
static unsigned int heap_size = 0;

/*Pinned variables will be marked on a GC cycle*/
struct pin {
  object_t *val;
  struct pin *next;
};
static struct pin *pinned, *head;

/*Stores all symbols. Used by mark()*/
static symbol_t **symbols = NULL;
static unsigned int nsym = 0; /*Stores the number of symbols to be marked*/

symbol_t *sym_init()
{
  symbol_t *sym = malloc(sizeof(symbol_t));
  if (sym == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  symbols = realloc(symbols, sizeof(symbol_t *)*nsym + sizeof(symbol_t *));
  if (symbols == NULL) {
    perror("realloc");
    exit(EXIT_FAILURE);
  }
  symbols[nsym++] = sym;

  return sym;
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

object_t *obj_init()
{
  object_t *obj;
  if ((obj = malloc(sizeof(object_))) == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  obj->marked = false;
  heap = realloc(heap, sizeof(object_t *)*heap_size + sizeof(object_t *));

  if (heap == NULL) {
    perror("realloc");
    exit(EXIT_FAILURE);
  }
  
  heap[heap_size++] = obj;
  return obj;
}

void obj_free(object_t *obj)
{
  switch(obj->type) {
    case LIST:
      cons_free(obj->cell);
      break;

    case SYMBOL:
    case STRING:
      free(obj->string);
    default:
      free(obj);
  }
}
struct pin *alloc_pin()
{
  struct pin *p = malloc(sizeof(struct pin));
  if (p == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  return p;
}

void pin(object_t *obj) {

  if (head == NULL) {
    pinned = alloc_pin();
    head = pinned;
  }
  else {
    head->next = alloc_pin();
    head = head->next;
  }
  head->val = obj;
}

void unpin(object_t *obj)
{
  struct pin *p = pinned;
  /*unpinning a non-existing object will result in a segfault*/
  while (p != NULL) {
    if (p->next->val == obj) {
      struct pin *next = p->next->next;
      free(p->next);
      p->next = next;
    }
    p = p->next;
  }
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

static inline void mark_symbol(symbol_t *sym)
{
  mark(sym->sym);
  mark(sym->val);
}

void mark_all()
{
  unsigned int i;
  for (i = 0; i <= nsym; i++) {
    mark_symbol(symbols[i]);
  }
}

void sweep()
{
  unsigned int i;
  for (i = 0; i <= heap_size; i++) {
    if (!heap[i]->marked) {
      free(heap[i]);
      heap_size--;
    }
    else
      heap[i]->marked = false;
  }
}

void gc()
{
  mark_all();
  sweep();
}
