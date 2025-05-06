#!/bin/bash

URL="http://localhost:80"

print_test() {
    echo ""
    echo "===== Test $1 ====="
}

# Test 1: GET /
print_test "GET /"
curl -i "$URL/" 2>/dev/null

# Test 2: POST / with 0-byte body
print_test "POST / (0-byte)"
curl -i -X POST "$URL/" -H "Content-Type: test/file" --data '' 2>/dev/null

# Test 3: HEAD /
print_test "HEAD /"
curl -i -X HEAD "$URL/" 2>/dev/null

# Test 4: GET /directory
print_test "GET /directory"
curl -i "$URL/directory" 2>/dev/null

# Optional: add checks for malformed responses
echo ""
echo "===== Result Check ====="
echo "If any response starts directly with '<!DOCTYPE html>' or '<html>', it's missing a proper HTTP status line!"

curl -X POST http://localhost:80/directory/youpi.bla -d @payload.txt > res.txt
