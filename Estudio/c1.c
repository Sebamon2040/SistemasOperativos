#include <pthread.h>
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
int adentro[2] = {0, 0};
Queue *q[2]; // = makeQueue()*2;enum { ROJO=0,AZUL=1};

typedef struct
{
    int ready;
    pthread_cond_t w;
} Request;

void entrar(int color)
{
    pthread_mutex_lock(&m);
    int oponente = !color;
    if (adentro[oponente] > 0 || !emptyQueue(q[oponente]))
    {
        Request req = {0, PTHREAD_COND_INITIALIZER};
        put(q[color], &req);
        while (!req.ready)
            pthread_cond_wait(&req.w, &m);
    }
    adentro[color]++;
    pthread_mutex_unlock(&m);
}

void salir(int color)
{
    pthread_mutex_lock(&m);
    int oponente = !color;
    adentro[color]--;
    if (adentro[color] == 0)
    {
        while (!emptyQueue(q[oponente]))
        {
            Request *preq = get(q[oponente]);
            preq->ready = 1;
            pthread_cond_signal(&preq->w);
        }
    }
    pthread_mutex_unlock(&m);
}