/** 
 * @file client_library.h
 * @brief Libreria del client per consentire la comunicazione con il server
 *
 * @author Matteo De Francesco 562176
 *  
 * Si dichiara che il contenuto di questo file è in ogni sua parte
 * opera originale dell'autore
 * 	
 */

#if !defined(size_t)
#define size_t unsigned long
#endif

/**
 * @function os_connect
 * @brief funzione del client che consente la creazione del socket e la registrazione
 * 
 * @param name  nome del cliente che si vuole registrare
 * 
 * @returns intero che comunica se la risposta del server è stata 
 *          OK \n oppure KO message \n
 */
int os_connect(char* name);

/**
 * @function os_store
 * @brief funzione del client che consente il salvataggio di un dato 
 *        nel suo storage personale
 * 
 * @param name   nome del file da salvare
 *        block  puntatore al blocco di dati da memorizzare  
 * 				len 	 lunghezza del blocco di memoria da memorizzare
 * 
 * @returns intero che comunica se la risposta del server è stata 
 * 					OK \n oppure KO message \n	
 */
int os_store(char* name,void* block,size_t len);

/**
 * @function os_retrieve
 * @brief funzione del client che consente il recupero di un dato 
 *        dal suo storage personale
 * 
 * @param name   nome del file da salvare
 * 
 * @returns intero che comunica se la risposta del server è stata 
 * 					DATA len \n message oppure KO message \n	
 */
void *os_retrieve(char* name);

/** 
 * @function os_delete
 * @brief funzione del client che consente la cancellazione di un dato 
 *        dal suo storage personale
 * 
 * @param name   nome del file da cancellare
 * 
 * @returns intero che comunica se la risposta del server è stata 
 * 					OK \n oppure KO message \n	
 */
int os_delete(char* name);

/** 
 * @function os_disconnect
 * @brief funzione del client che consente la disconnessione 
 * 				di un client
 * 
 * @param NULL
 * 
 * @returns intero che comunica che la risposta del server è stata OK \n	
 */
int os_disconnect();
