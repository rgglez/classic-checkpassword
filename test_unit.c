/* unit tests for the library primitives; run via "make test" */
#include <assert.h>
#include <stdio.h>
#include "stralloc.h"
#include "str.h"
#include "byte.h"

static stralloc sa;

int main(void)
{
  char buf[16];

  /* str_len / str_chr / str_start */
  assert(str_len("") == 0);
  assert(str_len("hello") == 5);
  assert(str_chr("hello",'l') == 2);
  assert(str_chr("hello",'z') == 5); /* not found -> index of NUL */
  assert(str_chr("",'a') == 0);
  assert(str_start("hello","he"));
  assert(!str_start("hello","hz"));
  assert(str_start("hello",""));

  /* byte_copy / byte_diff */
  byte_copy(buf,6,"abcde");
  assert(byte_diff(buf,6,"abcde") == 0);
  assert(byte_diff("abc",3,"abd") != 0);
  assert(byte_diff("",0,"") == 0);

  /* stralloc build + content */
  sa.len = 0;
  assert(stralloc_copys(&sa,"USER"));
  assert(stralloc_cats(&sa,"="));
  assert(stralloc_cats(&sa,"root"));
  assert(stralloc_0(&sa));
  assert(sa.len == 10);
  assert(byte_diff(sa.s,10,"USER=root\0") == 0);

  /* stralloc growth across reallocation */
  { int i;
    sa.len = 0;
    for (i = 0;i < 1000;++i) assert(stralloc_cats(&sa,"xy"));
    assert(sa.len == 2000);
    assert(sa.s[0] == 'x' && sa.s[1999] == 'y');
  }

  /* allocator overflow guards: wrap-inducing sizes rejected, no crash */
  assert(stralloc_ready(&sa,1000) == 1);
  sa.len = 1000;
  assert(stralloc_readyplus(&sa,1000) == 1);
  assert(stralloc_ready(&sa,0xFFFFFFF0u) == 0);
  assert(stralloc_readyplus(&sa,0xFFFFFFF0u) == 0); /* n + len wraps */
  assert(stralloc_readyplus(&sa,0xF0000000u) == 0); /* growth math wraps */
  /* guards must not have corrupted the existing buffer */
  assert(sa.s[0] == 'x' && sa.s[1999] == 'y');

  printf("test_unit: all tests passed\n");
  return 0;
}
