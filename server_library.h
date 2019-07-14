/** 
 *	@file server_library.h
 *  @brief Libreria del server per consentire le operazioni
 *
 *  @author Matteo De Francesco 562176
 *  
 * 	Si dichiara che il contenuto di questo file è in ogni sua parte
 * 	opera originale dell'autore
 * 	
 */

/**
 * @function registra_nuovo_cliente
 * @brief funzione di supporto ai thread del server per registrare
 *				un nuovo cliente(creazione cartella) o connetterlo 
 *
 * @param nome  	nome del cliente da registrare
 * 
 * @returns intero che dice se è stata creata la cartella o meno
 */
int registra_nuovo_cliente(char *nome);

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
 * @returns intero che dice se il dato è stato salvato correttamente,
 * 					se non è stato possibile salvarlo o se c'è stato un errore
 * 					nelle system call
 */
int salva_dato_cliente(char *nomefile,int lunghezza,void *dato,char *path);

/**
 * @function recupera_dato_cliente
 * @brief funzione di supporto ai thread del server per recuperare 
 * 				un dato di un cliente dal suo storage personale
 * 
 * @param nomefile	nome del file da recuperare
 * 				path			nome del client relativo al thread
 * 
 * @returns puntatore al dato recuperato, altrimenti NULL se il dato
 * 					non è presente o ci sono stati errori nelle system call
 */
void *recupera_dato_cliente(char *nomefile,char *path);

/**
 * @function cancellazione_dato_cliente
 * @brief funzione di supporto ai thread del server per cancellare
 * 				un dato di un cliente dal suo storage personale
 * 
 * @param nomefile	nome del file da cancellare
 * 				path			nome del client relativo al thread
 * 
 * @returns	intero che dice se il dato è stato cancellato correttamente,
 * 					se il dato non esiste oppure se c'è stato un errore nelle system call
 */
int cancellazione_dato_cliente(char *nomefile,char *path_client);
