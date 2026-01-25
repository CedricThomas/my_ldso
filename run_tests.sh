#!/usr/bin/env bash

run_test() {
  local test_name="$1"
  local expected="$2"
  shift 2
  local cmd=("$@")

  local output
  output="$("${cmd[@]}")"

  if [[ "$output" == "$expected" ]]; then
    echo "${test_name}: OK"
  else
    echo "${test_name}: KO"
    echo "---- diff ----"
    diff -u \
      <(printf "%s\n" "$expected") \
      <(printf "%s\n" "$output")
    echo "--------------"
  fi
}

expected_output="$(cat <<'EOF'
./test-standalone
1
2
3
EOF
)"
run_test "test direct standalone" "$expected_output" ./test-standalone 1 2 3

expected_output="$(cat <<'EOF'
./test-onelib
1
2
3
EOF
)"
run_test "test direct one lib" "$expected_output" ./test-onelib 1 2 3

expected_output="$(cat <<'EOF'
./test-libs
1
2
3
EOF
)"
run_test "test direct libs" "$expected_output" ./test-libs 1 2 3

