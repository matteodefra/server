/** 
 * @file server.c
 * @brief Server per consentire le comunicazioni con i client
 *
 * @author Matteo De Francesco 562176
 *  
 * Si dichiara che il contenuto di questo file è in ogni sua parte
 * opera originale dell'autore
 * 	
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <pthread.h>
#include "server_library.h"
#include "queue.h"
#include "util.h"

//define per consentire lo swtich all'interno del thread
//per la corrispondente richiesta
#define BADKEY -1
#define REGISTER 1
#define STORE 2
#define RETRIEVE 3
#define DELETE 4
#define LEAVE 5

//struttura per effettuare lo switch nel singolo thread
typedef struct {
  char *key;
  int val;
} t_symstruct;

static t_symstruct lookuptable[] = {
  {"REGISTER",REGISTER},{"STORE",STORE},{"RETRIEVE",RETRIEVE},
  {"DELETE",DELETE},{"LEAVE",LEAVE}
};

//tutte le possibili richieste
#define NKEYS (sizeof(lookuptable)/sizeof(t_symstruct))

/**
 * @function keyfromstring
 * @brief riconosce la richiesta del client per entrare nello switch
 * 
 * @param key  richiesta del client
 * @returns una delle chiavi definite nelle define di sopra
 */
int keyfromstring(char *key) {
  int i;
  for (i=0; i<NKEYS; i++) {
    if (strcmp(lookuptable[i].key,key)==0) return lookuptable[i].val;
  }
  return BADKEY;
}

//variabile globale statica che conta il numero di client attivi 
static int numero_attivi=0;
//contiene tutto il path globale alla cartella data
char path_data[200];
//variabile volatile che viene modificata nell'handler dei segnali per terminare 
//i thread attivi e il main thread
volatile sig_atomic_t terminato=0;
//variabile volatile che viene modificata nell'handler dei segnali nel caso 
//di SIGUSR1 per stampare tutte le informazioni del server
volatile sig_atomic_t usr=0;
//variabile che contiene la dimensione dell'intero objstore, viene modificata 
//all'arrivo di un SIGUSR1 
long size_totale_store=0;
//variabile che contiene il numero di oggetti presenti nello store, viene
//modificata nella libreria del server nel momento di una store o una delete
long numero_oggetti_store=0;
//lista linkata che contiene tutti i client attualmente connessi
queue *head;
//socket del server e del client
int fd_sock,fd_client;
//lucchetto per consentire la modifica della variabile condivisa numero_attivi
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

/**
 * @function is_dot
 * @brief controlla che una directory non sia 
 *        la directory "." oppure ".."
 * 
 * @param path  nome della directory
 * 
 * @returns 1 se è "." o ".."
 *          0 altrimenti
 */
int is_dot(char *path) {
  int l=strlen(path);
  
  if (l>0 && path[l-1]=='.') return 1;
  return 0;
}

/**
 * @function get_size
 * @brief calcola ricorsivamente la dimensione dell'objstore quando
 *        arriva un segnale SIGUSR1
 * 
 * @param  name, path globale alla cartella data
 * 
 * @returns void
 */
void get_size(char *name) {
  DIR *data;
  struct dirent *info;
  if ((data=opendir(name))==NULL) {perror("in opendir get_size");return;}  
  while ((errno=0,info=readdir(data))!=NULL) {
    struct stat statbuf;
    char filename[MAXFILENAME];
    if ((strlen(name)+strlen(info->d_name)+2)>MAXFILENAME) {perror("MAXFILENAME troppo piccolo");return;}
    strncpy(filename,name,strlen(name)+1);
    strncat(filename,"/",2);
    strncat(filename,info->d_name,strlen(info->d_name)+1);
    if (stat(filename,&statbuf)==-1) {perror("in stat get_size");return;}
    if (S_ISDIR(statbuf.st_mode)) {
      if (!is_dot(filename))get_size(filename);
    }
    else size_totale_store+=statbuf.st_size;
  }
  if (errno!=0) {perror("in readdir di get_size");return;}  
  if (closedir(data)==-1) {perror("in closedir di get_size");return;}
}
 

