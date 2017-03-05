#include <stdio.h>

struct s{
    void (*q)();
    int value;
} s1;

void F(){
	printf("Hello\n");
}

int main(){

    s1.q = &F;

    struct s* ptr_s = &s1;

    ptr_s->q();

}
