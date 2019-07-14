/**
 * @file  util.h
 * @brief Macro per gestione degli errori
 * 
 * @author Matteo De Francesco 562176
 * 
 * Si dichiara che il contenuto di questo file e' in ogni sua parte opera
 * originale dell'autore
 * 
 */

#include <stdio.h>
#include <stdlib.h>

//define per la grandezza del buffer di richiesta
#if !defined(M)
#define M 101
#endif

//define per il nome del socket
#if !defined(SOCKNAME)
#define SOCKNAME "./objstore.sock"
#endif

#if !defined(UNIX_PATH_MAX)
#define UNIX_PATH_MAX 108
#endif

//define per la lunghezza massima del nome di un file
#if !defined(MAXFILENAME)
#define MAXFILENAME 2048
#endif

//define usata all'interno dei thread. Se c'è un errore in qualche system call 
//il thread viene chiuso
#if !defined(SYSCALLTHREAD)
#define SYSCALLTHREAD(r, c, e) \
    if ((r = c) == -1)         \
    {                          \
        perror(e);             \
        r=0;                    \
    }
#endif

//define usata all'interno del client. Se c'è un errore il client
//si disconnette subito
#if !defined(SYSCALL)
#define SYSCALL(r, c, e) \
    if ((r = c) == -1)   \
    {                    \
        perror(e);       \
        exit(errno);     \
    }
#endif

#if !defined(SYSCALLCLIENT) 
#define SYSCALLCLIENT(r,c,e) \
    if ((r = c) == -1)       \
    {                        \
        perror(e);           \
        return 0;            \
    }
#endif


//define usata nella libreria del server nel caso del recupero del dato
#if !defined(CALL)
#define CALL(r, c, e)  \
    if ((r = c) == -1) \
    {                  \
        perror(e);     \
        return (void*)-2;   \
    }
#endif

//define usata nella libreria del server nelle altre funzioni
#if !defined(SYSCALLSERVERLIB)
#define SYSCALLSERVERLIB(r, c, e) \
    if ((r = c) == -1)            \
    {                             \
        perror(e);                \
        return -2;                \
    }                                
#endif

/**
 * @function Malloc
 * @brief malloc con controllo dell'errore
 *
 * @param size       byte di memoria da allocare
 *
 * @returns puntatore al blocco di memoria allocato
 */
void *Malloc(size_t size);
