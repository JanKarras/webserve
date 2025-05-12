#!/bin/bash

# Pfad zu deinem Programm
PROGRAM_PATH="./webserv"
LOG_FILE="./log.txt"  # Hier den Pfad zu deiner Log-Datei angeben

# Anzahl der Wiederholungen
REPEATS=20

for i in $(seq 1 $REPEATS); do
    echo "Starte Durchlauf $i..."

    # Starte das Programm im Hintergrund und leite die Ausgabe in die Log-Datei um
    $PROGRAM_PATH > $LOG_FILE 2>&1 &

    # Warten bis der Server die Zeile "[INFO] Server init finished successfully" in die Log-Datei schreibt
    while ! tail -n 50 $LOG_FILE | grep -q "Server init finished successfully"; do
        echo "Warte auf Serverstart..."
        sleep 1
    done
    echo "Server ist bereit!"

    # Führe 10x curl-Anfragen aus (warten auf den Abschluss jeder Anfrage)
    for j in $(seq 1 10); do
        curl -s localhost:80 > /dev/null
    done

    # Beende das Programm
    pkill -f "$PROGRAM_PATH"

    # Kurze Pause vor dem nächsten Durchlauf
    sleep 2
done

echo "Fertig mit allen Durchläufen!"
