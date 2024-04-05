#define _XOPEN_SOURCE 500

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "reservar.h"

// Defina aca las variables globales y funciones auxiliares que necesite
// La idea es atender por orden de llegada a los procesos que quieren reservar
// No podemos usar una cola FIFO porque el enunciado no permite múltiples condiciones, por lo que hay que implementar tickets.

int ticket_dist = 0;   // tickets que se han entregado
int ticket_actual = 0; // ticket que se esta atendiendo

// debemos usar un mutex por cada vehículo que entra a pedir.
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// definimos una estructura para los 10 estacionamiento que existen

int estacionamiento[10] = {0};

void marcarEstacionamientos(int i, int k)
{
  // marcamos los estacionamientos ocupados , desde el i hasta el i+k
  for (int j = i; j < i + k; j++)
  {
    estacionamiento[j] = 1;
  }
}

void liberarEstacionamientos(int i, int k)
{
  // liberamos los estacionamientos ocupados , desde el i hasta el i+k
  for (int j = i; j < i + k; j++)
  {
    estacionamiento[j] = 0;
  }
}

int buscarEstacionamiento(int k)
{

  // Aquí tenemos el mutex tomado, por lo que no debemos de preocuparnos de que otro vehiculo nos tome un estacionamiento mientras nosotros chequeamos.
  // buscamos si es que hay k estacionamiento contiguos disponibles. Si es que los encontramos, retornamos el indice del primer estacionamiento usado,
  // por ejemplo, si es que el algoritmo , para k = 3,  encuentra que el estacionamiento 3, 4 y 5 están disponibles, retornará 3.
  // si no encuentra, retornará -1.
  for (int i = 0; i < 10; i++)
  {
    int j = 0;
    while (j < k && i + j < 10 && estacionamiento[i + j] == 0)
    {
      j++;
    }
    if (j == k)
    {
      return i;
    }
  }
  return -1;
}

void initReservar()
{
  // inicializamos todos los estacionamiento en 0
  for (int i = 0; i < 10; i++)
  {
    estacionamiento[i] = 0;
  }
}

void cleanReservar()
{
  // limpiamos todos los estacionamiento
  for (int i = 0; i < 10; i++)
  {
    estacionamiento[i] = 0;
  }
}

int reservar(int k)
{
  // lo primero, un vehiculo llega al parking lot y pide un numero. No pueden pedir
  // dos vehiculos el mismo numero, por lo que entramos a seccion critica
  pthread_mutex_lock(&mutex);
  int ticket = ticket_dist++; // agarramos un ticket.
  // ahora debemos de chequear si es que nos toca o no. Como no podemos usar mas de una condición, tocará hacer busy waiting solamente
  while (ticket != ticket_actual)
  {
    pthread_cond_wait(&cond, &mutex); // acá esperamos. Soltamos el mutex, por lo que otro vehículo puede entrar y llegar a pedir un ticket si quiere.
  }
  // si llegamos acá, es porque nos toca. Ahora debemos de buscar los estacionamientos disponibles.
  // Volvemos a tener el mutex tomado, por lo que la funcion buscarEstacionamiento se va a ejecutar en exclusion mutua
  while (buscarEstacionamiento(k) == -1)
  {
    // si es que no encontramos los estacionamientos, debemos de esperar a que se libere uno. Soltamos el mutex y esperamos.
    pthread_cond_wait(&cond, &mutex);
  }
  int i = buscarEstacionamiento(k);
  // si llegamos acá, es porque encontramos los estacionamientos.
  // marcamos los estacionamientos ocupados , desde el i hasta el i+k
  marcarEstacionamientos(i, k);

  // aumentamos el ticket actual
  ticket_actual++;
  // liberamos el mutex
  pthread_mutex_unlock(&mutex);
  // retornamos nuestro estacionamiento inicial
  return i;
}
void liberar(int e, int k)
{
  // un vehiculo se va del parking lot. Debemos de liberar los estacionamientos que ocupaba.
  // tomamos el mutex

  pthread_mutex_lock(&mutex);
  // liberamos los estacionamientos ocupados , desde el e hasta el e+k
  liberarEstacionamientos(e, k);
  // liberamos el mutex
  pthread_mutex_unlock(&mutex);
  // ahora, debemos de avisar a los vehículos que están esperando.
  pthread_cond_broadcast(&cond);
  // Aqui hay algo que no me gustó mucho, pero debido a restricciones de la tarea, tocará hacerlo asi nomas.
  // fijate que se despiertan a TODOS los autos, no solo a los que estan buscando estacionamiento.
  // Este broadcast despertará a todos los vehiculos que estén esperando, tanto como entrar a pedir el estacionamiento como
  // para buscar los estacionamientos disponibles.
}
