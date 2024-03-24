// Plantilla para maleta.c

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <string.h>
#include "maleta.h"
#include <pthread.h>

// Defina aca las estructuras y funciones adicionales que necesite
// ...

typedef struct
{
  double *w;   // puntero al arreglo de pesos
  double *v;   // puntero al arreglo de valores
  int *z;      // puntero al arreglo de la solución
  int n;       // número de articulos que puedo elegir
  double maxW; // peso máximo que puedo llevar
  int k;       // número de iteraciones
  double res;  // resultado del cada thread al calcular la solución
} thread_args;

/*
thread_func
función que se encarga de llamar a la función llenarMaletaSec
se pasa como argumento al pthread_create

*/
void *thread_func(void *arg)
{
  thread_args *args = (thread_args *)arg;                                               // Casteamos a thread_args
  args->res = llenarMaletaSec(args->w, args->v, args->z, args->n, args->maxW, args->k); // Corremos la función guardando el resultado en el res de cada thread
  return NULL;                                                                          // retornamos null
}

double llenarMaletaPar(double w[], double v[], int z[], int n,
                       double maxW, int k)
{
  // Primero creamos los 8 objetos threads.
  pthread_t threads[8];
  // Creamos un arreglo de argumentos para cada thread.
  thread_args args[8];
  // Guardamos el valor del mejor como -1 de momento
  double best = -1;

  // Ahora debemos de iterar para crear los threads y asignarles los argumentos
  for (int i = 0; i < 8; i++)
  {
    args[i].w = w;
    args[i].v = v;
    args[i].z = malloc(n * sizeof(int)); // reservamos memoria para el arreglo de la solución de cada función secuencial (no se pueden compartir)
    // usamos malloc porque no podemos compartir memoria entre threads
    // ademas quiero mantener la solucion de cada thread despues de que les haga join.
    // asi puedo retornarlo cuando modifique z
    args[i].n = n;
    args[i].maxW = maxW;
    args[i].k = k / 8;

    pthread_create(&threads[i], NULL, thread_func, &args[i]); // creamos el thread
  }
  // En este momento, todos los threads están corriendo.
  // Debemos de esperar a que todos terminen para poder obtener el mejor valor.
  // Esto se hace con un join iterado sobre todos los threads.

  for (int i = 0; i < 8; i++)
  {

    pthread_join(threads[i], NULL); // esperamos a que termine el thread
    // Vamos por thread recuperando el valor de la mejor solución
    if (args[i].res > best)
    {
      best = args[i].res;
      memcpy(z, args[i].z, n * sizeof(int)); // copiamos la solución del thread que tiene el mejor valor
    }
    free(args[i].z); // liberamos la memoria de la solución del thread
  }
  return best;
}
