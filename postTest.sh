#!/bin/bash

# URL für die POST-Anfragen
url="http://localhost:80/post_body"

# Header für die POST-Anfragen
headers=(
    "-H 'User-Agent: TestClient/1.0'"
    "-H 'Transfer-Encoding: chunked'"
    "-H 'Content-Type: test/file'"
    "-H 'Accept-Encoding: gzip'"
)

# Anzahl der parallelen Anfragen
num_requests=1000

# Senden der parallelen Anfragen
for i in $(seq 1 $num_requests); do
    curl -s -X POST "$url" "${headers[@]}" -d "" &  # Anfrage im Hintergrund
done

# Warten bis alle Anfragen abgeschlossen sind
wait

echo "$num_requests Anfragen wurden gesendet."
