#!/bin/bash

URL="http://localhost:80/"
CLIENTS=20
REQUESTS_PER_CLIENT=5000

# Gemeinsame Datei für Statuscodes
RESULT_FILE="results.txt"
> $RESULT_FILE

# Funktion: Ein Client schickt N Anfragen und speichert Codes
client_job() {
  local client_id=$1
  local successes=0
  local failures=0

  for ((i=1; i<=REQUESTS_PER_CLIENT; i++)); do
    # Strenger Curl-Aufruf: -s silent, -o /dev/null, nur HTTP-Code ausgeben, Timeout 5s
    status=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "$URL")

    if [[ "$status" == "200" ]]; then
      ((successes++))
    else
      ((failures++))
    fi

    # Optional: Fortschritt ausgeben (auskommentieren wenn zu viel Output)
    # echo "Client $client_id - Request $i: $status"
  done

  echo "Client $client_id: Success=$successes Failures=$failures" >> $RESULT_FILE
}

# Starte alle Clients parallel
for ((c=1; c<=CLIENTS; c++)); do
  client_job $c &
done

wait

# Zusammenfassung aller Clients
total_success=0
total_fail=0
echo "Ergebnisse pro Client:"
cat $RESULT_FILE

while read line; do
  suc=$(echo $line | grep -oP 'Success=\K[0-9]+')
  fail=$(echo $line | grep -oP 'Failures=\K[0-9]+')
  ((total_success+=suc))
  ((total_fail+=fail))
done < $RESULT_FILE

echo "----------------------------"
echo "Gesamt Erfolgreich: $total_success"
echo "Gesamt Fehler:      $total_fail"
