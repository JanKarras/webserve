#!/bin/bash

printf "Status: 200 OK\r\n"
printf "Content-Type: text/plain; charset=UTF-8\r\n"
printf "\r\n"

echo "The current time is: $(date +"%Y-%m-%d %H:%M:%S")"