/** 
 * Thread worker del server. Viene creato nel momento 
 * dell'accettazione di una connessione da parte del thread main,
 * riceve le richieste del client e restituisce una risposta a quest'ultimo.
 * Legge la richiesta del client fino al \n, dopodiche entra in uno 
 * dei casi di richiesta, nel caso della store, continua a leggere dal socket 
 * il dato di lunghezza che conosco dalla richiesta fino al \n.
 * In caso di ritorno -2 da parte delle funzioni di libreria, il client 
 * viene disconnesso e il thread viene chiuso 
*/
static void* worker(void *arg) {
  
  int fd=*(int*)arg;
  int r=1;
  int res;
  char nome_client[20];
  char directory_client[200];
  
  while (!terminato && r>0) {

    char vuoto;
    char *richiesta=Malloc(sizeof(char)*M);
    memset(richiesta,0,M);
    char answer[M];
    memset(answer,0,M);
    char *resto;
    int i=0,finish=0;
    /**
     * Lettura della richiesta fino al \n
     */
    while (i<M && !finish) {
      SYSCALLTHREAD(r,read(fd,&richiesta[i],1),"in lettura nel thread dal client");
      if (r==0) {
        if (pop_queue(&head, nome_client)==-1) perror("in pop_queue thread");
        close(fd);
        free(richiesta);
        free(arg);
        return (void*)-1;
      }
      if (richiesta[i]=='\n') finish=1;
      i++;
    }
    
    char *token=strtok_r(richiesta," \n",&resto);
    
    switch(keyfromstring(token)) {
      /**
       * Caso REGISTER: controlla che il client non sia presente nella coda,
       * se lo è chiude il thread e il client è gia connesso, altrimenti 
       * chiama la registra_nuovo_cliente
       */
      case REGISTER:
        token=strtok_r(NULL," \n",&resto);
        
        if (push_queue(&head,token)==-1) {
          strcpy(answer,"KO cliente gia connesso");
          SYSCALLTHREAD(r,write(fd,answer,strlen(answer)),"in scrittura thread caso 1,gia connesso");
          return (void*)-1;
        }

        else {
          strcpy(nome_client,token);
          strcpy(directory_client,token);
          
          pthread_mutex_lock(&mutex);
          numero_attivi++;
          pthread_mutex_unlock(&mutex);
          
          res=registra_nuovo_cliente(token);
          if (res==-2) {
            strcpy(answer,"KO impossibile registrarsi\n");
            r=0;
          }
          else strcpy(answer,"OK \n");
          
          SYSCALLTHREAD(r,write(fd,answer,strlen(answer)),"in scrittura thread caso 1,nuovo connesso");
        }
        
        break;
      /**
       * Caso STORE: legge il dato di lunghezza conosciuta dal socket
       * client, dopodiche chiama la salva_dato_cliente. Restituisce 
       * la risposta OK \n oppure KO message \n a seconda del valore di
       * ritorno della salva_dato_cliente
       */
      case STORE:
        SYSCALLTHREAD(r, read(fd, &vuoto, 1), "in lettura dello spazio");
        if (r==0) {
          pthread_mutex_lock(&mutex);
          numero_attivi--;
          pthread_mutex_unlock(&mutex);
          if (pop_queue(&head, nome_client)==-1) perror("in pop_queue thread");
          close(fd);
          free(arg);
          free(richiesta);
          return (void*)-1;
        }
        token=strtok_r(NULL," \n",&resto);
        char *nome_file=Malloc(sizeof(char)*(strlen(token)+1));
        strcpy(nome_file,token);
        token=strtok_r(NULL," \n",&resto);
        int len_dato=atoi(token);
        int appo=len_dato;
        void *dato_client=malloc(len_dato);

        while (appo>0) {
          SYSCALLTHREAD(r,read(fd,dato_client,appo),"in lettura del dato os_store");
          if (r==0) {
            if (pop_queue(&head, nome_client)==-1) perror("in pop_queue thread");
            SYSCALLTHREAD(r, close(fd), "in chiusura file descriptor client");
            free(richiesta);
            free(arg);
            return (void*)-1;
          }
          appo=appo-r;
        }
       
        res=salva_dato_cliente(nome_file,len_dato,dato_client,directory_client);
        free(nome_file);
        free(dato_client);
        if (res==1) strcpy(answer,"OK \n");
        else if (res==-1) strcpy(answer,"KO impossibile salvare \n");
        else {
          strcpy(answer,"KO errore nella memorizzazione");
        }
        
        SYSCALLTHREAD(r,write(fd,answer,strlen(answer)),"in scrittura thread caso 2");
        
        break;
      /**
       * Caso RETRIEVE: ottiene il dato dalla recupera_dato_cliente, se è NULL
       * manda la risposta al client di KO, altrimenti crea la stringa DATA len \n dato
       * di lunghezza conosciuta che manda al client, prima manda la stringa 
       * DATA len \n dopodiche manda il dato di tipo void*
       */
      case RETRIEVE:
        token=strtok_r(NULL," \n",&resto);
        void *dato_recuperato;
        dato_recuperato=recupera_dato_cliente(token,directory_client);
        if (dato_recuperato==(void*)-2) {
          strcpy(answer,"KO errore nel recupero\n");
          SYSCALLTHREAD(r,write(fd,answer,strlen(answer)),"in scrittura thread caso 3.1");
        }
        else if (dato_recuperato==NULL) {
          strcpy(answer,"KO dato non esistente \n");
          SYSCALLTHREAD(r,write(fd,answer,strlen(answer)),"in scrittura thread caso 3");
        }
        else {
          char lung[strlen(dato_recuperato)+1];
          sprintf(lung,"%ld",strlen(dato_recuperato));
          char risposta_dato[9+strlen(lung)];
          memset(risposta_dato,0,9+strlen(lung));
          strcpy(risposta_dato,"DATA ");
          strcat(risposta_dato,lung);
          strcat(risposta_dato," \n ");
          int len_risposta=strlen(risposta_dato);
          int appo=len_risposta;
          while (appo>0) {
            SYSCALLTHREAD(r,write(fd,&risposta_dato[len_risposta-appo],appo),"in scrittura thread caso 3");        
            if (r==0) {
              pthread_mutex_lock(&mutex);
              numero_attivi--;
              pthread_mutex_unlock(&mutex);
              if (pop_queue(&head, nome_client) == -1) perror("in pop_queue thread");
              close(fd);
              free(dato_recuperato);
              free(richiesta);
              free(arg);
              return (void *)-1;
            }
            appo=appo-r;
          }
          int left=strlen(dato_recuperato);
          while (left>0) {
            SYSCALL(r,write(fd,dato_recuperato,left),"in scrittura dato thread caso 3");
            if (r==0) {
              pthread_mutex_lock(&mutex);
              numero_attivi--;
              pthread_mutex_unlock(&mutex);
              if (pop_queue(&head, nome_client) == -1) perror("in pop_queue thread");
              close(fd);
              free(richiesta);
              free(dato_recuperato);
              free(arg);
              return (void *)-1;
            }
            left=left-r;
          }
        }
        free(dato_recuperato);
        break;
      /**
       * Caso DELETE: chiama la cancellazione_dato_cliente, se restituisce 0 manda 
       * il messaggio di KO, altrimenti manda il messaggio di OK al client
       */
      case DELETE:
        token=strtok_r(NULL," \n",&resto);
        res=cancellazione_dato_cliente(token,directory_client);
        
        if (res==0) {
          strcpy(answer,"KO dato non esistente \n");
        }
        else if (res==-2) {
          strcpy(answer,"KO errore in memorizzazione\n");
        } 
        else strcpy(answer,"OK \n");
        
        SYSCALLTHREAD(r,write(fd,answer,strlen(answer)),"in scrittura thread caso 4");
        
        break;
      /**
       * Caso LEAVE: manda il messaggio di OK al client e esce dal while del thread
       */
      case LEAVE:
        strcpy(answer,"OK \n");
        
        SYSCALLTHREAD(r,write(fd,answer,M),"in scrittura thread caso 5"); 
        r=0;
        break;
      case BADKEY:
        strcpy(answer,"KO opzione non valida\n");
        SYSCALLTHREAD(r,write(fd,answer,strlen(answer)),"in scrittua caso badkey");
        break;
    }
    free(richiesta);
  }
  /**
   * Dopo la leave decremento il numero dei clienti attivi, tolgo il client dalla lista
   * dopodiche chiudo la connessione con il client e chiudo il thread
   */
  pthread_mutex_lock(&mutex);
  numero_attivi--;
  pthread_mutex_unlock(&mutex);
  if (pop_queue(&head, nome_client) == -1) perror("in pop_queue thread");
  SYSCALLTHREAD(r, close(fd), "in chiusura file descriptor client");
  free(arg);
  return (void*)0;
}

