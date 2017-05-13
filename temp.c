#include <stdio.h>
void pass(void* );
int main()
{
    int x;
    x = 10;

    pass((void*)&x);
    return 0;
}

void pass(void *x)
{
   int y = *((int *)x);
   printf("%d\n", y);
   while(1);
}


