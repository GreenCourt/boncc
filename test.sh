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

assert 0 'int main(){0;}'
assert 42 'int main(){42;}'
assert 21 'int main(){5+20-4;}'
assert 41 'int main(){ 12 + 34 - 5 ; }'
assert 47 'int main(){5+6*7;}'
assert 15 'int main(){5*(9-6);}'
assert 4 'int main(){(3+5)/2;}'
assert 24 'int main(){-2*(9-6)+30;}'
assert 24 'int main(){-2*(+9-6)+30;}'
assert 36 'int main(){+2*(+9-6)+30;}'
assert 12 'int main(){+2*+6;}'
assert 10 'int main(){- -10;}'
assert 10 'int main(){- - +10;}'
assert 1 'int main(){-3 < 2;}'
assert 0 'int main(){2 < 1;}'
assert 0 'int main(){-3 > 2;}'
assert 1 'int main(){2 > 1;}'
assert 1 'int main(){-3 == -3;}'
assert 0 'int main(){-3 != -3;}'
assert 1 'int main(){-3 <= 2;}'
assert 1 'int main(){2 <= 2;}'
assert 0 'int main(){2 <= 1;}'
assert 0 'int main(){-3 >= 2;}'
assert 1 'int main(){2 >= 2;}'
assert 1 'int main(){2 >= 1;}'
assert 14 'int main(){int a; a = 3; int b; b = 5 * 6 - 8; a + b / 2;}'
assert 14 'int main(){int aaa; aaa = 3; int bb; bb = 5 * 6 - 8; aaa + bb / 2;}'
assert 5 'int main(){int aaa; aaa = 3; int bb; bb = 5 * 6 - 8; aaa + bb / 2; return aaa+2; return 14;}'
assert 14 'int main(){int aaa; aaa = 3; int bb; bb = 5 * 6 - 8; return aaa + bb / 2;}'
assert 5 'int main(){int a; a = 3; if(a==3) a+2; else a;}'
assert 3 'int main(){int a; a = 3; if(a < 3) a+2; else a;}'
assert 18 'int main(){int b; b = 0; int a; for(a=0; a<10; a = a + 1) b = a * 2; return b;}'
assert 10 'int main(){int a; a = 0; for(; a<10; ) a=a+1; return a;}'
assert 20 'int main(){int a; a = 0; int b; b = 0; for(; a<10; ) {a=a+1; b = b + 2;} return b;}'
assert 12 'int main(){int a; a = foo(); return a;}'
assert 6 'int main(){int a; a = bar(1,5); return a;}'
assert 12 'int main(){int a; a = baz(1,5,6); return a;}'
assert 12 'int main(){baz(1,5,6);}'
assert 6 'int add(int a, int b){a+b;} int main(){add(1,5);}'
assert 18 'int add(int a,int b){int x; x=12;x+a+b;} int main(){add(1,5);}'
assert 5 'int fib(int a){int r; r=0; if(a==0) return 0; else if(a==1) return 1; else return fib(a-1) + fib(a-2);} int main(){fib(5);}'
assert 8 'int fib(int a){int r; r=0; if(a==0) return 0; else if(a==1) return 1; else return fib(a-1) + fib(a-2);} int main(){fib(6);}'
assert 13 'int fib(int a){int r; r=0; if(a==0) r = 0; else if(a==1) r = 1; else r = fib(a-1) + fib(a-2); return r;} int main(){fib(7);}'
assert 3 'int main(){int x; x=3; int y; y=&x; return *y;}'
assert 3 'int main(){int x; x=3; int y; y=5; int z; z = &y + 8; return *z;}'

echo OK
