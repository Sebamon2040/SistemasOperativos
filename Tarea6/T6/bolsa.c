#include "bolsa.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pss.h"
#include "spinlocks.h"
#define INT_MAX 2147483647

// Declare aca sus variables globales

// VARIABLES GLOBALES

int mutex = OPEN;               // variable global para el spinlock
int precio_barato = INT_MAX;    // variable global para el precio mas barato
char *nombre_vendedor = NULL;   // variable global para el vendedor mas barato
char *nombre_comprador = NULL;  // variable global para el comprador
enum Estado { ESPERA, ADJUDICADO, RECHAZADO };
enum Estado VG;
enum Estado *pVG = &VG;
enum Estado *pEstadoOferta;

int DEBUG = 0;
int *m;

void cleanUp() {
    // limpiamos las variables globales
    precio_barato = INT_MAX;
    nombre_vendedor = NULL;
    nombre_comprador = NULL;
}

int vendo(int precio, char *vendedor, char *comprador) {
    spinLock(&mutex);  // ZONA CRITICA COMIENZA ACÁ.

    enum Estado estadoOferta = ESPERA;
    pEstadoOferta = &estadoOferta;

    if (precio <= precio_barato) {
        if (DEBUG) {
            printf("DEBUG: Precio de %d de %s aceptado\n", precio, vendedor);
        }
        // aquí hay dos casos. Si antes había un vendedor, entonces debemos
        // hacer que retorne false. Para ello, debemos de despertarlo y avisarle
        // que ya no es el precio mas bajo.
        if (nombre_vendedor != NULL && m != NULL) {
            if (*pVG == ESPERA) {
                if (DEBUG) {
                    printf("DEBUG: Despertando al vendedor\n");
                }
                *pVG = RECHAZADO;
                spinUnlock(m);
            }
        }

        // subasta con el nuevo precio actualizamos el precio mas barato
        *pEstadoOferta = ESPERA;
        precio_barato = precio;
        // marcamos a este vendedor como el mas barato
        nombre_vendedor = vendedor;

        // spinLock cerrado.
        int w = CLOSED;

        m = &w;
        spinUnlock(&mutex);  // soltamos el mutex.
        spinLock(&w);        // wait
        // este core se levantará luego con spinUnlock(m) por la función compro.
        if (*pVG == ADJUDICADO) {
            *pEstadoOferta = ADJUDICADO;
        }
        if (*pVG == RECHAZADO) {
            *pEstadoOferta = RECHAZADO;
        }

    } else {
        *pEstadoOferta = RECHAZADO;
    }

    if (*pEstadoOferta == ADJUDICADO) {
        printf("DEBUG: Vendedor : %s vendió a %s \n", vendedor,
               nombre_comprador);

        // COPIAMOS EL NOMBRE DEL COMPRADOR EN EL TERCER PARÁMETRO.
        strcpy(comprador, nombre_comprador);
        // cleanUp();
        spinUnlock(&mutex);  // ZONA CRITICA TERMINA ACÁ.

        return 1;
    } else if (*pEstadoOferta == RECHAZADO) {
        printf(
            "DEBUG: Precio de %d de %s rechazado, %d de %s sigue siendo el más "
            "barato\n",
            precio, vendedor, precio_barato, nombre_vendedor);

        spinUnlock(&mutex);  // ZONA CRITICA TERMINA ACÁ.

        return 0;
    } else {
        spinUnlock(&mutex);  // ZONA CRITICA TERMINA ACÁ.

        printf(
            "DEBUG: Fallo crítico. Thread en espera no fue atendido. "
            "Abortando\n");

        exit(1);
    }
}

int compro(char *comprador, char *vendedor) {
    spinLock(&mutex);  // ZONA CRITICA COMIENZA ACÁ.

    // chequeamos si es que hay algun vendedor esperando
    if (nombre_vendedor == NULL) {
        // si no hay vendedores esperando, entonces retornamos 0
        spinUnlock(&mutex);
        return 0;
    } else {
        // si hay un vendedor esperando, entonces debemos de adjudicar la venta
        // cambiamos el estado de la venta a adjudicado

        *pVG = ADJUDICADO;
        // copiamos el nombre del comprador en la variable global
        nombre_comprador = comprador;
        // DESBLOQUEAMOS EL SPIN LOCK, PARA QUE EL VENDEDOR NOS VEA

        // copiamos el nombre del vendedor en el segundo parámetro
        strcpy(vendedor, nombre_vendedor);
        int precio = precio_barato;
        if (m != NULL) {
            if (DEBUG) {
                printf("DEBUG: Despertando al vendedor\n");
            }

            spinUnlock(m);
        }
        precio_barato = INT_MAX;
        spinUnlock(&mutex);  // ZONA CRITICA TERMINA ACÁ.
        printf("DEBUG: Comprador : %s compró a %s por %d\n", comprador,
               vendedor, precio);
        return precio;
    }
}
