// simulador.c (o continuar en main.c)

#include "simulador.h" 

// Función auxiliar: Ordena el arreglo inicial de procesos por tiempo de llegada.
void ordenar_por_llegada(Proceso procesos[], int n) {
    // Usamos qsort de C para ordenar el arreglo de estructuras
    // Se requiere una función de comparación:
    int comparar_llegada(const void* a, const void* b) {
        Proceso* p1 = (Proceso*)a;
        Proceso* p2 = (Proceso*)b;
        return p1->llegada - p2->llegada;
    }
    qsort(procesos, n, sizeof(Proceso), comparar_llegada);
}

int comparar_por_pid(const void* a, const void* b) {
    Proceso* proc_a = (Proceso*)a;
    Proceso* proc_b = (Proceso*)b;
   
    // Restar los PIDs: Si a->pid < b->pid, devuelve negativo (a va primero)
    return proc_a->pid - proc_b->pid;
}

void iniciar_simulacion(Proceso procesos[], int num_procesos, Configuracion conf, BloqueMemoria** memoria_ptr) {
    // Inicialización y ordenamiento...
    ordenar_por_llegada(procesos, num_procesos);
   
    NodoProceso* cola_listos = NULL;
    NodoProceso* lista_terminados = NULL;
    Proceso* proceso_en_cpu = NULL;
   
    int tiempo_actual = 0;
    int procesos_terminados = 0;
    int indice_proceso_llegada = 0;
   
    // **NUEVA VARIABLE PARA RR**
    int quantum_restante_actual = 0;

    printf("\n--- INICIANDO SIMULACIÓN (%s) ---\n", conf.algoritmo_cpu);

    while (procesos_terminados < num_procesos || proceso_en_cpu != NULL || indice_proceso_llegada < num_procesos) {
       
        // 1. Manejo de Llegadas
        while (indice_proceso_llegada < num_procesos &&
               procesos[indice_proceso_llegada].llegada == tiempo_actual) {
           
            Proceso* p_nuevo = &procesos[indice_proceso_llegada];
           
            if (strcmp(conf.algoritmo_cpu, "SPN") == 0) {
                insertar_spn(&cola_listos, p_nuevo);
            } else { // FCFS y RR
                encolar(&cola_listos, p_nuevo);
            }
            indice_proceso_llegada++;
        }

        // 2. Selección del Proceso (Despacho)
        if (proceso_en_cpu == NULL && cola_listos != NULL) {
            proceso_en_cpu = desencolar(&cola_listos);
           
            // Registrar Inicio (solo la primera vez)
            if (proceso_en_cpu->inicio == -1) {
                proceso_en_cpu->inicio = tiempo_actual;
            }
           
            // **INICIALIZAR QUANTUM para RR**
            if (strcmp(conf.algoritmo_cpu, "RR") == 0) {
                quantum_restante_actual = conf.quantum;
            }
        }
        
        if (proceso_en_cpu->restante == 0) {
	    proceso_en_cpu->fin = tiempo_actual;
	   
	    // **AQUÍ SE LIBERA LA MEMORIA ASIGNADA**
	    if (proceso_en_cpu->tam_memoria > 0) {
		desasignar_memoria(memoria_ptr, proceso_en_cpu); // Asumiendo que memoria_principal es accesible
	    }

	    encolar(&lista_terminados, proceso_en_cpu);
	    proceso_en_cpu = NULL;
	    procesos_terminados++;
	}
       
        // 3. Ejecución
        if (proceso_en_cpu != NULL) {
           
            // Avanzar el tiempo y actualizar restante
            tiempo_actual++;
            proceso_en_cpu->restante--;
           
            // **MANEJO DE QUANTUM**
            if (strcmp(conf.algoritmo_cpu, "RR") == 0) {
                quantum_restante_actual--;
            }
           
            // 4. Manejo de Terminación
            if (proceso_en_cpu->restante == 0) {
                proceso_en_cpu->fin = tiempo_actual;
                encolar(&lista_terminados, proceso_en_cpu);
                proceso_en_cpu = NULL;
                procesos_terminados++;
               
            }
            // 5. Manejo de Expropiación (Solo Round Robin)
            else if (strcmp(conf.algoritmo_cpu, "RR") == 0 && quantum_restante_actual == 0) {
                // Quantum expiró, pero el proceso no ha terminado. ¡Expropiar!
                encolar(&cola_listos, proceso_en_cpu); // Mover al final de la cola
                proceso_en_cpu = NULL; // CPU queda libre
            }
           
        } else {
            // CPU está libre y la cola de listos está vacía.
            // Avanzar el tiempo solo si hay procesos por llegar, evitando un bucle infinito
            if (indice_proceso_llegada < num_procesos) {
                tiempo_actual++;
            } else if (procesos_terminados == num_procesos) {
                // Si todos terminaron y la CPU está libre, salimos del bucle.
                break;
            }
        }
    } // Fin del bucle principal

    // 6. Cálculo e Impresión de Métricas
    calcular_y_mostrar_metricas(lista_terminados, num_procesos, tiempo_actual);
}

// Función de cálculo e impresión
void calcular_y_mostrar_metricas(NodoProceso* lista_terminados, int num_procesos, int tiempo_total) {
   
    // 1. Copiar los datos de la lista enlazada a un array para ordenarlos
    Proceso* procesos_ordenados = (Proceso*)malloc(num_procesos * sizeof(Proceso));
    NodoProceso* actual = lista_terminados;
    int k = 0;
    while (actual != NULL) {
        // Copiar la estructura completa del proceso
        procesos_ordenados[k] = *(actual->pcb);
        actual = actual->siguiente;
        k++;
    }

    // 2. Ordenar el array por PID usando qsort
    // Necesitas una función de comparación:
    // int comparar_por_pid(const void* a, const void* b);
    qsort(procesos_ordenados, num_procesos, sizeof(Proceso), comparar_por_pid);

    // 3. Imprimir el encabezado (como ya lo corregiste)
    printf("\nNone\n");
    printf("%-3s | %-7s | %-8s | %-6s | %-3s | %-9s | %-7s | %-7s\n",
           "PID", "Llegada", "Servicio", "Inicio", "Fin", "Respuesta", "Espera", "Retorno");
    printf("--- | ------- | -------- | ------ | --- | --------- | ------- | -------\n");

    // 4. Imprimir la tabla usando el array ordenado
    for (int i = 0; i < num_procesos; i++) {
        Proceso* p = &procesos_ordenados[i];
       
        // El cálculo de métricas es el mismo...
        p->respuesta = p->inicio - p->llegada;
        p->retorno = p->fin - p->llegada;
        p->espera = p->retorno - p->servicio;

        // ... Imprimir línea ...
        printf("%-3d | %-7d | %-8d | %-6d | %-3d | %-9d | %-7d | %-7d\n",
               p->pid, p->llegada, p->servicio, p->inicio, p->fin,
               p->respuesta, p->espera, p->retorno);
    }

    free(procesos_ordenados);
}
