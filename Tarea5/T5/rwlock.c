#define _XOPEN_SOURCE 500

#include "rwlock.h"

#include "nthread-impl.h"

struct rwlock {
    NthQueue *writersQueue;
    NthQueue *readersQueue;
    int writersCount;
    int readersCount;
    int WRITING;
};

nRWLock *nMakeRWLock() {
    nRWLock *rwl = nMalloc(sizeof(nRWLock));
    rwl->writersQueue = nth_makeQueue();
    rwl->readersQueue = nth_makeQueue();
    rwl->writersCount = 0;
    rwl->readersCount = 0;
    rwl->WRITING = 0;
    return rwl;
}

void nDestroyRWLock(nRWLock *rwl) {
    nth_destroyQueue(rwl->writersQueue);
    nth_destroyQueue(rwl->readersQueue);
    rwl->writersCount = 0;
    rwl->readersCount = 0;
    nFree(rwl);
}

void timeoutFunction(nThread thread) {
    NthQueue *queue = (NthQueue *)thread->ptr;
    nth_delQueue(queue, thread);
    thread->ptr = NULL;
}

int nEnterRead(nRWLock *rwl, int timeout) {
    START_CRITICAL
    // SI HAY UN ESCRITOR TRABAJANDO Y HAY ESCRITORES PENDIENTES , ESPERAMOS.
    if (!nth_emptyQueue(rwl->writersQueue) || rwl->WRITING) {
        nThread thisTh = nSelf();
        nth_putBack(rwl->readersQueue, thisTh);
        suspend(WAIT_RWLOCK);
        schedule();
    } else {
        rwl->readersCount++;
    }

    END_CRITICAL

    return 1;
}

int nEnterWrite(nRWLock *rwl, int timeout) {
    START_CRITICAL
    // SI HAY LECTORES O UN ESCRITOR TRABAJANDO, EL ESCRITOR ESPERA

    if (rwl->readersCount > 0 || rwl->WRITING) {

        nThread thisTh = nSelf();
        nth_putBack(rwl->writersQueue, thisTh);

        /**
         * Ahora debemos de distinguir entre los threads que
         * invocaron timer con timeout >0 o <0,Para ello, ahora usaremos esto.
         */
        if (timeout > 0) {
            // lo suspendemos con el macro del timer
            suspend(WAIT_RWLOCK_TIMEOUT);
            /**
             * Dado que el timer recibe el parámetro en nanosegundos,
             * pero el timeout está en milisegundos, debemos de convertirlo
             */
            long long timeout_ns = timeout * 1000000LL;
            /**
             * Le damos la queue a la funcion que se va a encargar
             * de interrumpir al thread y eliminarlo de la cola cuando
             * termine su timeout.
             */
            thisTh->ptr = rwl->writersQueue;
            nth_programTimer(timeout_ns, timeoutFunction);
        } else {
            // si el timeout es menor a 0, lo suspendemos con el macro de rwlock
            thisTh->ptr = rwl->writersQueue;
            suspend(WAIT_RWLOCK);
        }
        // sea cual sea el caso, se hace schedule
        schedule();
        /**
         * Si después de hacer schedule, el descriptor es null, eso significa
         * que el thread se sacó de la cola, por lo que la función no le dará el
         * recurso. Si no es null, entonces se le dará el recurso.
         *
         */
        if (thisTh->ptr == NULL) {
            END_CRITICAL
            return 0;
        }
    } else {

        rwl->WRITING = 1;
        rwl->writersCount++;
    }

    END_CRITICAL
    return 1;
}

void nExitRead(nRWLock *rwl) {
    START_CRITICAL
    rwl->readersCount--;
    //  si es que no hay mas lectores trabajando y hay solicitudes de
    //  escritores, se deja pasar al primer escritor de la cola
    if (rwl->readersCount == 0 && !nth_emptyQueue(rwl->writersQueue)) {
        /**
         * Ahora tenemos un dilema. Debemos ponernos en ambos casos de la cola,
         * tanto para los threads con timeout o sin timeout.
         */
        nThread w = nth_getFront(rwl->writersQueue);

        if (w->status == WAIT_RWLOCK_TIMEOUT) {
            // si el thread tiene timeout, lo cancelamos
            nth_cancelThread(w);
        }
        // si no tiene timeout, seguimos con lo mismo de antes.
        rwl->writersCount++;
        rwl->WRITING = 1;
        setReady(w);
        schedule();
    }
    END_CRITICAL
}

void nExitWrite(nRWLock *rwl) {
    START_CRITICAL
    rwl->writersCount--;
    rwl->WRITING = 0;

    // si es que no hay mas escritores esperando y hay solicitudes de lectores,
    // se deja pasar a todos los lectores de la cola
    if (!nth_emptyQueue(
            rwl->readersQueue)) { // hay solicitudes de lectores pendientes

        while (!nth_emptyQueue(rwl->readersQueue)) {
            rwl->readersCount++;
            nThread r = nth_getFront(rwl->readersQueue);
            setReady(r);
        }
        schedule();
    } else {
        // si no hay lectores esperando, se deja pasar a un escritor
        if (!nth_emptyQueue(rwl->writersQueue)) {
            // printf("WRITER ENTERED\n");
            nThread w = nth_getFront(rwl->writersQueue);
            // si el thread tiene timeout, lo cancelamos
            if (w->status == WAIT_RWLOCK_TIMEOUT) {
                nth_cancelThread(w);
            }
            // si no, seguirá como antes
            rwl->writersCount++;
            rwl->WRITING = 1;

            setReady(w);
            schedule();
        }
    }
    END_CRITICAL
}
