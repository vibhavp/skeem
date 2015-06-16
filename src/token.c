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
#include "mem.h"
#include "builtins.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

enum tok_type {
  TOK_INT,
  TOK_FLOAT,
  /* TOK_CHAR, */
  TOK_STRING,
  TOK_SYMBOL,
  TOK_OPERATOR_PLUS,
  TOK_OPERATOR_MINUS,
  TOK_OPERATOR_DIVIDE,
  TOK_OPERATOR_MULTIPLY,
  TOK_PAREN_OPEN,
  TOK_PAREN_CLOSE,
};

typedef struct _token {
  enum tok_type type;
  union {
    char *string;
    char character;
    int64_t integer;
    double flt;
  };

  struct _token *next;
} token_t;

static token_t *tokens, *head_tok, *list_end;

object_t *token_to_obj(token_t *tok);

enum tok_type type(char *word, size_t start)
{
  enum tok_type sub;

  switch(word[start]) {
    case '+':
      if (word[start+1] != '\0') {
        sub = type(word, start+1);
        return sub == TOK_FLOAT || sub == TOK_INT ? sub : TOK_SYMBOL;
      }
      return TOK_OPERATOR_PLUS;
    case '-':
      if (word[start+1] != '\0') {
        sub = type(word, start+1);
          return sub == TOK_FLOAT || sub == TOK_INT ? sub : TOK_SYMBOL;
      }
      return TOK_OPERATOR_MINUS;
    case '*':
      if (word[start+1] != '\0')
        return TOK_SYMBOL;
      return TOK_OPERATOR_MULTIPLY;
    case'/':
      if (word[start+1] != '\0')
        return TOK_SYMBOL;
      return TOK_OPERATOR_DIVIDE;
    case '0'...'9':
      for (size_t i = start; i < strlen(word); i++) {

        if (!(word[i] >= '0' && word[i] <= '9')) {
          if (word[i] == '.' && type(word, i+1) == TOK_INT)
            return TOK_FLOAT;
          return TOK_SYMBOL;
        }
      }
      return TOK_INT;
    case '\"':
      return TOK_STRING;
    case '(':
      return TOK_PAREN_OPEN;
    case ')':
      return TOK_PAREN_CLOSE;
    default:
      return TOK_SYMBOL;
  }
}

token_t *str_to_tok(char *word)
{
  token_t *tok = ERR_MALLOC(sizeof(token_t));
  tok->type = type(word, 0);
  tok->next = NULL;

  switch(tok->type)
  {
    case TOK_INT:
      tok->integer = atoi(word);
      return tok;
    case TOK_FLOAT:
      tok->flt = atof(word);
      return tok;
    case TOK_STRING:
      {
      /*Subtract 2 for the commas*/
        size_t size = (strlen(word) - 2)*sizeof(char);
        tok->string = malloc(size);
        strncpy(tok->string, word+1, size);
      }
      return tok;
    case TOK_SYMBOL:
      tok->string = strdup(word);
      return tok;
    default:
      return tok;
  }
  return NULL;
}

void add_token(char *str)
{
  if (tokens == NULL) {
    tokens = str_to_tok(str);
    head_tok = tokens;
  }
  else {
    head_tok->next = str_to_tok(str);
    head_tok = head_tok->next;
  }
}

cons_t *token_to_cons(token_t *tok)
{
  object_t *obj = token_to_obj(tok);
  cons_t *cell;
  
  if (obj == NULL)
    return NULL;
  if (obj->type == LIST)
    tok = list_end;
  
  cell = cons_init();
  cell->car = obj;
  cell->cdr = token_to_cons(tok->next);
  return cell;
}

object_t *token_to_obj(token_t *tok)
{
  object_t *obj;
  no_gc = true;

  switch (tok->type) {
    case TOK_INT:
      obj = obj_init(INTEGER);
      obj->integer = tok->integer;
      return obj;
    case TOK_FLOAT:
      obj = obj_init(FLOAT);
      obj->flt = tok->flt;
      return obj;
    case TOK_STRING:
      obj = obj_init(STRING);
      obj->string = tok->string;
      return obj;
    case TOK_SYMBOL:
      /*Check if symbol is a builtin*/
      for (int i = 0; i < BUILTIN_LEN; i++) {
        if (strcmp(tok->string, builtin_syms[i]) == 0)
          return builtins[i];
      }
      if (tok->string[1] != '\0' && tok->string[0] == '#') {
        if (tok->string[1] == 't')
          return CONST_TRUE;
        else if (tok->string[1] == 'f')
          return CONST_FALSE;
      }

      obj = obj_init(SYMBOL);
      obj->string = tok->string;
      return obj;
    case TOK_PAREN_OPEN:
      if (tok->next->type == TOK_PAREN_CLOSE)
        return EMPTY_LIST;
      obj = obj_init(LIST);
      obj->cell = token_to_cons(tok->next);
      return obj;
    case TOK_PAREN_CLOSE:
      list_end = tok;
      return NULL;
    case TOK_OPERATOR_PLUS:
      return builtins[OPERATOR(ADD)];
    case TOK_OPERATOR_MINUS:
      return builtins[OPERATOR(SUBTRACT)];
    case TOK_OPERATOR_MULTIPLY:
      return builtins[OPERATOR(MULTIPLY)];
    case TOK_OPERATOR_DIVIDE:
      return builtins[OPERATOR(DIVIDE)];
  }
}

object_t *tokens_to_obj()
{
  return token_to_obj(tokens);
}

void scan(char *str, size_t limit)
{
  char word[100];
  size_t word_index = 0;

  for (size_t i = 0; i < limit; i++) {
    switch(str[i]) {
      case ' ':
        if (str[i-1] == ' ' || head_tok->type == TOK_PAREN_CLOSE)
          continue;
        word[word_index] = '\0';
        add_token(word);
        word_index = 0;
        continue;
      case '(':
        paren_depth++;
        add_token("(");
        continue;
      case ')':
        paren_depth--;
        if (str[i-1] != ' ') {
          word[word_index] = '\0';
          if (word[0] != '\0')
            add_token(word);
          word_index = 0;
        }
        add_token(")");
        continue;
      case '\n':
        if (head_tok == NULL || head_tok->type != TOK_PAREN_CLOSE) {
          word[word_index] = '\0';
          add_token(word);
          word_index = 0;
          break;
        }
        break;
      default:
        if (str[i] == '"')
          nquotes++;
        word[word_index++] = str[i];
    }
  }
}

void clear_tokens()
{
  token_t *cur = tokens, *next;

  while (cur != NULL) {
#ifdef DEBUG
    printf("Cleared token\n");
#endif

    next = cur->next;
    free(cur);
    cur = next;
  }
  tokens = NULL;
  head_tok = NULL;
}
