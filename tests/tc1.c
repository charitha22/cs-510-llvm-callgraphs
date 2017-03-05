//Please do not use any optimization switch like O3,...
void F() {  }
void E() {  }
void D() {  }
void C() { D(); E(); }
void B() { C(); }
void A() { B(); }

int main() {

	void (*p)();
	A();
	p = &C;
	(*p)();
}