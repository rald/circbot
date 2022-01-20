#include "list.h"

List *List_New(void *data, List * next)
{
	List *list = malloc(sizeof(List));
	if (list) {
		list->data = data;
		list->next = next;
	}
	return list;
}

void List_Delete(List *head,void(*Free)(void*)) {
	int i;
	int n=List_Count(head);
	for(i=0;i<n;i++) {
		Free(head->data);
		head->data=NULL;
	}
	free(head);
	head=NULL;
}

int List_Count(List * head)
{
	int i = 0;
	List *iter=head;
	while (iter) {
		i++;
		iter = iter->next;
	}
	return i;
}

List *List_PushFront(List ** head, void *data)
{
	return ((*head) = List_New(data, *head));
}

List *List_PushBack(List ** head, void *data)
{
	List *node=List_New(data,NULL);
	List *last=*head;
	if(!(*head)) {
		(*head)=node;
		return (*head);
	}

	while(last->next) {
		last=last->next;
	}

	last->next=node;

	return (*head);
}

List *List_RemoveAt(List ** head, int index)
{
	List *temp = *head;
	int i;

	if (!(*head))
		return NULL;

	if (index == 0) {
		(*head) = temp->next;
		temp->next = NULL;
		return temp;
	}

	for (i = 0; temp && i < index - 1; i++) {
		temp = temp->next;
	}

	if (!temp || !temp->next)
		return NULL;

	List *next = temp->next->next;
	List *curr = temp->next;
	temp->next = next;
	curr->next = NULL;
	return curr;
}

List *List_Get(List * head, int index)
{
	int i=0;
	List *curr=head;
	while(curr && i<index) {
		i++;
		curr=curr->next;
	}
	return curr;
}
