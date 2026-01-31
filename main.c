#include "heap.h"
#include <stdio.h>

int main()
{
    int* x = (int*)_alloc(10*sizeof(int));

    for(int i = 0; i < 10; i++)
    {
        x[i] = i;
    }

    for(int i = 0; i < 10; i++)
    {
        printf("%d\n", x[i]);
    }

    _free(x);

}