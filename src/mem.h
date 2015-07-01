/* Copyright (c) 2015 Vibhav Pant <vibhavp@gmail.com>

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

#ifndef MEM_H
#define MEM_H
#include <stdbool.h>
#include <stddef.h>
#include "types.h"
#include <setjmp.h>

bool no_gc;
jmp_buf err;

/*Binary tree for symbol table*/
struct bind_tree {
  object_t *symbol;
  object_t *val;
  struct bind_tree *root;
  struct bind_tree *parent;
  struct bind_tree *left;
  struct bind_tree *right;
};

extern cons_t *cons_init();
extern void cons_free(cons_t *cell);
extern object_t *obj_init(type_t type);
extern void obj_free(object_t *obj);
extern void pin(object_t *obj);
extern void unpin_head();
extern void print_heap();
extern void print_pinned();
extern void mem_init();
extern void env_push();
extern void env_pop();
extern void goto_top();
extern void env_insert(struct _object_t *sym, struct _object_t *val);
extern void arg_insert(struct _object_t *sym, struct _object_t *val);
extern struct bind_tree *env_lookup_node(object_t *symbol);
extern struct _object_t *env_lookup(struct _object_t *sym);
extern struct _object_t *arg_lookup(struct _object_t *sym);
extern void gc();
extern void mark();

#ifdef emalloc
#define ERR_MALLOC err_malloc
#else
#define ERR_MALLOC emalloc
#endif
extern void *ERR_MALLOC(size_t bytes);


#define INIT_GC_THRESHOLD 20

#endif
