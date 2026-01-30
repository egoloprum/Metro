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
for server in server_404_test server_body_test server_middleware_test server_multi_query_test server_query_test server_sleep_test server_stream_test server_params_test server_content_negotiation_test server_error_test server_middleware_chain_test server_keepalive_test; do
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
wait_for_port 3007
wait_for_port 3008
wait_for_port 3009
wait_for_port 3010
wait_for_port 3011
wait_for_port 3012

echo
# -----------------------
# Run curl tests
# -----------------------
for server in server_404_test server_body_test server_middleware_test server_multi_query_test server_query_test server_sleep_test server_stream_test server_params_test server_content_negotiation_test server_error_test server_middleware_chain_test server_keepalive_test; do
  echo "========== TEST: $server =========="
  echo
  case "$server" in

    # # -----------------------
    # # 404 Not Found / Method Not Allowed tests
    # # -----------------------
    # server_404_test)
    #   echo "[TEST] POST to GET-only route (should fail 405)"
    #   echo
    #   curl --silent --show-error --output - -i -X POST http://127.0.0.1:3004/only-get 
    #   echo
    #   ;;

    # # -----------------------
    # # Body type tests
    # # -----------------------
    # server_body_test)
    #   # Text body
    #   echo "[TEST] POST text/plain"
    #   echo
    #   curl -i -X POST http://127.0.0.1:3003/echo/text -H "Content-Type: text/plain" -d "Hello text"
    #   echo
    #   echo

    #   # JSON body
    #   echo "[TEST] POST application/json"
    #   echo
    #   curl -i -X POST http://127.0.0.1:3003/echo/json -H "Content-Type: application/json" -d '{"msg":"Hello JSON"}'
    #   echo
    #   echo

    #   # Form body
    #   echo "[TEST] POST application/x-www-form-urlencoded"
    #   echo
    #   curl -i -X POST http://127.0.0.1:3003/echo/form -H "Content-Type: application/x-www-form-urlencoded" -d "key1=value1&key2=value2"
    #   echo
    #   echo

    #   # Binary body
    #   echo "[TEST] POST application/octet-stream"
    #   echo
    #   curl -i -X POST http://127.0.0.1:3003/echo/binary -H "Content-Type: application/octet-stream" --data-binary $'\x01\x02\x03\x04'
    #   echo
    #   echo

    #   # Empty body
    #   echo "[TEST] POST empty body"
    #   echo
    #   curl -i -X POST http://127.0.0.1:3003/echo/text -H "Content-Type: text/plain" -d ""
    #   echo
    #   echo

    #   # Malformed JSON
    #   echo "[TEST] POST malformed JSON (expect 400)"
    #   echo
    #   curl -i -X POST http://127.0.0.1:3003/echo/json -H "Content-Type: application/json" -d '{"msg": "missing quote}'
    #   echo
    #   echo

    #   # Form with repeated keys
    #   echo "[TEST] POST form with repeated keys"
    #   echo
    #   curl -i -X POST http://127.0.0.1:3003/echo/form -H "Content-Type: application/x-www-form-urlencoded" -d "key=value1&key=value2"
    #   echo
    #   echo

    #   # No Content-Type header
    #   echo "[TEST] POST without Content-Type (expect 415)"
    #   echo
    #   curl -i -X POST http://127.0.0.1:3003/echo/text -d "No content-type header"
    #   echo
    #   echo

    #   # Method not allowed
    #   echo "[TEST] PUT to POST-only route (expect 405)"
    #   echo
    #   curl -i -X PUT http://127.0.0.1:3003/echo/text
    #   echo
    #   echo
    #   ;;

    # # -----------------------
    # # Middleware tests
    # # -----------------------
    # server_middleware_test)
    #   echo "[TEST] GET protected route without auth (expect 401)"
    #   echo
    #   curl --silent --show-error --output - -i http://127.0.0.1:3005/protected 
    #   echo
    #   echo

    #   echo "[TEST] GET protected route with auth"
    #   echo
    #   curl --silent --show-error --output - -i -H "Authorization: secret" http://127.0.0.1:3005/protected 
    #   echo
    #   echo
    #   ;;

    # # -----------------------
    # # Multi-query param tests
    # # -----------------------
    # server_multi_query_test)
    #   echo "[TEST] GET multiple query params"
    #   echo
    #   curl --silent --show-error --output - -i "http://127.0.0.1:3002/tags?tag=a&tag=b&tag=c"
    #   echo
    #   echo

    #   echo "[TEST] GET with no query params"
    #   echo
    #   curl --silent --show-error --output - -i "http://127.0.0.1:3002/tags"
    #   echo
    #   echo
    #   ;;

    # # -----------------------
    # # Query param tests
    # # -----------------------
    # server_query_test)
    #   echo "[TEST] GET with single query param"
    #   echo
    #   curl -i -X GET "http://127.0.0.1:3001/search?q=hello"
    #   echo
    #   echo

    #   echo "[TEST] GET with URL-encoded param"
    #   echo
    #   curl -i -X GET "http://127.0.0.1:3001/search?q=hello%20world"
    #   echo
    #   echo

    #   echo "[TEST] GET with multiple query params of same name"
    #   echo
    #   curl -i -X GET "http://127.0.0.1:3001/search?q=hello&q=world"
    #   echo
    #   echo

    #   echo "[TEST] GET with empty query string"
    #   echo
    #   curl -i -X GET "http://127.0.0.1:3001/search?"
    #   echo
    #   echo

    #   echo "[TEST] GET with custom header"
    #   echo
    #   curl -i -X GET http://127.0.0.1:3001/search -H "X-Custom-Header: test"
    #   echo
    #   echo

    #   echo "[TEST] GET non-existent route (expect 404)"
    #   echo
    #   curl -i -X GET http://127.0.0.1:3001/notfound 
    #   echo
    #   echo

    #   echo "[TEST] GET with excessive query params"
    #   echo
    #   # Limit to 50 to avoid argument list too long, but still test limits
    #   QUERY=$(for i in $(seq 1 50); do echo -n "q$i=value$i&"; done)
    #   curl -i -X GET "http://127.0.0.1:3001/search?${QUERY}"
    #   echo
    #   echo
    #   ;;

    # # -----------------------
    # # Sleep / delay tests
    # # -----------------------
    # server_sleep_test)
    #   echo "[TEST] GET echo with 1s delay"
    #   echo
    #   curl --silent --show-error --output - -i -X GET http://127.0.0.1:3006/echo 
    #   echo
    #   echo
    #   ;;

    # -----------------------
    # Streaming tests
    # -----------------------
    server_stream_test)
      # Test SSE streaming
      echo "[TEST] Server-Sent Events stream"
      curl -i --silent --show-error -N --max-time 2 http://127.0.0.1:3007/stream/sse || true
      echo
      echo

      # Test chunked transfer
      echo "[TEST] Chunked encoding stream"
      curl -i --silent --show-error http://127.0.0.1:3007/stream/chunks
      echo
      echo

      # Test file serving
      echo "[TEST] File serving"
      curl -i --silent --show-error http://127.0.0.1:3007/file/test
      echo
      echo

      # Test fixed length streaming
      echo "[TEST] Fixed length stream"
      curl -i --silent --show-error http://127.0.0.1:3007/stream/fixed
      echo
      echo
      ;;

    # -----------------------
    # Path parameters tests
    # -----------------------
    server_params_test)
      echo "[TEST] Single path parameter"
      curl -i --silent --show-error http://127.0.0.1:3008/users/123
      echo
      echo

      echo "[TEST] Multiple path parameters"
      curl -i --silent --show-error http://127.0.0.1:3008/users/42/posts/99
      echo
      echo

      echo "[TEST] Path param with query string"
      curl -i --silent --show-error "http://127.0.0.1:3008/search/electronics?q=phone"
      echo
      echo

      echo "[TEST] URL-encoded path param"
      curl -i --silent --show-error http://127.0.0.1:3008/users/John%20Doe
      echo
      echo

      echo "[TEST] Root level param"
      curl -i --silent --show-error http://127.0.0.1:3008/my-slug
      echo
      echo

      echo "[TEST] Static route vs param priority"
      curl -i --silent --show-error http://127.0.0.1:3008/api/v1/items/456
      echo
      echo
      ;;

    # -----------------------
    # Content negotiation tests
    # -----------------------
    server_content_negotiation_test)
      echo "[TEST] Valid content negotiation (JSON)"
      curl -i --silent --show-error -H "Accept: application/json" http://127.0.0.1:3009/api/data
      echo
      echo

      echo "[TEST] Content negotiation wildcard"
      curl -i --silent --show-error -H "Accept: */*" http://127.0.0.1:3009/api/data
      echo
      echo

      echo "[TEST] Content negotiation mismatch (expect 406)"
      curl -i --silent --show-error -H "Accept: text/html" http://127.0.0.1:3009/api/data
      echo
      echo

      echo "[TEST] Text endpoint with HTML accept"
      curl -i --silent --show-error -H "Accept: text/html" http://127.0.0.1:3009/api/text
      echo
      echo
      ;;

    # -----------------------
    # Error handling tests
    # -----------------------
    server_error_test)
      echo "[TEST] Payload too large (expect 413)"
      curl -i --silent --show-error -X POST http://127.0.0.1:3010/upload -H "Content-Type: text/plain" -d "This is way more than 100 bytes of data to trigger the payload too large error"
      echo
      echo

      echo "[TEST] Missing Content-Type on POST (expect 415)"
      curl -i --silent --show-error -X POST http://127.0.0.1:3010/strict -d "data"
      echo
      echo

      echo "[TEST] Internal server error (expect 500)"
      curl -i --silent --show-error http://127.0.0.1:3010/panic
      echo
      echo

      echo "[TEST] Malformed JSON (expect 400)"
      curl -i --silent --show-error -X POST http://127.0.0.1:3010/json-strict -H "Content-Type: application/json" -d '{"invalid'
      echo
      echo

      echo "[TEST] Request header fields too large"
      # Create a very long header value (>64KB)
      LONGVAL=$(python3 -c "print('X' * 70000)")
      curl -i --silent --show-error -H "X-Long-Header: $LONGVAL" http://127.0.0.1:3010/upload || echo "Connection reset or error"
      echo
      echo
      ;;

    # -----------------------
    # Middleware chain tests
    # -----------------------
    server_middleware_chain_test)
      echo "[TEST] Middleware chain execution (check X-Request-ID and X-Processing-Time headers)"
      curl -i --silent --show-error http://127.0.0.1:3011/chain-test
      echo
      echo

      echo "[TEST] Middleware short-circuit (expect 403)"
      curl -i --silent --show-error -H "X-Block: yes" http://127.0.0.1:3011/chain-test
      echo
      echo

      echo "[TEST] Middleware without short-circuit"
      curl -i --silent --show-error http://127.0.0.1:3011/chain-test
      echo
      echo
      ;;

    # -----------------------
    # Keep-alive tests
    # -----------------------
    server_keepalive_test)
      echo "[TEST] Keep-Alive header present"
      curl -i --silent --show-error http://127.0.0.1:3012/keepalive-test
      echo
      echo

      echo "[TEST] Connection close header"
      curl -i --silent --show-error http://127.0.0.1:3012/close-me
      echo
      echo

      echo "[TEST] Reusing connection (pipelining test)"
      (echo -e "GET /keepalive-test HTTP/1.1\r\nHost: localhost\r\n\r\nGET /keepalive-test HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n") | nc 127.0.0.1 3012 | head -30
      echo
      echo
      ;;
  esac
  echo
done

echo "========== ALL TESTS DONE =========="

stop_all
