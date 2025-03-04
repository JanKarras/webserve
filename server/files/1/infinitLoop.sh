#!/bin/bash

counter=1
while true; do
    echo $counter
    ((counter++))  # Erhöht den Zähler um 1
    sleep 1  # Warten für eine Sekunde, um die Ausgabe zu verlangsamen
done

