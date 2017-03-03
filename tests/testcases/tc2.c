//Please do not use any optimization switch (like O3,...) when you generate llvm bitcodes

#include <stdio.h>

struct s{
	void (*q)();
	int value;
} s1;

void F() { printf("f must be invoked!\n"); }
void E() { 
	s1.q = &F;
	s1.q();
}
void D() {  }
void C() { D(); E(); }
void B() { C(); }
void A() { B(); }

int main() {

	A();

}


