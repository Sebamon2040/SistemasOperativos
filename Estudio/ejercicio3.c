#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#define TRUE 1
#define FALSE 0
int readers = 0;
int WRITING = FALSE;
int waiting_writers =0;
#define BUFFER_SIZE 10
#define NUM_THREADS 20
#define RAND_MAX 10
int buffer[BUFFER_SIZE] = {0};
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

#include <time.h>
#include <stdlib.h>

void leer()
{
    // inicializa la semilla del generador de números aleatorios
    int index = rand() % BUFFER_SIZE; // genera un índice aleatorio
    printf("Lector lee el valor %d en el índice %d\n", buffer[index], index);
    sleep(1);
}

void escribir()
{
    // inicializa la semilla del generador de números aleatorios
    int index = rand() % BUFFER_SIZE; // genera un índice aleatorio
    buffer[index] = 1;                // escribe el índice en el buffer
    printf("Escritor escribe el valor %d en el índice %d\n", index, index);
    sleep(1);
}

void *reader(void *arg)
{
    // un lector puede trabajar en paralelo con otro lector, pero no con otro escritor.

    pthread_mutex_lock(&m);

    // si hay alguien escribiendo, esperamos
    while (WRITING == TRUE || waiting_writers >0)
    {
        
        pthread_cond_wait(&cond, &m); // esperamos y soltamos el mutex.
    }
    // nos despertaron. Ahora tenemos el mutex tomado. Avisamos que vamos a leer.
    readers++;
    
    // leemos
    leer();
    // terminamos de leer. Ahora avisamos que terminamos de leer.
    
    readers--;
    if (readers == 0)
    {
        pthread_cond_broadcast(&cond);
    }
    pthread_mutex_unlock(&m);
}

void *writer(void *arg)
{
    // primero tomamos el mutex.
    pthread_mutex_lock(&m);
    // ahora , con el mutex tomado, preguntamos si es que hay escritores o lectores. Un escritor puede entrar SOLO SI no hay nadie mas que él.
    while (WRITING == TRUE || readers > 0)
    {
        waiting_writers++;
        pthread_cond_wait(&cond, &m); // si es que hay alguien escribiendo o algun lector, esperamos. soltamos el mutex.
    }
    waiting_writers --;
    // volvemos a tomar el mutex. Ahora tenemos la libertad de escribir.
    WRITING = TRUE;
    escribir();
    // despues de escribir, avisamos de que terminamos.
    
    WRITING = FALSE;
    pthread_mutex_unlock(&m);
    pthread_cond_broadcast(&cond);
}

int main()

{
    srand(time(NULL));
    pthread_t lectores[NUM_THREADS], escritores[NUM_THREADS];

    // Crea varios hilos que ejecutan las funciones de lector y escritor
    for (int i = 0; i < NUM_THREADS; i++)
    {
        if (i % 2 == 0)
        {
            pthread_create(&escritores[i], NULL, writer, NULL);
        }
        else
        {
            pthread_create(&lectores[i], NULL, reader, NULL);
        }
    }

    // Espera a que todos los hilos terminen
    for (int i = 0; i < NUM_THREADS; i++)
    {
        if (i % 2 == 0)
        {
            pthread_join(escritores[i], NULL);
        }
        else
        {
            pthread_join(lectores[i], NULL);
        }
    }

    // Verifica que el buffer contenga los valores correctos

    return 0;
}