#include <pthread.h>
#include <stdio.h>

pthread_mutex_t m;
int counter = 0;

void *increment_counter(void *arg)
{
    pthread_mutex_lock(&m);
    for (int i = 0; i < 1000000; i++)
    {
        counter++;
    }
    pthread_mutex_unlock(&m);

    return NULL;
}

int main()
{
    pthread_t thread1, thread2;

    pthread_mutex_init(&m, NULL);

    pthread_create(&thread1, NULL, increment_counter, NULL);
    pthread_create(&thread2, NULL, increment_counter, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    printf("Counter is: %d\n", counter);

    if (counter != 2000000)
    {
        printf("Error: %d\n", counter);
    }
    pthread_mutex_destroy(&m);
    printf("Fin del programa\n");
    return 0;
}