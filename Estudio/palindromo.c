#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int palindro(int *a, int i, int j, int n)
{
    while (i < j)
    {
        if (a[i] != a[n - 1 - i])
            return 0;
        i++;
    }
    return 1;
}

typedef struct
{
    int *a;
    int i;
    int j;
    int n;
    int res;
} Args;

void *thread(void *p)
{
    Args *args = (Args *)p;
    int *a = args->a;
    int i = args->i;
    int j = args->j;
    int n = args->n;
    args->res = palindro(a, i, j, n);
    return NULL;
}

int palindro_par(int *a, int n)
{
    pthread_t pid;
    Args *args = malloc(sizeof(Args));
    // dividimos la primera mitad en cuartos.

    int division = n / 4;
    args->a = a;
    args->i = division + 1;
    args->j = n / 2;
    args->n = n;
    int res1 = palindro(a, 0, division, n);
    pthread_create(&pid, NULL, thread, args);
    pthread_join(pid, NULL);
    int res2 = args->res;
    free(args);
    return res1 && res2;
}

int main()
{
    int a[] = {1, 2, 2, 4, 3, 2, 1, 100};
    int n = 7;

    int result = palindro_par(a, n);
    printf("result%d\n ", result);
    return 1;
}