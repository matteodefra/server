/** 
 * @file queue.c 
 * @author Matteo De Francesco 562176
 * 
 * Si dichiara che il contenuto di questo file e' in ogni sua parte opera
 * originale dell'autore
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "queue.h"
#include <pthread.h>
#include "util.h"

/**
 * Semaforo globale per consentire l'accesso in mutua 
 * esclusione ai thread che accedono alla lista 
*/
static pthread_mutex_t sem=PTHREAD_MUTEX_INITIALIZER;

/**
 * @function start_queue
 * @brief funzione di supporto ai thread main del server per inizializzare
 *        la lista
 * 
 * @param head  testa della lista
 * 
 * @returns NULL
 */
void start_queue(queue **head) {

  *head=NULL;
}

/**
 * @function push_queue
 * @brief funzione di supporto ai thread del server per salvare un client
 *        nella lista dei connessi
 *
 * @param head 	testa della lista
 *        client  nome del client da aggiungere alla lista
 * 
 * @returns -1 se il client è gia connesso ed è gia presente nella lista
 *          1 se il client viene inserito nella lista correttamente
 */
int push_queue(queue **head,char *client) {

  queue *new_client=Malloc(sizeof(queue));
  strcpy(new_client->nome_client,client);
  new_client->next=NULL;
  int r=check_queue(*head,client);
  if (r==0) return -1;
  
  pthread_mutex_lock(&sem);
  
  if (*head==NULL) {
    *head=new_client;
  }
  else {
    queue *tmp=*head;
    while (tmp->next!=NULL) {
      tmp=tmp->next;
    }
    tmp->next=new_client;
  }
  
  pthread_mutex_unlock(&sem);
  
  return 1;
}

/**
 * @function pop_queue
 * @brief funzione di supporto ai thread del server per rimuovere un 
 *        client dalla lista dei connessi
 *
 * @param head  testa della lista
 *        client  nome del client da rimuovere dalla lista
 * 
 * @returns 0 se la lista è vuota o se il client viene rimosso con successo
 *          1 se il client non è presente nella lista
 */
int pop_queue(queue **head,char *client) {
  
  pthread_mutex_lock(&sem);
  
  queue *tmp,*prev;
  if (*head==NULL) return 0;
  tmp=*head;
  
  if (strcmp((*head)->nome_client,client)==0) {
    *head=(*head)->next;
    free(tmp);
    pthread_mutex_unlock(&sem);
    return 1;
  }
  else {
    prev=(*head)->next;
    while (prev!=NULL) {
      if (strcmp(prev->nome_client,client)==0) {
        tmp->next=prev->next;
        free(prev);
        pthread_mutex_unlock(&sem);
        return 1;
      }
      else {
        tmp=tmp->next;
        prev=prev->next;
      }
    }
    pthread_mutex_unlock(&sem);
    return 0;
  }

  pthread_mutex_unlock(&sem);
  
  return 1;
}

/**
 * @function check_queue
 * @brief funzione di supporto alla push_queue per controllare che un client
 *        non sia gia presente nella lista dei connessi 
 *
 * @param head  testa della lista
 *        client  nome del client da controllare 
 * 
 * @returns 0 se il client è presente nella lista
 *          1 se il client non è presente nella lista
 */
int check_queue(queue *head,char *client) {

  pthread_mutex_lock(&sem);
  
  while (head!=NULL) {
    if (strcmp(head->nome_client,client)==0) {
      pthread_mutex_unlock(&sem);
      return 0;
    } 
    else head=head->next;
  }

  pthread_mutex_unlock(&sem);
  
  return 1;
}

/**
 * @function destroy_queue
 * @brief routine di pulizia, libero la memoria occupata dalla lista
 *
 * @param head  testa della lista
 * 
 * @returns NULL
 */
void destroy_queue(queue **head) {

  pthread_mutex_lock(&sem);
  
  if (*head==NULL) return;
  else {
    queue *tmp=*head;
    *head=(*head)->next;
    free(tmp);
    pthread_mutex_unlock(&sem);
    destroy_queue(head);
  }
}
