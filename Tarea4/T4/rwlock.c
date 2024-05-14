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

int nEnterRead(nRWLock *rwl, int timeout) {
    START_CRITICAL
    // SI HAY UN ESCRITOR TRABAJANDO Y HAY ESCRITORES PENDIENTES , ESPERAMOS.
    if (!nth_emptyQueue(rwl->writersQueue) || rwl->WRITING) {
        // printf("Reader WAITING\n");
        nThread thisTh = nSelf();
        nth_putBack(rwl->readersQueue, thisTh);
        suspend(WAIT_RWLOCK);
        schedule();
    } else {
        // printf("Reader ENTERED\n");
        rwl->readersCount++;
        // printf("Ammount of readers = %d\n", rwl->readersCount);
        // printf("WRITING = %d\n", rwl->WRITING);
    }

    END_CRITICAL

    return 1;
}

int nEnterWrite(nRWLock *rwl, int timeout) {
    START_CRITICAL
    // SI HAY LECTORES O UN ESCRITOR TRABAJANDO, EL ESCRITOR ESPERA

    if (rwl->readersCount > 0 || rwl->WRITING) {
        // printf("there are %d readers\n", rwl->readersCount);
        // printf("Writer WAITING\n");
        nThread thisTh = nSelf();
        nth_putBack(rwl->writersQueue, thisTh);
        suspend(WAIT_RWLOCK);
        schedule();
    } else {
        // printf("ammount of readers when writer entering = %d\n",
        //        rwl->readersCount);
        // printf("Writer ENTERED\n");
        rwl->WRITING = 1;
        rwl->writersCount++;
        // printf("Ammount of writers = %d\n", rwl->writersCount);
    }

    END_CRITICAL
    return 1;
}

void nExitRead(nRWLock *rwl) {
    START_CRITICAL
    rwl->readersCount--;
    // printf("Reader EXITED\n");
    // printf("Ammount of readers = %d\n", rwl->readersCount);
    //  si es que no hay mas lectores trabajando y hay solicitudes de
    //  escritores, se deja pasar al primer escritor de la cola
    if (rwl->readersCount == 0 && !nth_emptyQueue(rwl->writersQueue)) {
        // printf("scheduling a writer\n");
        nThread w = nth_getFront(rwl->writersQueue);
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
    // printf("Writer EXITED\n");
    // printf("Ammount of writers = %d\n", rwl->writersCount);
    if (!nth_emptyQueue(
            rwl->readersQueue)) {  // hay solicitudes de lectores pendientes

        // printf("letting readers go in");
        while (!nth_emptyQueue(rwl->readersQueue)) {
            rwl->readersCount++;
            nThread r = nth_getFront(rwl->readersQueue);
            setReady(r);
        }

    } else {
        // si no hay lectores esperando, se deja pasar a un escritor
        if (!nth_emptyQueue(rwl->writersQueue)) {
            // printf("WRITER ENTERED\n");
            rwl->writersCount++;
            rwl->WRITING = 1;
            nThread w = nth_getFront(rwl->writersQueue);
            setReady(w);
        }
    }
    END_CRITICAL
}
