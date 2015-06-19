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

struct env {
  struct bind_tree *tree;
  object_t *next;
  object_t *prev;
};

object_t *env_global, *env_head;

static unsigned int max_obj = INIT_GC_THRESHOLD, num_obj;
/*Stores all allocated objects. Used by sweep()*/
static struct obj_list *heap = NULL, *heap_head = NULL;
/*Pinned objects get marked every GC cycle*/
static struct obj_list *pinned = NULL, *pin_head = NULL;

void *ERR_MALLOC(size_t bytes)
{
  void *ptr = calloc(1, bytes);

  if (ptr == NULL) {
    perror("calloc");
    exit(EXIT_FAILURE);
  }
  return ptr;
}

struct obj_list *obj_list_init()
{
  struct obj_list *n = ERR_MALLOC(sizeof(struct obj_list));
  return n;
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
void mem_init()
{
  heap = ERR_MALLOC(sizeof(struct obj_list));
  heap_head = heap;

  env_global = ERR_MALLOC(sizeof(object_t));
  env_global->env = ERR_MALLOC(sizeof(struct env));
  env_global->env->tree = ERR_MALLOC(sizeof(struct bind_tree));
  env_head = env_global;
}

object_t *obj_init(type_t type)
{
  if (num_obj == max_obj && !no_gc)
    gc();

  object_t *obj = ERR_MALLOC(sizeof(object_t));
  obj->type = type;
  obj->marked = false;

  if (type == SYMBOL){
    num_obj += 1;
    return obj;
  }

  heap_head->val = obj;
  heap_head->next = obj_list_init();
  heap_head->next->prev = heap_head;
  heap_head = heap_head->next;

#ifdef DEBUG
  printf("Allocated object type %s\n", strtype(type));
#endif

  if (type == ENVIRONMENT) {
    obj->env = ERR_MALLOC(sizeof(struct env));
    obj->env->tree = ERR_MALLOC(sizeof(struct bind_tree));
  }
  num_obj += 1;
  return obj;
}

void bind_tree_free(struct bind_tree *tree)
{
  if (tree != NULL)
  {
    bind_tree_free(tree->left);
    struct bind_tree *right = tree->right;
    free(tree->symbol);
    free(tree);
    bind_tree_free(right);
  }
}

void obj_free(object_t *obj)
{
  switch(obj->type) {
    case ENVIRONMENT:
      bind_tree_free(obj->env->tree);
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
#ifdef DEBUG
  printf("Freed object type %s\n", strtype(obj->type));
#endif
  free(obj);
}

void mark(object_t *obj)
{
  if (obj->marked)
    return;

  obj->marked = true;

  if (obj->type == LIST) {
    cons_t *cur = obj->cell;

    while (cur != NULL) {
      mark(cur->car);
      cur = cur->cdr;
    }
  }
}

void mark_bind_tree(struct bind_tree *tree)
{
  if (tree != NULL) {
    mark_bind_tree(tree->left);
    mark(tree->symbol);
    mark(tree->val);
    mark_bind_tree(tree->right);
  }
}

void mark_all()
{

  object_t *cur = env_global;
  
  while (cur != NULL) {
    if (cur->env->tree->symbol != NULL) /*tree isnt empty*/
      mark_bind_tree(cur->env->tree);
    cur->marked = true;
    cur = cur->env->next;
  }

  /*mark all pinned objects*/
  struct obj_list *cur_pin = pinned;
  if (pinned == NULL)
    return;

  if (cur_pin->val == NULL) {
    while (cur_pin != NULL) {
      mark(cur_pin->val);
      cur_pin = cur_pin->next;
    }
  }
}

void sweep()
{
  struct obj_list *curr = heap, *prev = NULL;

  while (curr->val != NULL) {

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
      curr->val->marked = false;
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

void tree_insert(struct bind_tree *tree, object_t *symbol, object_t *val)
{
  struct bind_tree *y = NULL, *x = tree;

  if (tree->symbol != NULL) { /*tree isnt empty*/
    while (x != NULL) {
      y = x;
      int diff = strcmp(symbol->string, x->symbol->string);

      if (diff < 0)
        x = x->left;
      else if (diff > 0)
        x = x->right;
      else { /*Symbol already exists*/
        x->val = val;
        return;
      }
    }
  }

  struct bind_tree *new = tree->symbol == NULL ?
      tree : ERR_MALLOC(sizeof(struct bind_tree));
  new->symbol = symbol;
  new->val = val;
  new->parent = y;

  if (y == NULL) {
    tree->root = new;
    return;
  }
  else if (strcmp(symbol->string, y->symbol->string) < 0)
    y->left = new;
  else
    y->right = new;
}

inline void env_insert(object_t *symbol, object_t *val)
{
  tree_insert(env_head->env->prev->env->tree, symbol, val);
}

inline void arg_insert(object_t *symbol, object_t *val)
{
  tree_insert(env_head->env->tree, symbol, val);
}

struct bind_tree *tree_lookup(struct bind_tree *tree, object_t *symbol)
{
  if (tree->symbol == NULL) /*tree is empty*/
    return NULL;
  int diff;

  while (tree != NULL && (diff = strcmp(symbol->string, tree->symbol->string))
                         != 0) {                    
    if (diff < 0)
      tree = tree->left;
    else if (diff > 0)
      tree = tree->right;
  }

  return tree == NULL ? NULL : tree;
}

object_t *env_lookup(object_t *symbol)
{
  struct bind_tree *bind = NULL;
  object_t *cur = env_head;

  while (bind == NULL && cur != NULL) {
    bind = tree_lookup(cur->env->tree, symbol);
    cur = cur->env->prev;
  }
  
  bind = bind == NULL ? tree_lookup(env_head->env->tree, symbol) : bind;
  
  return bind == NULL ? NULL : bind->val;  
}

struct bind_tree *env_lookup_node(object_t *symbol)
{
  struct bind_tree *bind = NULL;
  object_t *cur = env_head->env->prev;

  while (bind == NULL && cur != NULL) {
    bind = tree_lookup(cur->env->tree, symbol);
    cur = cur->env->prev;
  }

  return bind == NULL ? tree_lookup(env_head->env->tree, symbol) : bind;
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
  env_head->env->next = obj_init(ENVIRONMENT);
  env_head->env->next->env->prev = env_head;
  env_head = env_head->env->next;
  env_head->marked = true;
}

inline void env_pop()
{
  env_head = env_head->env->prev;
  env_head->env->next = NULL;
}

#if (__GNUC__ >= 4 && __GNUC_MINOR > 6) || defined(__clang__)
_Noreturn
#endif
void goto_top()
{
  while (env_head->env->next != NULL)
    env_pop();
  while (pinned != NULL)
    unpin_head();

  longjmp(err, 1);
}
