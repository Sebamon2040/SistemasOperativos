#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#define MAX_SIZE 100


typedef struct {
    
    int* array; // Array para almacenar los elementos de la cola de prioridad
    int size; // Tamaño actual de la cola de prioridad
    pthread_mutex_t mutex; // Mutex para controlar el acceso a la cola de prioridad
    pthread_cond_t cond_empty; //cond para avisar cuando la cola está vacia. 
    pthread_cond_t cond_full ; //cond para avisar cuando la cola está llena.
} PriorityQueue;

void push(PriorityQueue *q, int item) {

   
    //aqui debemos de insertar el nuevo valor. Ojo porque el array pueda que el array se llene.
    if (q->size >= MAX_SIZE) {
            // Si la cola de prioridad está llena, retornar un error o simplemente no hacer nada
            printf("Error: la cola de prioridad está llena.\n");
            pthread_mutex_unlock(&q->mutex);
            return;
        }
    //ahora insertamos el nuevo item en la posicion nueva.
    q->array[q->size] = item;
    //printf("Producer produced : %d\n",q->array[q->size]);
    //aumentamos el tamaño de la fila
    q->size++;    
}

int pop(PriorityQueue *q) {

    
    //luego revisamos si es que la lista está vacía o no,
    if (q->size == 0){
        printf("Error: La cola ya está vacía");
        pthread_mutex_unlock(&q->mutex);
        return -1;
    }

    //si es que no está vacía, extraemos el primer valor.
    int result = q->array[0];
    q->array[q->size] = -1;
    //ahora corremos toda la fila uno hacia la izquierda.
    // [1,2,3,4,5,6,7,8,9,10,-1] size = 10
    // [-1,2,3,4,5,6,7,8,9,10,-1] size = 10
    // => [2,3,4,5,6,7,8,9,-1,-1] size = 9
    for (int i = 0; i < q->size - 1; i++) {
        q->array[i] = q->array[i + 1];
    }
    
    q->size--;
    //printf("Consumer consumed : %d\n",result);
    
    return result;
}

void *producer(void *arg) {
    //tomamos el mutex
    PriorityQueue *q =(PriorityQueue*)arg;
    while(1){
        pthread_mutex_lock(&q->mutex);
        while(q->size == MAX_SIZE){
            pthread_cond_wait (&q->cond_full,&q->mutex);
        }   
        push(q,1);
        printf("size:%d\n",q->size);
        pthread_mutex_unlock(&q->mutex);
        pthread_cond_broadcast(&q->cond_empty);
        sleep(2);
    }
    
}

void *consumer(void *arg) {
    PriorityQueue *q =(PriorityQueue*)arg;
    while(1){
        pthread_mutex_lock(&q->mutex);
        while(q->size ==0){
            pthread_cond_wait(&q->cond_empty,&q->mutex);
        }
        pop(q);
        printf("size:%d\n",q->size);
        pthread_mutex_unlock(&q->mutex);
        pthread_cond_broadcast(&q->cond_full);
        
        sleep(1);
    }
}

int main() {
    // Crear la cola de prioridad
    PriorityQueue q;
    q.array = malloc(MAX_SIZE * sizeof(int));
    q.size = 0;
    pthread_mutex_init(&q.mutex, NULL);

    // Crear los hilos de productores y consumidores
    pthread_t producer_thread, consumer_thread;
    pthread_create(&producer_thread, NULL, producer, &q);
    pthread_create(&consumer_thread, NULL, consumer, &q);

    // Esperar a que todos los hilos terminen
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    // Limpiar
    free(q.array);
    pthread_mutex_destroy(&q.mutex);

    return 0;
}