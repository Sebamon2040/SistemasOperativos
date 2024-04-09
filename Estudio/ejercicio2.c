#include <pthread.h>
#include <stdio.h>

#define BUFFER_SIZE 10
#define NUM_OPERATIONS 1000

int buffer[BUFFER_SIZE] = {0};
int count = 0; // Número de elementos en el buffer

pthread_mutex_t m; // iniciamos el mutex

pthread_cond_t cond_full = PTHREAD_COND_INITIALIZER;  // cuando el buffer se llena.
pthread_cond_t cond_empty = PTHREAD_COND_INITIALIZER; // cuando el buffer tiene elementos.

void *producer(void *arg)
{
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        pthread_mutex_lock(&m);
        // ahora haremos busy waiting. La idea es que si el buffer está lleno, el productor se duerme
        while (count == BUFFER_SIZE)
        {
            pthread_cond_wait(&cond_full, &m);
        }
        // si llegamos aquí, el buffer no está lleno
        // llenamos el elemento en la posición count con un 1 solamente.
        buffer[count] = 1;
        count++;
        pthread_cond_signal(&cond_empty); // despertamos a los consumidores
        pthread_mutex_unlock(&m);         // desbloqueamos el mutex.
    }
    return NULL;
}

void *consumer(void *arg)
{

    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        pthread_mutex_lock(&m); // bloqueamos el mutex
        while (count == 0)
        {                                       // si no hay elementos en el buffer, esperamos
            pthread_cond_wait(&cond_empty, &m); // espera y suelta el mutex
        }
        // volvemos a tomar el mutex. Ahora extraemos el valor y decrementamos la cantidad de elementos en buffer
        buffer[count - 1] = 0;
        count--;

        pthread_cond_signal(&cond_full);
        pthread_mutex_unlock(&m);
    }
    return NULL;
}

int main()
{
    pthread_t producer_thread, consumer_thread;

    pthread_mutex_init(&m, NULL);

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    if (count != 0)
    {
        printf("Test failed: buffer should be empty but contains %d elements\n", count);
        return 1;
    }

    pthread_mutex_destroy(&m);

    printf("Test passed: buffer is empty as expected\n");
    return 0;
}