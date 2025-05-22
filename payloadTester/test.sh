#!/bin/bash

{
  echo -ne "POST /directory/youpi.bla HTTP/1.1\r\n"
  echo -ne "Host: localhost:81\r\n"
  echo -ne "Content-Type: application/x-www-form-urlencoded\r\n"
  echo -ne "Content-Length: $(stat -c%s payload.txt)\r\n"
  echo -ne "Connection: close\r\n"
  echo -ne "\r\n"
  cat payload.txt
} | nc localhost 81 > res.txt
