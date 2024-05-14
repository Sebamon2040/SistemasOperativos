#include "disk.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pss.h"

int current_track = 0;

// tengo dos colas, una para los que voy a atender, y otra para que se van a la
// espera

PriQueue
    *afterCurrentTrackQueue;  // request que van a ser atendidas en esta pasada
PriQueue *beforeCurrentTrackQueue;  // request que van a ser atendidas en la
                                    // proxima pasada

pthread_mutex_t m;

typedef struct {
    pthread_cond_t cond;
    int track;
} Request;

void iniDisk(void) {
    // inicializamos las colas
    pthread_mutex_init(&m, NULL);
    current_track = 0;
    afterCurrentTrackQueue = makePriQueue();
    beforeCurrentTrackQueue = makePriQueue();
    // inicializamos el mutex
}

void cleanDisk(void) {
    // limpiamos las colas
    pthread_mutex_destroy(&m);
    destroyPriQueue(afterCurrentTrackQueue);
    destroyPriQueue(beforeCurrentTrackQueue);
    // limpiamos el mutex
}

void requestDisk(int track) {
    // si es que el track es mayor a current_track, mando esta request a
    // atencion si es que es menor, mando la request a espera pedimos acceso al
    // disco (bloqueamos al mutex)
    pthread_mutex_lock(&m);
    // creamos el objeto request
    Request *req = malloc(sizeof(Request));
    // con el objeto request creado, tenemos que llenarlo con los datos del
    // request
    pthread_cond_init(&req->cond, NULL);
    req->track = track;
    // con la request llenada , ahora debemos decidir donde mandarla
    if (req->track >= current_track) {
        // lo mandamos a la queue de afterCurrentTrack
        priPut(afterCurrentTrackQueue, req, req->track);
    } else {
        priPut(beforeCurrentTrackQueue, req, req->track);
    }
    // ahora que mandamos a la cola a la request, debemos de leer. Para ello
    // revisamos el primer elemento de la cola.
    while (priPeek(afterCurrentTrackQueue) != req) {
        pthread_cond_wait(
            &req->cond,
            &m);  // si es que no se cumple, esperamos y soltamos el mutex.
    }

    // si llegamos acÃ¡ es porque le toco a la request hacer su pega.
    current_track = track;
    pthread_mutex_unlock(&m);
}

void releaseDisk() {
    // pedimos el mutex
    pthread_mutex_lock(&m);
    // sacamos el primer elemento de la lista de espera.
    Request *req = priGet(afterCurrentTrackQueue);
    // eliminamos la condiciÃ³n para que no cause problemas de memoria
    pthread_cond_destroy(&req->cond);
    // eliminamos de la memoria la request que fue anteriormente pedida con
    // malloc
    free(req);
    // si la cola de atencion estÃ¡ vacÃ­a, se swapea y comenzamos a atender a
    // los que estÃ¡n en espera y colocamos el current track en 0.
    if (emptyPriQueue(afterCurrentTrackQueue)) {
        // debemos cambiar el puntero entre las colas.
        current_track = 0;
        // swapeamos las colas de afterCurrentTrack y beforeCurrentTrack
        PriQueue *temp = afterCurrentTrackQueue;
        afterCurrentTrackQueue = beforeCurrentTrackQueue;
        beforeCurrentTrackQueue = temp;
    }
    if (!emptyPriQueue(afterCurrentTrackQueue)) {
        // si no estÃ¡ vacÃ­a, simplemente despertamos al siguiente valor.
        Request *next = priPeek(afterCurrentTrackQueue);
        pthread_cond_signal(&next->cond);
    }
    // desbloqueamos el mutex
    pthread_mutex_unlock(&m);
}
