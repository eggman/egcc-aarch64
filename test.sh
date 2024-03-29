#!/bin/bash
cat <<EOF | aarch64-none-linux-gnu-gcc -xc -c -o tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
int add(int x, int y) { return x+y; }
int sub(int x, int y) { return x-y; }
int add6(int a, int b, int c, int d, int e, int f, int g, int h) {
  return a+b+c+d+e+f+g+h;
}
EOF

assert() {
  expected="$1"
  input="$2"

  ./egcc "$input" > tmp.s
  aarch64-none-linux-gnu-gcc -static -o tmp tmp.s tmp2.o
  qemu-aarch64 ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "return 0;"
assert 42 "return 42;"
assert 21 "return 5+20-4;"
assert 41 'return  12 + 34 - 5;'
assert 47 'return 5+6*7;'
assert 15 'return 5*(9-6);'
assert 4 'return (3+5)/2;'
assert 10 'return -10+20;'

assert 0 'return 0==1;'
assert 1 'return 42==42;'
assert 1 'return 0!=1;'
assert 0 'return 42!=42;'

assert 1 'return 0<1;'
assert 0 'return 1<1;'
assert 0 'return 2<1;'
assert 1 'return 0<=1;'
assert 1 'return 1<=1;'
assert 0 'return 2<=1;'

assert 1 'return 1>0;'
assert 0 'return 1>1;'
assert 0 'return 1>2;'
assert 1 'return 1>=0;'
assert 1 'return 1>=1;'
assert 0 'return 1>=2;'

assert 3 "return a=3;"
assert 3 'return a=3; a;'
assert 8 "a=b=4;return a+b;"
assert 18 'a=3; z=a*5;return  a+z;'

assert 3 'foo=3;return  foo;'
assert 8 'foo123=3; bar=5;return  foo123+bar;'

assert 10 'return 10;return 20;'
assert 22 'return_hoge = 22; return return_hoge;';

assert 3 'if (0) return 2; return 3;'
assert 3 'if (1-1) return 2; return 3;'
assert 2 'if (1) return 2; return 3;'
assert 2 'if (2-1) return 2; return 3;'
assert 7 'if (3-3) return 2; else return 7; return 3;'

assert 3 '{1; {2;} return 3;}'

assert 10 'i=0; while(i<10) i=i+1; return i;'
assert 55 'i=0; j=0; while(i<=10) {j=i+j; i=i+1;} return j;'

assert 55 'i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j;'
assert 3 'for (;;) return 3; return 5;'

assert 3 'return ret3();'
assert 5 'return ret5();'
assert 8 'return add(3, 5);'
assert 2 'return sub(5, 3);'
assert 36 'return add6(1,2,3,4,5,6,7,8);'

echo OK
