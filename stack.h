#include <stdio.h>
#include <stdlib.h>


typedef struct stack{
  int offs;
  struct stack *next;
}stack_t;
stack_t *New_Stack(void);
void push(int x, stack_t **head);
int isEmpty(stack_t **head);
unsigned  pop(stack_t **head);
void top(stack_t **head);
int Stack_Size(stack_t *head);
void Stack_Printing(stack_t **head);
stack_t* Stack_Merging(stack_t *t1, stack_t *t2);
