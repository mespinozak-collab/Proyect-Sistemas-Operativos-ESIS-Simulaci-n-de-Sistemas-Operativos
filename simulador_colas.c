// Funciones de manejo de colas (simulador_colas.c)

#include "simulador.h"

// Crea un nuevo nodo con el proceso dado
NodoProceso* crear_nodo(Proceso* pcb) {
    NodoProceso* nuevo_nodo = (NodoProceso*)malloc(sizeof(NodoProceso));
    if (nuevo_nodo == NULL) {
        perror("Error al asignar memoria para NodoProceso");
        exit(EXIT_FAILURE);
    }
    nuevo_nodo->pcb = pcb;
    nuevo_nodo->siguiente = NULL;
    return nuevo_nodo;
}

// Añade un proceso al final de la cola (para FCFS y RR)
void encolar(NodoProceso** cola_ptr, Proceso* pcb) {
    NodoProceso* nuevo = crear_nodo(pcb);
    if (*cola_ptr == NULL) {
        *cola_ptr = nuevo;
    } else {
        NodoProceso* actual = *cola_ptr;
        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }
        actual->siguiente = nuevo;
    }
}

// Extrae y devuelve el primer proceso de la cola (para FCFS y RR)
Proceso* desencolar(NodoProceso** cola_ptr) {
    if (*cola_ptr == NULL) {
        return NULL; // Cola vacía
    }
    NodoProceso* temp = *cola_ptr;
    Proceso* pcb_extraido = temp->pcb;
    *cola_ptr = temp->siguiente;
    free(temp);
    return pcb_extraido;
}

// Implementación de la función para el algoritmo SPN
// (Inserta ordenando por menor servicio, y menor llegada en caso de empate)
void insertar_spn(NodoProceso** cola_ptr, Proceso* pcb) {
    NodoProceso* nuevo = crear_nodo(pcb);
    NodoProceso* actual = *cola_ptr;
    NodoProceso* previo = NULL;
   
    // Busca la posición donde el nuevo proceso debe ir:
    // Criterio: (servicio de pcb < servicio de actual) O
    //           (servicio de pcb == servicio de actual Y llegada de pcb < llegada de actual)
    while (actual != NULL &&
           (pcb->servicio > actual->pcb->servicio ||
           (pcb->servicio == actual->pcb->servicio && pcb->llegada > actual->pcb->llegada))) {
       
        previo = actual;
        actual = actual->siguiente;
    }

    if (previo == NULL) {
        // Insertar al principio (o lista vacía)
        nuevo->siguiente = *cola_ptr;
        *cola_ptr = nuevo;
    } else {
        // Insertar en el medio/final
        nuevo->siguiente = actual;
        previo->siguiente = nuevo;
    }
}
