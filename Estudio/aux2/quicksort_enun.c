#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#define SIZE 100000000
#define THRESHOLD 1000

void quicksort(int a[], int i, int j, int n);

void swap(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

int particionar(int a[], int low, int high) {
    int pivot = a[high];  
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        if (a[j] <= pivot) {
            i++;
            swap(&a[i], &a[j]);
        }
    }
    swap(&a[i+1],&a[high]);
    return (i + 1);  
}


void quicksort_seq(int a[], int i, int j){
    if (i < j){
        int h = particionar(a, i, j);
        quicksort_seq(a, i, h - 1);
        quicksort_seq(a, h + 1, j);
    }
}

typedef struct {
    int *a; // a[]
    int i, j , n;  //indice inicial y final | tamaño del arreglo
} Args;

void *thread_function(void *p){
    //primero, debemos de castear el input a tipo args.
    Args (*args ) = (Args *)p;
    quicksort(args->a,args->i,args->j,args->n);
    return NULL;
}
int thread_numb = 1;
void quicksort(int a[], int i, int j, int n){
    
    //primero, hay que ver que i sea menor a j, 
    if (i<j){
        if (n<=THRESHOLD){
            quicksort_seq(a, i, j);
        }
        else{
            thread_numb++;
            int h = particionar(a,i,j); //obtemeos el indice del numero del medio (pivote)
            //creamos un thread para paralelizar.
            pthread_t pid ; //creamos objeto pid de tipo thread.
            //vamos a pasarle los argumentos para crear su propia rutina
            Args args = {a,i,h,n/2};
            pthread_create(&pid,NULL,thread_function,&args); //creamos un thread con la thread_func

            //obviamente, nos queda el propio thread del main. Este va a ejecutar también quicksort, pero para el resto del arreglo
            quicksort(a,h+1,j,n/2); //ejecutamos quicksort entre h+1 y j en un arreglo de tamaño n/2. Del resto se encarga el otro thread.
            pthread_join(pid, NULL);
            
        }
    }

}
int main() {
    srand(time(NULL));

    int*a = malloc(SIZE*sizeof(int));

    for (int i = 0; i < SIZE; i++) {
        a[i] = rand();  // Genera un número aleatorio
    }
    int n = SIZE / sizeof(int);

    clock_t start, end;
    double cpu_time_used;

    start = clock();

    quicksort(a, 0, SIZE-1, SIZE);

    end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    

    printf("Time taken: %f seconds\n", cpu_time_used);
    printf("Número de threads %d\n",thread_numb);
    free(a);
    return 0;
}

