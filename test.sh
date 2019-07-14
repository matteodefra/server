#!/bin/bash

#Lancio 50 istanze di client che eseguono operazioni di STORE
for i in {{a..z},{A..X}}; do 
  ./client $i 1 &  
done
wait

#Lancio 50 istanze di client che eseguono operazioni di DELETE e RETRIEVE
for i in {{a..z},{A..D}}; do
  ./client $i 2 &
done 
for i in {E..X}; do
  ./client $i 3 &
done
wait
