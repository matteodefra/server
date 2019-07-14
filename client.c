/** 
 * @file client.c 
 * @author Matteo De Francesco 562176
 * 
 * Si dichiara che il contenuto di questo file e' in ogni sua parte opera
 * originale dell'autore
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
#include "client_library.h"
#include "util.h"
#define MAXLEN 100000
#define MINLEN 100
#define SUM 4995


int numero_operazioni_effettuate=0;
int numero_operazioni_successo=0;
int numero_operazioni_fallite=0;


int main(int argc, char const *argv[]) {

  if (argc!=3) {
    perror("Almeno due argomenti");
    exit(EXIT_FAILURE);
  }


  int r;
  //per generare l'elemento della stringa da mandare
  char c;
  //per recuperare il dato nella retrieve
  void *dato;
  
  char *nome_file=Malloc(sizeof(char)*7);
  char *cliente=(char*)argv[1];
  int case_test=atoi(argv[2]);

  r=os_connect(cliente);
  numero_operazioni_effettuate++;
  if (r==0) { 
    numero_operazioni_fallite++;
    fprintf(stdout,"Gia connesso:%s\nEffettuate:%d\nSuccesso:%d\nFallite:%d\n",
    cliente,numero_operazioni_effettuate,numero_operazioni_successo,numero_operazioni_fallite);
    //free(cliente);
    free(nome_file);
    return 0;
  }
  numero_operazioni_successo++;
    
  switch (case_test) {
    
    case 1:
      /**
       * Genero stringhe con contenuto crescente da 100 a 100000 byte con 
       * nome del file uguale al numero dei byte mandati
       */
      for (int i=0; i<=19; i++) {
        char *s=Malloc(sizeof(char)*(MINLEN+(i*SUM)+1));
        memset(s,0,MINLEN+(i*SUM)+1);
        c='0'+(i%10);
        sprintf(nome_file,"%d",MINLEN+(i*SUM));
        for (size_t z=0; z<MINLEN+(i*SUM); z++) {
          s[z]=c;
        }
        r=os_store(nome_file,(void*)s,MINLEN+(i*SUM));
        numero_operazioni_effettuate++;
        if (r==0) { 
          fprintf(stdout,"client:%s-%d",cliente,case_test);
          numero_operazioni_fallite++;
        }
        else numero_operazioni_successo++;
        free(s);
      }

      break;
    case 2:
        /**
         * Recupero un dato dall'objstore
         */
        dato=os_retrieve("15085");
        numero_operazioni_effettuate++;
        if (dato==NULL || strlen(dato)!=15085) { 
          fprintf(stdout,"client:%s-%d\n",cliente,case_test);
          numero_operazioni_fallite++;
          free(dato);
        }
        else { 
          numero_operazioni_successo++;
          free(dato);
        }
      break;
    
    case 3:
        /**
         * Elimino un dato dall'objstore
         */
        r=os_delete("100");
        numero_operazioni_effettuate++;
        if (r==0) { 
          fprintf(stdout,"client:%s-%d\n",cliente,case_test);
          numero_operazioni_fallite++;
        }
        else { 
          numero_operazioni_successo++;
        }
      break;

    default:
      fprintf(stdout,"Opzione di test non valida, usare solo 1,2 o 3\n");
      break;
  }
  
  //free(cliente);

  //Disconnetto il cliente dal server
  r=os_disconnect();
  numero_operazioni_effettuate++;
  if (r==0) numero_operazioni_fallite++;
  else numero_operazioni_successo++;
  free(nome_file);

  fprintf(stdout,"Effettuate:%d\nSuccesso:%d\nFallite:%d\n",numero_operazioni_effettuate,numero_operazioni_successo,numero_operazioni_fallite);

  return 0;
}
