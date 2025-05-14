#!/bin/bash

# CGI-Header mit CRLF
printf "Status: 200 OK\r\n"
printf "Content-Type: text/plain; charset=UTF-8\r\n"
printf "\r\n"

# Body
ls
