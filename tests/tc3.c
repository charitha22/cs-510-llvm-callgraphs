//Please do not use any optimization switch (like O3,...) when you generate llvm bitcodes

#include <stdio.h>

struct s{
	void (*q)();
	int value;
} s1;

void F() { printf("f must be invoked!\n"); }
void E() { 
	s1.q = &F;
}
void D() {  }
void C() { D(); E(); }
void B() { C(); }
void A() { }

int main() {

	void (*p)();
	A();

	int x = 12; //another option would be 21
	if(x > 20) { 
		p = &C; 
	} else { 
		p = &E; 
	}

	(*p)();
	s1.q();

}