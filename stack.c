#include <stdio.h>
#include <stdlib.h>
#include "stack.h"





stack_t *New_Stack(void){
  stack_t *n_node = (stack_t *)malloc(sizeof(stack_t ));
  n_node->offs = 0;
  n_node->next =  NULL;
 
  return n_node;
}

void push(int x, stack_t **head){
  
  stack_t *n_node = (stack_t *)malloc(sizeof(stack_t ));
  n_node->offs = x;
  n_node->next = NULL;
  if(n_node == NULL)
  {printf("Not enough memory\n");return;}
  if( (n_node != NULL) && (*head ==NULL) && ( Stack_Size(*head) ==1))
  { 
	  (*head)->offs=x;
	  (*head)->next= NULL;
  }
  else if((n_node != NULL) &&  (*head !=NULL ) &&  ((*head)->offs ==0 ))
  { 
	  (*head)->offs=x;  
	  (*head)->next= NULL;
  }
  else
  {
     n_node->next = (*head);  
     (*head) = n_node;
  }
}

int isEmpty(stack_t **head){
  
    if(Stack_Size(*head) == 0)
      return 1;
  
  return 0;
}

unsigned  pop(stack_t **head){
  stack_t *temp =(stack_t*)malloc(sizeof(stack_t ));
  temp = *head;
  unsigned number;
  if(isEmpty(head) == 1){
      
     printf("Cannot pop an element the stack is empty!!!!\n");
     return 0;
  }
  else{
    number = temp->offs;
    temp = (*head)->next;
    free(*head);
    *head = temp;
    return number;
  }
}

void top(stack_t **head){
  if(isEmpty(head) == 1)
    printf("The stack is empty\n");
  else
    printf("The last element of the stack is %d\n", (*head)->offs);

}

int Stack_Size(stack_t *head){
  int counter  = 0;
  
  
  stack_t *temp = (stack_t *)malloc(sizeof(stack_t));
  if (head != NULL)
  { 
    temp = (head);

    for(;temp != NULL;temp= temp->next){
      counter++;
      //printf("\nCOUNTING THE %d", counter);
    }

  }

  //printf("THE TOTAL COUNTER IS %d \n",counter);
  return counter;
}


void Stack_Printing(stack_t **head){
  stack_t *temp = (stack_t *)malloc(sizeof(stack_t));
  
  if(*head == NULL ){
    return;
  }
  
  if(*head != NULL ){
    temp = (*head);
    printf("The element is %d\n", temp->offs);
    while(temp!= NULL){
      temp= temp->next;
    
    }
    
  }
}


stack_t * Stack_Merging(stack_t *t1, stack_t *t2){
  stack_t *temp = (stack_t *)malloc(sizeof(stack_t));
  
  

  if(isEmpty(&t1)){
    if(isEmpty(&t2))
    {
      return New_Stack();
    }
  }
  if(isEmpty(&t1)){
    if(!isEmpty(&t2))
    {
      return  t2;
    }
  }
  if(!isEmpty(&t1)){
    if(isEmpty(&t2))
    {
      return t1;
    }
  }
  if((t1) != NULL && (t2) != NULL){
     
     
    if(Stack_Size(t1) >= Stack_Size(t2)){
     
      temp = (t2);
      for(;(temp)->next != NULL;temp=(temp)->next){}
     
      (temp)->next = (t1);
      
      return ( t2);
    }
    else{
      temp = (t1);
      for(;(temp)->next != NULL;temp =(temp)->next){}
     
      (temp)->next = (t2);

     
      return ( t1)->next;
    }
  }
}


