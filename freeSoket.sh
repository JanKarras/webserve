#!/bin/bash

PORT=8080

# Überprüfe, ob der Port belegt ist
PID=$(lsof -t -i :$PORT)

if [ ! -z "$PID" ]; then
    echo "Port $PORT ist belegt. Beende Prozess $PID..."
    kill $PID
    sleep 1  # Warte, damit der Port wirklich frei wird
fi

PORT=80

# Überprüfe, ob der Port belegt ist
PID=$(lsof -t -i :$PORT)

if [ ! -z "$PID" ]; then
    echo "Port $PORT ist belegt. Beende Prozess $PID..."
    kill $PID
    sleep 1  # Warte, damit der Port wirklich frei wird
fi
