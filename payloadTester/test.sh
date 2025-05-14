#!/bin/bash

curl -X POST http://localhost:80/directory/youpi.bla -d @payload.txt > res.txt
