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

#include "token.h"
#include "types.h"
#include "builtins.h"
#include "mem.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <setjmp.h>

#define SKEEM_VERSION "1.0a"
#define get_input(i) {size_t __n__; getline(&(i), &__n__, stream);}

int main(int argc, char **argv)
{
#ifdef DEBUG
  setbuf(stdout, NULL);
#endif
  char *input = NULL;
  FILE *stream = argc > 1 ? fopen(argv[1], "r") : stdin;

  if (stream == NULL) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  if (stream == stdin)
    printf("skeem version %s\n", SKEEM_VERSION);
  mem_init();
  builtins_init();

  paren_depth = 0;
  nquotes = 0;

  while (true) {
    if (stream == stdin)
      printf("skeem> ");

    if (setjmp(err)) {
      if (stream != stdin)
        exit(EXIT_FAILURE);

      clear_tokens();
      continue;
    }
input:
    get_input(input);

    if (input[0] == '\n')    
      continue;

    scan(input, strlen(input));

    if (feof(stdin))
      break;
    if (paren_depth < 0) {
      fprintf(stderr, "Unbalanced expression\n");
      paren_depth = 0;
      longjmp(err, 1);
    }
    if (nquotes % 2 != 0 || paren_depth != 0) {
      if (stream == stdin) {
          printf("... ");
          fflush(stdout);
      }
      goto input;
    }

    register object_t *obj = tokens_to_obj();
    if (obj == NULL)
      continue;
    no_gc = false;
    obj = eval(obj);
    
    if (stream == stdin) {
      printf("\n=> ");
      fflush(stdout);
      print_obj(obj, stdout);
      putchar('\n');
    }

    clear_tokens();
    if (feof (stream))
      break;
  }

  return 0;
}
