#include<stdio.h>

void foo(){}

void bar(){
	foo();
}

void barr(){}

int main(){
	barr();
	bar();
	return 0;
}
