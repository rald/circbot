#ifndef TRIE_H
#define TRIE_H

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>


#define NUM_LETTERS 26

typedef struct trie_node TRIE_NODE;

struct trie_node {
  char let;
  bool mark;
  TRIE_NODE *next[NUM_LETTERS];
};

void trie_init(void);
void trie_walk(void);
void trie_add_word(char *word);

TRIE_NODE trie_root;
int trie_num_nodes;



#ifdef TRIE_IMPLEMENTATION


static void trie_init_node(TRIE_NODE *n)
{
  int i;

  n->let = '\0';
  n->mark = 0;
  for (i = 0; i < NUM_LETTERS; i++)
    n->next[i] = NULL;
}



static TRIE_NODE *trie_new_node(int let)
{
  TRIE_NODE *new_node;

  trie_num_nodes++;

  new_node = malloc(sizeof(TRIE_NODE));
  assert(new_node != NULL);

  trie_init_node(new_node);
  new_node->let = let;

  return new_node;
}



void trie_init()
{
  trie_init_node(&trie_root);
  trie_num_nodes = 0;
}



void trie_add_word(char *word)
{
  TRIE_NODE *curr_node = &trie_root;
  char *curr_let;
  int next_let;

  if (word == NULL || *word == '\0') {
    fprintf(stderr, "error: can't add null word to trie\n");
    return;
  }

  curr_let = word;

  while (*curr_let != '\0') {
    next_let = tolower(*curr_let) - 'a';

    if (next_let < 0 || next_let >= NUM_LETTERS) {
      fprintf(stderr, "error: can't add word '%s' (illegal chars)\n", word);
      return;
    }

    if (curr_node->next[next_let] == NULL)
      curr_node->next[next_let] = trie_new_node(next_let);

    curr_node = curr_node->next[next_let];
    curr_let++;
  }

  curr_node->mark = 1;
}



static char walk_buf[128];

static void trie_walk_node(TRIE_NODE *n, int depth)
{
  int i;

  depth++;

  if (n->mark) {
    walk_buf[depth] = '\0';
    puts(walk_buf);
  }

  for (i = 0; i < NUM_LETTERS; i++) {
    if (n->next[i] != NULL) {
      walk_buf[depth] = i + 'a';
      trie_walk_node(n->next[i], depth);
    }
  }
}



void trie_walk()
{
  trie_walk_node(&trie_root, -1);
}



#endif /* TRIE_IMPLEMENTATION */

#endif /* TRIE_H  */