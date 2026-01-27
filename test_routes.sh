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

# Start all servers
for server in server_404_test server_body_test server_middleware_test server_multi_query_test server_query_test server_sleep_test; do
  start_server "$server"
done

# Wait for ports
wait_for_port 3001
wait_for_port 3002
wait_for_port 3003
wait_for_port 3004
wait_for_port 3005
wait_for_port 3006

echo
# Run curl tests
for server in server_404_test server_body_test server_middleware_test server_multi_query_test server_query_test server_sleep_test; do
  echo "========== TEST: $server =========="
  case "$server" in
    server_404_test)
      curl --silent --show-error --output - -i -X POST http://127.0.0.1:3004/only-get
      echo
      ;;
    server_body_test)
      curl --silent --show-error --output - -i -X POST http://127.0.0.1:3003/echo -d "hello world"
      echo
      curl --silent --show-error --output - -i -X POST http://127.0.0.1:3003/echo
      echo
      ;;
    server_middleware_test)
      curl --silent --show-error --output - -i http://127.0.0.1:3005/protected
      echo
      curl --silent --show-error --output - -i -H "Authorization: secret" http://127.0.0.1:3005/protected
      echo
      ;;
    server_multi_query_test)
      curl --silent --show-error --output - -i "http://127.0.0.1:3002/tags?tag=a&tag=b&tag=c"
      echo
      curl --silent --show-error --output - -i "http://127.0.0.1:3002/tags"
      echo
      ;;
    server_query_test)
      curl --silent --show-error --output - -i "http://127.0.0.1:3001/search?q=hello&page=2"
      echo
      curl --silent --show-error --output - -i "http://127.0.0.1:3001/search"
      echo
      ;;
    server_sleep_test)
      curl --silent --show-error --output - -i -X GET http://127.0.0.1:3006/echo
      echo
      ;;
  esac
  echo
done

echo "========== ALL TESTS DONE =========="

stop_all
