/*
 * @file  util.c
 * @author Matteo De Francesco 562176
 * 
 * Si dichiara che il contenuto di questo file e' in ogni sua parte opera
 * originale dell'autore
 */

#include <stdio.h>
#include <stdlib.h>
#include "util.h"

/**
 * @function Malloc
 * @brief malloc con controllo dell'errore
 *
 * @param size       byte di memoria da allocare
 *
 * @returns puntatore al blocco di memoria allocato
 */
void* Malloc (size_t size) {
    void * tmp;
    if ( ( tmp = malloc(size) ) == NULL) {
        perror("Malloc");
        exit(EXIT_FAILURE); }
    else
    return tmp;
}