/*
 * queue.c
 * Implementation of a standard generalized queue
 *
 * Author: Thomas Kiernan
 */

 #include <stdio.h>
 #include <stdlib.h>

 #define TRUE 1
 #define FALSE 0

 typedef struct queue_node queue_node;
 struct queue_node {
   queue_node* next;
   void* data;
 };

 typedef struct queue_t {
   queue_node* head;
   queue_node* tail;
 } queue_t;


 /* create an empty queue */
 queue_t* qopen(void){
   queue_t* queue = malloc(sizeof(queue_t));
   queue->head = NULL;
   queue->tail = NULL;
   return queue;
 }

 /* deallocate a queue, frees everything in it */
 void qclose(queue_t *qp){
   queue_node* node = qp->head;
   while(node){
     queue_node* next = node->next;
     free(node->data);
     free(node);
     node = next;
   }
   free(qp);
 }

 /* put element at end of queue */
 int qput(queue_t *qp, void *elementp){
   if(!qp->head){
     queue_node* node = malloc(sizeof(queue_node));
     node->data = elementp;
     node->next = NULL;

     qp->head = node;
     qp->tail = node;
   } else {
     queue_node* node = malloc(sizeof(queue_node));
     node->data = elementp;
     node->next = NULL;

     qp->tail->next = node;
     qp->tail = node;
   }
   return 0;
 }

 /* get first element from queue */
 void* qget(queue_t *qp){
   if(qp->head){
     queue_node* node = qp->head;
     void* data = node->data;

     qp->head = qp->head->next;
     free(node);

     return data;
   }
   return NULL;
 }

 /* apply a void function to every element of a queue */
 void qapply(queue_t *qp, void (*fn)(void* elementp)){
   queue_node* node = qp->head;
   while(node){
     fn(node->data);
     node = node->next;
   }
 }

 /* search a queue using a supplied boolean function, returns an element */
 void* qsearch(queue_t *qp, int (*searchfn)(void* elementp,const void* keyp), const void* skeyp){
   queue_node* node = qp->head;
   while(node){
     if(searchfn(node->data, skeyp) == 1){
       return node->data;
     }
     node = node->next;
   }
   return NULL;
 }

 /* search a queue using a supplied boolean function, removes and
  * returns the element
  */
 void* qremove(queue_t *qp, int (*searchfn)(void* elementp,const void* keyp), const void* skeyp){
   queue_node* node = qp->head;
   /* Empty Queue */
   if(!node){
     return NULL;
   }
   /* Head is found */
   if(searchfn(node->data, skeyp)){
     /* Only Element */
     if(qp->head == qp->tail){
       void* data = qp->head->data;
       free(qp->head);
       qp->head = NULL;
       qp->tail = NULL;
       return data;
     }
     /* Not the only */
     else {
       queue_node* node = qp->head;
       void* data = node->data;
       qp->head = node->next;
       free(node);
       return data;
     }
   }
   /* Search the rest of the queue */
   while(node->next){
     if(searchfn(node->next->data, skeyp)){
       queue_node* found = node->next;
       void* data  = found->data;

       node->next = found->next;

       if(qp->tail == found){
         qp->tail = node;
       }
       free(found);
       return data;
     }
     node = node->next;
   }

   return NULL;
 }

 /* concatenatenates elements of q2 into q1, q2 is dealocated upon completion */
 void qconcat(queue_t *q1p, queue_t *q2p){
   /* Niether Queue Empty */
   if(q1p->head && q2p->head){
     q1p->tail->next = q2p->head;
     q1p->tail = q2p->tail;
     free(q2p);
   }
   /* Q1 Nonempty, Q2 empty */
   else if(q1p->head && !q2p->head){
     free(q2p);
   }
   /* Q1 Empty, Q2 Nonempty */
   else if(!q1p->head && q2p->head){
     q1p->head = q2p->head;
     q1p->tail = q2p->tail;
     free(q2p);
   }
   /* Both Empty */
   else if (!q1p->head && !q2p->head){
     free(q2p);
   }
 }
