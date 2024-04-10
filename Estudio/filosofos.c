#include <pthread.h>

int wait_time[] = {-1, -1, -1, -1, -1};
int pal[] = {1, 1, 1, 1, 1}; // 1= disponible
int cnt_glb = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
void pedir(int id)
{
    pthread_mutex_lock(&m);
    cnt_glb++;
    wait_time[id] = cnt_glb;
    while (!isWaitingMore(id) || !pal[id] || !pal[(id + 1) % 5])
    {
        pthread_cond_wait(&c, &m);
    }
    wait_time[id] = -1;
    pal[id] = 0;
    pal[(id + 1) % 5] = 0;
    pthread_mutex_unlock(&m);
}

void devolver(int id)
{
    pthread_mutex_lock(&m);
    pal[id] = 1;
    pal[(id + 1) % 5] = 1;
    pthread_mutex_unlock(&m);
    pthread_cond_broadcast(&c);
}

int isWaitingMore(int pos)
{
    int izq = (pos + 1) % 5;
    int der = (pos + 4) % 5;
    if (((wait_time[pos] < wait_time[izq]) & wait_time[izq] != -1) & ((wait_time[pos] < wait_time[der]) & wait_time[der] != -1))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// esta bien esta solucion al problema de los filosofos, sin hambruna y que coman en paralelo
