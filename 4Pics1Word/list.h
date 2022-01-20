#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct List List;

struct List {
	void *data;
	List *next;
};

List *List_New(void *data, List * next);
void List_Delete(List *head,void(*Free)(void*));
int List_Count(List * head);
List *List_PushFront(List ** head, void *data);
List *List_PushBack(List ** head, void *data);
List *List_RemoveAt(List ** head, int index);
List *List_Get(List * head, int index);

#endif
