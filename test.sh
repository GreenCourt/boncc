#!/bin/bash
cc -x c -c - -o tmpfunc.o<< 'EOF' || exit $?
int foo() { return 12; }
int bar(int x,int y) { return x+y; }
int baz(int x,int y, int z) { return x+y+z; }
EOF

assert() {
    expected="$1"
    input="$2"

    ./boncc "$input" > tmp.s
    [ $? -ne 0 ] && echo "${0}:${BASH_LINENO[0]}: boncc runtime error." && exit 1

    cc -o tmp tmp.s tmpfunc.o
    [ $? -ne 0 ] && echo "${0}:${BASH_LINENO[0]}: failed to assemble." && exit 1

    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]
    then
        echo "$input => $actual"
    else
        echo "${0}:${BASH_LINENO[0]}: failed."
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 '0;'
assert 42 '42;'
assert 21 '5+20-4;'
assert 41 ' 12 + 34 - 5 ; '
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 24 '-2*(9-6)+30;'
assert 24 '-2*(+9-6)+30;'
assert 36 '+2*(+9-6)+30;'
assert 12 '+2*+6;'
assert 10 '- -10;'
assert 10 '- - +10;'
assert 1 '-3 < 2;'
assert 0 '2 < 1;'
assert 0 '-3 > 2;'
assert 1 '2 > 1;'
assert 1 '-3 == -3;'
assert 0 '-3 != -3;'
assert 1 '-3 <= 2;'
assert 1 '2 <= 2;'
assert 0 '2 <= 1;'
assert 0 '-3 >= 2;'
assert 1 '2 >= 2;'
assert 1 '2 >= 1;'
assert 14 'a = 3; b = 5 * 6 - 8; a + b / 2;'
assert 14 'aaa = 3; bb = 5 * 6 - 8; aaa + bb / 2;'
assert 5 'aaa = 3; bb = 5 * 6 - 8; aaa + bb / 2; return aaa+2; return 14;'
assert 14 'aaa = 3; bb = 5 * 6 - 8; return aaa + bb / 2;'
assert 5 'a = 3; if(a==3) a+2; else a;'
assert 3 'a = 3; if(a < 3) a+2; else a;'
assert 18 'b = 0; for(a=0; a<10; a = a + 1) b = a * 2; return b;'
assert 10 'a = 0; for(; a<10; ) a=a+1; return a;'
assert 20 'a = 0; b = 0; for(; a<10; ) {a=a+1; b = b + 2;} return b;'
assert 12 'a = foo(); return a;'
assert 6 'a = bar(1,5); return a;'
assert 12 'a = baz(1,5,6); return a;'
assert 12 'baz(1,5,6);'

echo OK
