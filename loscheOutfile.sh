#!/bin/bash

DATEI1="output.txt"
DATEI2="output_response.txt"

# Funktion zum Leeren einer Datei, wenn sie existiert
leere_datei() {
  if [ -f "$1" ]; then
    > "$1"
    echo "Inhalt von $1 wurde gelöscht."
  else
    echo "Datei $1 existiert nicht."
  fi
}

# Beide Dateien leeren
leere_datei "$DATEI1"
leere_datei "$DATEI2"
