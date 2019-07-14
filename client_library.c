/** 
 * @file client_library.c
 * @brief Libreria del client per consentire la comunicazione con il server
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
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include "util.h"
#include "client_library.h"



//Socket globale, consente la connessione di ogni singolo client al server
int fd_skt;

/**
 * @function os_connect
 * @brief funzione del client che consente la creazione del socket e la registrazione
 * 
 * @param name  nome del cliente che si vuole registrare
 * 
 * @returns 1 se la risposta del server è OK \n 
 *          -1 se la risposta del server è KO message \n
 */
int os_connect(char* name) {

  struct sockaddr_un sa;
  strncpy(sa.sun_path, SOCKNAME,UNIX_PATH_MAX);
  sa.sun_family=AF_UNIX;
  int r;
  
  SYSCALLCLIENT(fd_skt,socket(AF_UNIX,SOCK_STREAM,0),"in socket os_connect");
  
  while ( connect(fd_skt,(struct sockaddr *)&sa,sizeof(sa))==-1 ) {
    if (errno!=ENOENT) exit(1);
  }
  
  char concat[12+strlen(name)];
  strcpy(concat,"REGISTER ");
  strcat(concat,name);
  strcat(concat," \n");
  char answer[M];
  
  SYSCALLCLIENT(r,write(fd_skt,concat,strlen(concat)),"in scrittura os_connect");
  if (r==0) {
    perror("Server disconnesso\n");
    exit(1);
  }
  
  SYSCALLCLIENT(r,read(fd_skt,answer,M),"in lettura os_connect");
  if (r==0) {
    perror("Server disconnesso\n");
    exit(1);
  }
  
  char *token=strtok(answer," ");
  
  if (strcmp(token,"KO")==0) return 0;
  else return 1;
}

/**
 * @function os_store
 * @brief funzione del client che consente il salvataggio di un dato 
 *        nel suo storage personale
 * 
 * @param name   nome del file da salvare
 *        block  puntatore al blocco di dati da memorizzare  
 * 				len 	 lunghezza del blocco di memoria da memorizzare
 * 
 * @returns 1 se la risposta del server è OK \n 
 *          -1 se la risposta del server è KO message \n	
 */
int os_store(char* name,void* block,size_t len){

  int r;
  char lung[len+1];
  sprintf(lung,"%ld",len);
  char concat[11+strlen(name)+strlen(lung)];
  strcpy(concat,"STORE ");
  strcat(concat,name);
  strcat(concat," ");
  strcat(concat,lung);
  strcat(concat," \n ");

  char answer[M];

  int len_domanda=strlen(concat);
  int appo=len_domanda;
  while (appo>0) {
    SYSCALLCLIENT(r,write(fd_skt,&concat[len_domanda-appo],appo),"in scrittura della domanda os_store");
    if (r==0) {
      perror("Server disconnesso\n");
      exit(1);
    }
    appo=appo-r;
  }

  int left=len;
  while (left>0) {
    SYSCALLCLIENT(r,write(fd_skt,block,left),"in scrittura del dato os_store");
    if (r==0) {
      perror("Server disconnesso\n");
      exit(1);
    }
    left=left-r;
  }
  

  SYSCALLCLIENT(r,read(fd_skt,answer,M),"in lettura os_store");
  if (r==0) {
    perror("Server disconnesso\n"),
    exit(1);
  }

  char *token=strtok(answer," ");
  
  if (strcmp(token,"KO")==0) return 0;
  else return 1;
}

/**
 * @function os_retrieve
 * @brief funzione del client che consente il recupero di un dato 
 *        dal suo storage personale
 * 
 * @param name   nome del file da salvare
 * 
 * @returns ritorna il dato se la risposta del server è stata DATA len \n dato
 *          altrimenti ritorna NULL se ha risposto KO \n	
 */
void *os_retrieve(char* name) {
 
  int r;
  char concat[12+strlen(name)];
  strcpy(concat,"RETRIEVE ");
  strcat(concat,name);
  strcat(concat," \n");
  char answer[M];
  char vuoto;

  SYSCALLCLIENT(r,write(fd_skt,concat,strlen(concat)),"in scrittura os_retrieve");
  if (r==0) {
    perror("Server disconnesso\n");
    exit(1);
  }
  
  int i=0,finish=0;
  while (i<M && !finish) {
    SYSCALLCLIENT(r,read(fd_skt,&answer[i],1),"in lettura os_retrieve");
    if (r==0) { 
      perror("Server disconnesso\n");
      exit(1);
    }
    if (answer[i]=='\n') finish=1;
    i++;
  }
  
  char *token=strtok(answer," \n");
  
  if (strcmp(token,"KO")==0) return NULL;
  
  
  else {
    SYSCALLCLIENT(r,read(fd_skt,&vuoto,1),"in lettura os_retrieve carattere vuoto");
    token=strtok(NULL," \n");
    int len_dato=atoi(token);
    int appo=len_dato;
    void *val=Malloc(len_dato);
    while (appo>0) {
      SYSCALLCLIENT(r,read(fd_skt,val,appo),"in lettura dato os_retrieve");
      appo-=r;
    }
    return val;
  }
}

/** 
 * @function os_delete
 * @brief funzione del client che consente la cancellazione di un dato 
 *        dal suo storage personale
 * 
 * @param name   nome del file da cancellare
 * 
 * @returns ritorna 1 se la risposta del server è stata OK \n 
 *          oppure -1 se la risposta del server è stata KO message \n	
 */
int os_delete(char* name){

  int r;
  char concat[10+strlen(name)];
  char answer[M];
  strcpy(concat,"DELETE ");
  strcat(concat,name);
  strcat(concat," \n");
  
  SYSCALLCLIENT(r,write(fd_skt,concat,strlen(concat)),"in scrittura os_delete");
  if (r==0) {
    perror("Server disconnesso\n");
    exit(1);
  }
  
  SYSCALLCLIENT(r,read(fd_skt,answer,M),"in lettura os_delete");
  if (r==0) {
    perror("Server disconnesso\n");
    exit(1);
  }
  char *token=strtok(answer," ");
  
  if (strcmp(token,"KO")==0) return 0;
  
  else return 1;
}

/** 
 * @function os_disconnect
 * @brief funzione del client che consente la disconnessione 
 * 				di un client
 * 
 * @param NULL
 * 
 * @returns 1 la risposta del server è OK \n	
 */
int os_disconnect() {
  
  int r;
  char concat[8];
  memset(concat,0,8);
  char answer[5];
  strcpy(concat,"LEAVE \n");
  
  SYSCALLCLIENT(r,write(fd_skt,concat,strlen(concat)),"in scrittura os_disconnect");
  if (r==0) {
    perror("Server disconnesso\n");
    return 0;
  }
    
  SYSCALLCLIENT(r,read(fd_skt,answer,4),"in lettura os_disconnect");
  if (r==0) {
    perror("Server disconnesso\n");
    return 0;
  }
  return 1;
}
