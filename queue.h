/** 
 * @file queue.h
 * @brief Interfaccia per le funzioni della lista dei connessi
 *
 * @author Matteo De Francesco 562176
 *  
 * Si dichiara che il contenuto di questo file è in ogni sua parte
 * opera originale dell'autore
 * 	
 */

//Lista linkata per consentire la memorizzazione dei nomi
//dei client connessi in un dato momento 
typedef struct queue_t {
  char nome_client[20];
  struct queue_t * next;
} queue;

/**
 * @function start_queue
 * @brief funzione di supporto ai thread main del server per inizializzare
 *        la lista
 * 
 * @param head  testa della lista
 * 
 * @returns NULL
 */
void start_queue(queue **head);

/**
 * @function push_queue
 * @brief funzione di supporto ai thread del server per salvare un client
 *        nella lista dei connessi
 *
 * @param head 	testa della lista
 *        client  nome del client da aggiungere alla lista
 * 
 * @returns intero che dice se è l'aggiunta è andata a buon fine o meno
 */
int push_queue(queue **head,char *client);

/**
 * @function pop_queue
 * @brief funzione di supporto ai thread del server per rimuovere un 
 *        client dalla lista dei connessi
 *
 * @param head  testa della lista
 *        client  nome del client da rimuovere dalla lista
 * 
 * @returns intero che dice se la rimozione è andata a buon fine o meno
 */
int pop_queue(queue **head,char *client);

/**
 * @function check_queue
 * @brief funzione di supporto alla push_queue per controllare che un client
 *        non sia gia presente nella lista dei connessi 
 *
 * @param head  testa della lista
 *        client  nome del client da controllare 
 * 
 * @returns intero che dice se il client è presente o meno
 */
int check_queue(queue *head,char *client);

/**
 * @function destroy_queue
 * @brief routine di pulizia, libero la memoria occupata dalla lista
 *
 * @param head  testa della lista
 * 
 * @returns NULL
 */
void destroy_queue(queue **head);

