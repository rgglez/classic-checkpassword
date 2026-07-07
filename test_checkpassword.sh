# exit-code tests for the checkpassword binary; run via "make test"
# interface: reads "login\0password\0timestamp\0" on fd 3
# exit 0 = ok, 1 = bad credentials, 2 = misuse, 111 = temporary failure

fail=0
check() { # check <name> <expected> <actual>
  if test "$2" = "$3"; then
    echo "ok: $1 (exit $3)"
  else
    echo "FAIL: $1 (expected $2, got $3)"; fail=1
  fi
}

# no subprogram argument -> 2
./checkpassword >/dev/null 2>&1 3</dev/null
check "no-arg" 2 $?

# empty input -> 2
printf '' | ./checkpassword /bin/true 3<&0
check "empty-input" 2 $?

# missing NUL terminators -> 2
printf 'loginonly' | ./checkpassword /bin/true 3<&0
check "malformed-input" 2 $?

# login but no password field -> 2
printf 'login\0' | ./checkpassword /bin/true 3<&0
check "missing-password" 2 $?

# unknown user -> 1 (and must not crash)
printf 'nosuchuser99\0pass\0Y0\0' | ./checkpassword /bin/true 3<&0
check "unknown-user" 1 $?

# real user, wrong password -> 1 (crypt() NULL path when shadow unreadable)
printf '%s\0wrongpass\0Y0\0' "`id -un`" | ./checkpassword /bin/true 3<&0
check "wrong-password" 1 $?

# oversized input (>512 bytes) -> 1
dd if=/dev/zero bs=1 count=600 2>/dev/null | tr '\0' 'a' | \
  ./checkpassword /bin/true 3<&0
check "oversized-input" 1 $?

test $fail = 0 && echo "test_checkpassword: all tests passed"
exit $fail
