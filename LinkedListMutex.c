#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t head_p_mutex = PTHREAD_MUTEX_INITIALIZER;

struct list_node_s{
  int data;
  struct list_node_s* next;
  pthread_mutex_t mutex;
};

int Member(int value, struct list_node_s* head_p) {
  struct list_node_s* temp_p;
  pthread_mutex_lock(&head_p_mutex);
  temp_p = head_p;
  while(temp_p != NULL && temp_p->data < value) {
    if(temp_p->next != NULL) { 
      pthread_mutex_lock(&(temp_p->next->mutex));
    }
    if(temp_p == head_p) {
      pthread_mutex_unlock(&head_p_mutex);
    }
    pthread_mutex_unlock(&(temp_p->mutex));
    temp_p = temp_p->next;
  }
  if(temp_p == NULL || temp_p->data > value){
    if(temp_p == head_p) {
      pthread_mutex_unlock(&head_p_mutex);
    }
    if(temp_p != NULL) {
      pthread_mutex_unlock(&(temp_p->mutex));
    }
    return 0;
  }
  else {
    if(temp_p == head_p) {
      pthread_mutex_unlock(&head_p_mutex);
    }
    pthread_mutex_unlock(&(temp_p->mutex));
    return 1;
  }
}

int Insert(int value, struct list_node_s** head_p){
  pthread_mutex_lock(&head_p_mutex);
  int flag = 0;
  struct list_node_s* curr_p = *head_p;
  struct list_node_s* pred_p = NULL;
  struct list_node_s* temp_p;  
  while(curr_p != NULL && curr_p->data < value){
    if(curr_p->next != NULL) pthread_mutex_lock(&(curr_p->next->mutex));
    if(curr_p == *head_p) pthread_mutex_unlock(&head_p_mutex);
    pthread_mutex_unlock(&curr_p->mutex);
    pred_p = curr_p;
    curr_p = curr_p->next;
  }
  if(curr_p == NULL || curr_p->data > value){
    temp_p = malloc(sizeof(struct list_node_s));   
    temp_p->data = value;
    temp_p->next = curr_p;
    if(curr_p == *head_p) flag = 1;
    if(pred_p == NULL)
      *head_p = temp_p;
    else pred_p->next = temp_p;
    if(curr_p != NULL) pthread_mutex_unlock(&curr_p->mutex);
    if(flag==1)  pthread_mutex_unlock(&head_p_mutex);
    return 1;
  }
  pthread_mutex_unlock(&curr_p->mutex);
  if(curr_p == *head_p) pthread_mutex_unlock(&head_p_mutex);
  else return 0;
}

int Delete(int value, struct list_node_s** head_p){
  pthread_mutex_lock(&head_p_mutex);
  struct list_node_s* curr_p = *head_p;
  struct list_node_s* pred_p = NULL;
  while(curr_p != NULL && curr_p->data < value){
    if(curr_p->next != NULL) pthread_mutex_lock(&(curr_p->next->mutex));
    if(curr_p == *head_p) pthread_mutex_unlock(&head_p_mutex);
    pthread_mutex_unlock(&curr_p->mutex);
    pred_p = curr_p;
    curr_p = curr_p->next;
  }
  if(curr_p != NULL && curr_p->data == value){
    if(pred_p == NULL){
      *head_p = curr_p->next;
      pthread_mutex_unlock(&curr_p->mutex);
      pthread_mutex_unlock(&head_p_mutex);
      free(curr_p);
    }
    else{
      pred_p->next = curr_p->next;
      pthread_mutex_unlock(&curr_p->mutex);
      free(curr_p);
    }
    return 1;
  }
  else{
    if(curr_p != NULL) pthread_mutex_unlock(&curr_p->mutex);
    if(curr_p == *head_p) pthread_mutex_unlock(&head_p_mutex);
    return 0;
  }
}

struct list_node_s* list = NULL;

void N_Insert(double ops){
  double i;
  for(i = 0; i < ops; i++) {
    Insert(1000,&list);
  }
  return;
}

void N_Member(double ops){
  double i;
  for(i = 0; i < ops; i++) {
    Member(1000,list);
  }
  return;
}

void N_Delete(double ops){
  double i;
  for(i = 0; i < ops; i++) {
    Delete(1000,&list);
  }
  return;

}

double members, inserts, deletes;

void* N_All(void* rank){
  long my_rank = (long) rank;
  //printf("%ld %lf %lf %lf\n", my_rank, members, inserts, deletes);
  N_Member(members);
  N_Insert(inserts);
  N_Delete(deletes);
  return NULL;
}

int main(int argc, char* argv[]){
  struct list_node_s* list = NULL;
  double thread_count = strtol(argv[1],NULL,10);
  pthread_t* thread_handles = malloc(thread_count* sizeof(pthread_t));
  scanf("%lf %lf %lf", &members, &inserts, &deletes);
  members = (members * 1000.0)/thread_count;
  inserts = (inserts * 1000.0)/thread_count;
  deletes = (deletes * 1000.0)/thread_count;
  for(double i = 0; i < 1000; i++){
    Insert(i,&list);
  }  
  for(long thread = 0; thread < thread_count; thread++) {
    pthread_create(&thread_handles[thread], NULL, N_All, (void*) thread);
  }
  for(long thread = 0; thread < thread_count; thread++) {
    pthread_join(thread_handles[thread], NULL);
  }
  return 0;
}