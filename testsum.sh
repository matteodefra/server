#!/bin/bash


#Prendo dal file la somma totale delle operazioni effettuate, 
#di quelle con successo e fallite e le stampo
neffettuate=$(grep Effettuate testout.log | cut -d : -f 2 | paste -s -d+ | bc)
echo "Numero totale operazioni:"$neffettuate
nsuccesso=$(grep Successo testout.log | cut -d : -f 2 | paste -s -d+ | bc)
echo "Numero operazioni successo:"$nsuccesso
nfallite=$(grep Fallite testout.log | cut -d : -f 2 | paste -s -d+ | bc)
echo "Numero operazioni fallite:"$nfallite

#Se ci sono stati errori nei test stampo il nome del client
#e la corrispondente batteria
echo "Client falliti nei test:"
grep client testout.log | cut -d : -f 2 | sort -g

#Se ci sono stati client chiusi in connessione stampo il nome
echo "Client falliti perche gia connessi:"
grep connesso testout.log

killall -SIGUSR1 server
