#!/bin/sh
set -e

BIN=bin
PIDS=""

start_server() {
  name=$1
  echo "[START] $name"
  "$BIN/$name" &
  pid=$!
  PIDS="$PIDS $pid"
}

stop_all() {
  for pid in $PIDS; do
    kill "$pid" 2>/dev/null || true
  done
  PIDS=""
}

trap stop_all EXIT

wait_for_port() {
  port=$1
  retries=20
  while ! nc -z 127.0.0.1 "$port"; do
    retries=$((retries-1))
    if [ $retries -le 0 ]; then
      echo "Port $port did not open in time"
      exit 1
    fi
    sleep 0.2
  done
}

# -----------------------
# Start all servers
# -----------------------
for server in server_404_test server_body_test server_middleware_test server_multi_query_test server_query_test server_sleep_test; do
  start_server "$server"
done

# -----------------------
# Wait for ports to open
# -----------------------
wait_for_port 3001
wait_for_port 3002
wait_for_port 3003
wait_for_port 3004
wait_for_port 3005
wait_for_port 3006

echo
# -----------------------
# Run curl tests
# -----------------------
for server in server_404_test server_body_test server_middleware_test server_multi_query_test server_query_test server_sleep_test; do
  echo "========== TEST: $server =========="
  echo
  case "$server" in

    ######################### should have sent 405

    # -----------------------
    # 404 Not Found / Method Not Allowed tests
    # -----------------------
    server_404_test)
      echo "[TEST] POST to GET-only route (should fail 404/405)"
      echo
      curl --silent --show-error --output - -i -X POST http://127.0.0.1:3004/only-get
      echo
      ;;

    # -----------------------
    # Body type tests
    # -----------------------
    server_body_test)
      # Text body
      echo "[TEST] POST text/plain"
      echo
      curl -i -X POST http://127.0.0.1:3003/echo/text -H "Content-Type: text/plain" -d "Hello text"
      echo
      echo

      # JSON body
      echo "[TEST] POST application/json"
      echo
      curl -i -X POST http://127.0.0.1:3003/echo/json -H "Content-Type: application/json" -d '{"msg":"Hello JSON"}'
      echo
      echo

      # Form body
      echo "[TEST] POST application/x-www-form-urlencoded"
      echo
      curl -i -X POST http://127.0.0.1:3003/echo/form -H "Content-Type: application/x-www-form-urlencoded" -d "key1=value1&key2=value2"
      echo
      echo

      # Binary body
      echo "[TEST] POST application/octet-stream"
      echo
      curl -i -X POST http://127.0.0.1:3003/echo/binary -H "Content-Type: application/octet-stream" --data-binary $'\x01\x02\x03\x04'
      echo
      echo

      # Empty body
      echo "[TEST] POST empty body"
      echo
      curl -i -X POST http://127.0.0.1:3003/echo/text -H "Content-Type: text/plain" -d ""
      echo
      echo

      # Malformed JSON
      echo "[TEST] POST malformed JSON"
      echo
      curl -i -X POST http://127.0.0.1:3003/echo/json -H "Content-Type: application/json" -d '{"msg": "missing quote}'
      echo
      echo

      ######################### repeated keys should be supported

      # Form with repeated keys
      echo "[TEST] POST form with repeated keys"
      echo
      curl -i -X POST http://127.0.0.1:3003/echo/form -H "Content-Type: application/x-www-form-urlencoded" -d "key=value1&key=value2"
      echo
      echo

      # No Content-Type header
      echo "[TEST] POST without Content-Type"
      echo
      curl -i -X POST http://127.0.0.1:3003/echo/text -d "No content-type header"
      echo
      echo

      ######################### should have sent 405

      # Method not allowed
      echo "[TEST] PUT to POST-only route"
      echo
      curl -i -X PUT http://127.0.0.1:3003/echo/text
      echo
      echo

      # Large binary body (~1MB)
      echo "[TEST] POST large binary body (~1MB)"
      echo
      # Create temp file
      TMPFILE=$(mktemp)
      head -c 1048576 /dev/urandom > "$TMPFILE"

      # Send using curl
      curl -i -X POST http://127.0.0.1:3003/echo/binary \
          -H "Content-Type: application/octet-stream" \
          --data-binary @"$TMPFILE"
      echo
      echo

      # Remove temp file
      rm "$TMPFILE"
      ;;

    # -----------------------
    # Middleware tests
    # -----------------------
    server_middleware_test)
      echo "[TEST] GET protected route without auth"
      echo
      curl --silent --show-error --output - -i http://127.0.0.1:3005/protected
      echo
      echo

      echo "[TEST] GET protected route with auth"
      echo
      curl --silent --show-error --output - -i -H "Authorization: secret" http://127.0.0.1:3005/protected
      echo
      echo
      ;;

    # -----------------------
    # Multi-query param tests
    # -----------------------
    server_multi_query_test)
      echo "[TEST] GET multiple query params"
      echo
      curl --silent --show-error --output - -i "http://127.0.0.1:3002/tags?tag=a&tag=b&tag=c"
      echo
      echo

      echo "[TEST] GET with no query params"
      echo
      curl --silent --show-error --output - -i "http://127.0.0.1:3002/tags"
      echo
      echo
      ;;

    # -----------------------
    # Query param tests
    # -----------------------
    server_query_test)
      echo "[TEST] GET with single query param"
      echo
      curl -i -X GET "http://127.0.0.1:3001/search?q=hello"
      echo
      echo

      echo "[TEST] GET with URL-encoded param"
      echo
      curl -i -X GET "http://127.0.0.1:3001/search?q=hello%20world"
      echo
      echo

      echo "[TEST] GET with multiple query params of same name"
      echo
      curl -i -X GET "http://127.0.0.1:3001/search?q=hello&q=world"
      echo
      echo

      echo "[TEST] GET with empty query string"
      echo
      curl -i -X GET "http://127.0.0.1:3001/search?"
      echo
      echo

      echo "[TEST] GET with custom header"
      echo
      curl -i -X GET http://127.0.0.1:3001/search -H "X-Custom-Header: test"
      echo
      echo

      echo "[TEST] GET non-existent route (should 404)"
      echo
      curl -i -X GET http://127.0.0.1:3001/notfound
      echo
      echo

      echo "[TEST] GET with excessive query params"
      echo
      curl -i -X GET "http://127.0.0.1:3001/search?$(for i in $(seq 1 100); do echo -n "q$i=value$i&"; done)"
      echo
      echo
      ;;

    # -----------------------
    # Sleep / delay tests
    # -----------------------
    server_sleep_test)
      echo "[TEST] GET echo (sleep test)"
      echo
      curl --silent --show-error --output - -i -X GET http://127.0.0.1:3006/echo
      echo
      echo
      ;;
  esac
  echo
done

echo "========== ALL TESTS DONE =========="

stop_all