/**
 * @function gestionesegnali
 * @brief gestore dei segnali SIGINT,SIGQUIT e SIGUSR1
 * nel caso di SIGINT e SIGQUIT il server e i thread vengono chiusi, 
 * nel caso di SIGUSR1 vengono stampate le informazioni sul server e 
 * ricomincia l'esecuzione
 * 
 * @param signum  tipo di segnale che arriva
 */
static void gestionesegnali(int signum) {
  if (signum==SIGINT) {
    write(1,"Server received signal SIGINT\n",30);
    terminato=1;
  }
  else if (signum==SIGQUIT) {
    write(1,"Server received signal SIGQUIT\n",31);
    terminato=1;
  }
  else if (signum==SIGTERM) {
    write(1,"Server received signal SIGTERM\n",31);
    terminato=1;
  }
  else if (signum==SIGTSTP) {
    write(1,"Server received signal SIGTSTP\n",31);
    terminato=1;
  }
  else if (signum==SIGUSR1) {
    usr=1;
  }
}

/* 
  Thread main server, crea il socket per le connessioni, crea 
  la cartella data principale dell'objstore, maschera il
  trattamento dei segnali, dopodiche aspetta connessioni sulla 
  accept. Quando arriva una connessione lancia il thread worker 
  che gestisce quel determinato cliente
*/
int main(void) {
  start_queue(&head);
  
  struct sockaddr_un sa;
  struct sigaction s;
  memset(&s,0,sizeof(s));
  s.sa_handler=gestionesegnali;
  int r;
  
  SYSCALL(r,mkdir("./data",0700),"nella creazione della directory principale");
  
  SYSCALL(r,sigaction(SIGINT,&s,NULL),"in sigaction SIGINT");
  SYSCALL(r,sigaction(SIGQUIT,&s,NULL),"in sigaction SIGQUIT");
  SYSCALL(r,sigaction(SIGTERM,&s,NULL),"in sigaction SIGTERM");
  SYSCALL(r,sigaction(SIGTSTP,&s,NULL),"in sigaction SIGTSTP");
  SYSCALL(r,sigaction(SIGUSR1,&s,NULL),"in sigaction SIGUSR1");
  
  strncpy(sa.sun_path, SOCKNAME,UNIX_PATH_MAX);
  sa.sun_family=AF_UNIX;
  SYSCALL(fd_sock,socket(AF_UNIX,SOCK_STREAM,0),"error in socket");
  SYSCALL(r,bind(fd_sock,(struct sockaddr *)&sa,sizeof(sa)),"in bind");
  SYSCALL(r,listen(fd_sock,SOMAXCONN),"in listen");
  
  SYSCALL(r,chdir("./data"),"in chdir della directory principale");
  
  if (getcwd(path_data,200)==NULL) {perror("in getcwd");exit(1);}
  pthread_t lavoratore;
  while (!terminato) {
    if (usr==1) {
      get_size(path_data);
      fprintf(stdout,"Numero connessi: %d\n",numero_attivi);
      fprintf(stdout,"Dimensione totale dello store: %ld\n",size_totale_store);
      fprintf(stdout,"Numero totale di oggetti: %ld\n",numero_oggetti_store); 
      size_totale_store=0;
      usr=0;
    }
    if ((fd_client=accept(fd_sock,NULL,0))<=0) {perror("aaaa");}
    else {  
      int *new_sock=Malloc(sizeof(int));
      *new_sock=fd_client;
      SYSCALL(r,pthread_create(&lavoratore,NULL,&worker,(void*)new_sock),"in pthread_create");
      pthread_detach(lavoratore);
    }
  }
  destroy_queue(&head);
  SYSCALL(r,close(fd_sock),"in chiusura socket server");
  return 0;
}
