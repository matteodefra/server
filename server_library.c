/** 
 * @file server_library.c 
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
#include "util.h"

//variabile long per contare il numero di oggetti presenti nello storage
extern long numero_oggetti_store;

//semaforo per garantire la mutua esclusione quando viene modificata la
//variabile condivisa numero_oggetti_store
pthread_mutex_t oggetti=PTHREAD_MUTEX_INITIALIZER;

/**
 * @function registra_nuovo_cliente
 * @brief funzione di supporto ai thread del server per registrare
 *				un nuovo cliente(creazione cartella) o connetterlo 
 *
 * @param nome  	nome del cliente da registrare
 * 
 * @returns 1 se il cliente si è connesso per la prima volta
 * 				e viene creata la directory
 * 			0 se il cliente si è connesso e la sua directory
 * 				è gia presente
 * 			-2 se c'è stato un errore nelle system call 
 */
int registra_nuovo_cliente(char *nome) {
  DIR *client;
  
	if ((client=opendir(nome))==NULL) {
		if (mkdir(nome,0700)==-1) {perror("in mkdir registra_nuovo_cliente");return -2;}
		return 1;
	}
	
	else {
		if (closedir(client)==-1) {perror("in closedir registra_nuovo_cliente");return -2;}
		return 0;
	}
}

/**
 * @function salva_dato_cliente
 * @brief funzione di supporto ai thread del server per salvare
 * 				un dato nel suo storage personale
 * 
 * @param nomefile	nome del file da memorizzare
 * 				lunghezza		lunghezza del dato che viene mandato dal client
 * 				dato			puntatore al blocco di memoria da salvare nello storage
 * 				path			nome del client relativo al thread
 * 
 * @returns 1 se il file è stato salvato correttamente o è stato sovrascritto
 * 					-1 se non è stato possibile salvare il file
 * 					-2 se c'è stato un errore nelle system call	
 */
int salva_dato_cliente(char *nomefile,int lunghezza,void *dato,char *dir_client) {
	int r;
	int ifp;
	DIR *client;
	struct dirent *contenuto;
	
	char path_file[strlen(nomefile)+strlen(dir_client)+2];
	strcpy(path_file,dir_client);
	strcat(path_file,"/");
	strcat(path_file,nomefile);
	
	if ((client=opendir(dir_client))==NULL) {perror("in opendir salva_dato_cliente");return -2;}
	
	while ((errno=0,contenuto=readdir(client))!=NULL) {
		if (strcmp(contenuto->d_name,nomefile)==0) {
			SYSCALLSERVERLIB(r,unlink(path_file),"in unlink di delete");
			pthread_mutex_lock(&oggetti);
			numero_oggetti_store--;
			pthread_mutex_unlock(&oggetti);
		}
	}
	
	if (errno!=0) {perror("in readdir di salva_dato_cliente");return -2;}
	
	else {
		if ((ifp=open(path_file,O_CREAT|O_RDWR,0666))==-1) {perror("in creazione file salva_dato_cliente");return -2;}

		int appo=lunghezza;
		while (appo>0) {
			SYSCALLSERVERLIB(r,write(ifp,dato,appo),"in scrittura sul file in salva_dato_cliente");
			appo=appo-r;
		}
		pthread_mutex_lock(&oggetti);
		numero_oggetti_store++;
		pthread_mutex_unlock(&oggetti);
		SYSCALLSERVERLIB(r,close(ifp),"in chiusura file salva_dato_cliente");
		SYSCALLSERVERLIB(r,closedir(client),"in chiusura directory salva_dato_cliente");
		return 1;
	}
	return -1;
}

/**
 * @function recupera_dato_cliente
 * @brief funzione di supporto ai thread del server per recuperare 
 * 				un dato di un cliente dal suo storage personale
 * 
 * @param nomefile	nome del file da recuperare
 * 		  	path			nome del client relativo al thread
 * 
 * @returns puntatore al dato recuperato, altrimenti NULL se il dato
 * 					non è presente o ci sono stati errori nelle system call
 */
void *recupera_dato_cliente(char *nomefile,char *dir_client) {
	int r;
	char path_file[strlen(nomefile)+strlen(dir_client)+2];
	DIR *client;
	struct dirent *contenuto;
	struct stat info_file;

	strcpy(path_file,dir_client);
	strcat(path_file,"/");
	strcat(path_file,nomefile);
	
	if ((client=opendir(dir_client))==NULL) {perror("in opendir di recupera_dato_cliente");return (void*)-2;}
	
	while ((errno=0,contenuto=readdir(client))!=NULL) {
		if (strcmp(contenuto->d_name,nomefile)==0) {
			int ifp;
			CALL(ifp,open(path_file,O_RDWR),"in apertura file recupera_dato_cliente");
			CALL(r,fstat(ifp,&info_file),"in fstat di recupera_dato_cliente");
			long grandezza=info_file.st_size;
			void *dato=Malloc(grandezza+1);
			memset(dato,0,grandezza+1);

			int appo=grandezza;
			while (appo>0) {
				CALL(r,read(ifp,dato,grandezza),"in lettura del dato dal file");
				appo=appo-r;
			}
			CALL(r,close(ifp),"in chiusura file recupera_dato_cliente");
			CALL(r,closedir(client),"in closedir directory recupera_dato_cliente");
			return dato;
		}
	}
	if (errno!=0) {perror("in readdir di recupera_dato_cliente");return (void*)-2;}
	else {
		CALL(r,closedir(client),"in closedir directory recupera_dato_cliente");
		return NULL;
	}
}

/**
 * @function cancellazione_dato_cliente
 * @brief funzione di supporto ai thread del server per cancellare
 * 				un dato di un cliente dal suo storage personale
 * 
 * @param nomefile	nome del file da cancellare
 * 				path			nome del client relativo al thread
 * 
 * @returns	1 se il dato è stato cancellato con successo
 * 					0 se il dato non è presente nella directory
 * 					-2 se c'è stato un errore nelle system call
 */
int cancellazione_dato_cliente(char *nomefile,char *dir_client) {
	int r;
	DIR *client;
	struct dirent *contenuto;
	char path_file[strlen(nomefile)+strlen(dir_client)+2];
	strcpy(path_file,dir_client);
	strcat(path_file,"/");
	strcat(path_file,nomefile);
	
	if ((client=opendir(dir_client))==NULL) {perror("in opendir di cancellazione_dato_cliente");return -2;}
	
	while ((errno=0,contenuto=readdir(client))!=NULL) {
		if (strcmp(contenuto->d_name,nomefile)==0) {
			SYSCALLSERVERLIB(r,unlink(path_file),"in unlink di delete");
			pthread_mutex_lock(&oggetti);
			numero_oggetti_store--;
			pthread_mutex_unlock(&oggetti);
			SYSCALLSERVERLIB(r,closedir(client),"in closedir di cancellazione_dato_cliente");
			return 1;
		}
	}
	
	if (errno!=0) {perror("in readdir di cancellazione_dato_cliente");return -2;}
	
	else {
		SYSCALLSERVERLIB(r,closedir(client),"in closedir di cancellazione_dato_cliente");
		return 0;
	}
}