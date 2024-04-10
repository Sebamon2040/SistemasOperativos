#include <pthread.h>

typedef struct {

    int meta;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Colecta;

/*
    Esta función crea una Colecta.
    - fija la meta.
    - inicializa el mutex.
    - inicializa la condición.

    double meta: meta de la colecta.

    Retorna un puntero a la Colecta creada.
*/
Colecta *nuevaColecta(double meta){
    //creamos colecta.
    Colecta *colecta = (Colecta *) malloc(sizeof(Colecta));
    pthread_mutex_init(&colecta->mutex,NULL);
    pthread_cond_init(&colecta->mutex,NULL);
    return colecta;
}

/*
    Esta función es para aportar a una Colecta:

    Colecta *colecta: puntero a la colecta a la que se aportara:
    double monto: monto que aportará este thread a la colecta.

    Retorna el monto efectivamente aportado a la colecta (si el saldo de la colecta
    es menor al monto a aportar, se retorna el saldo, si no el monto).
*/
double aportar(Colecta *colecta, double monto){
    //pedimos el mutex.
    pthread_mutex_lock(&colecta->mutex);
    //aqui tenemos tomado el mutex. Vamos a aportar a la colecta.
    if (monto < colecta->meta)
        colecta->meta -= monto;
    else
    {
        monto = colecta->meta;
        colecta->meta = 0;
        pthread_cond_broadcast(&colecta->cond);
    }

    while (colecta->meta > 0)
        pthread_cond_wait(&colecta->mutex, &colecta->mutex);

    pthread_mutex_unlock(&colecta->mutex);
    return monto;
};

#define NUM_THREADS 22


typedef struct {
    double monto;
    Colecta *colecta;
}AportarArgs;

void* aportar_thread(void*arg){
    AportarArgs* args = (AportarArgs* )arg;
    double aportado = aportar(args->colecta,args->monto);
    printf("Aportado: f\n",aportado);
    return NULL;
}
