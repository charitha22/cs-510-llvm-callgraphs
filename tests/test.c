#include<stdio.h>

void foo(){}

void bar(){
	foo();
}

int main(){
	bar();
	return 0;
}
