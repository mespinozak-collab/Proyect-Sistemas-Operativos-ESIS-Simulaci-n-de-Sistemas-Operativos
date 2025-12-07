// main.c 

#include "simulador.h" // Incluimos nuestras estructuras

// Función de ayuda para obtener un entero de un objeto JSON
int get_json_int(json_t* obj, const char* key) {
    json_t* val = json_object_get(obj, key);
    return json_is_integer(val) ? (int)json_integer_value(val) : -1;
}

// Dentro de la función que prepara la simulación (antes de llamar a iniciar_simulacion)
void preparar_simulacion(Proceso procesos[], int num_procesos, Configuracion conf) {
    // 1. Inicializar la memoria
    BloqueMemoria* memoria_principal = inicializar_memoria(conf.tam_memoria);

    // 2. Asignar memoria a los procesos antes de iniciar el scheduler
    for (int i = 0; i < num_procesos; i++) {
        if (procesos[i].tam_memoria > 0) {
            asignar_memoria(&memoria_principal, &procesos[i], conf.estrategia_mem);
        }
    }
   
    // 3. Iniciar la planificación de CPU (CORRECCIÓN: Descomentado y con argumento de memoria)
    iniciar_simulacion(procesos, num_procesos, conf, &memoria_principal);
}

// Función para cargar toda la configuración y procesos desde el JSON
int cargar_configuracion(const char* archivo, Proceso** procesos_out, int* num_procesos_out, Configuracion* conf_out) {

    json_error_t error;
    json_t* root = json_load_file(archivo, 0, &error);

    if (!root) {
        fprintf(stderr, "Error al leer JSON en línea %d: %s\n", error.line, error.text);
        return 0;
    }
   
    // 1. Cargar Configuración de CPU
    json_t* cpu = json_object_get(root, "cpu");
    if (json_is_object(cpu)) {
        strncpy(conf_out->algoritmo_cpu, json_string_value(json_object_get(cpu, "algoritmo")), 9);
        conf_out->quantum = get_json_int(cpu, "quantum");
    }

    // 2. Cargar Configuración de Memoria
    json_t* memoria = json_object_get(root, "memoria");
    if (json_is_object(memoria)) {
        conf_out->tam_memoria = get_json_int(memoria, "tam");
        strncpy(conf_out->estrategia_mem, json_string_value(json_object_get(memoria, "estrategia")), 9);
    }

    // 3. Cargar Procesos
    json_t* procesos_json = json_object_get(root, "procesos");
    if (!json_is_array(procesos_json)) {
        fprintf(stderr, "Error: 'procesos' no es un arreglo.\n");
        json_decref(root);
        return 0;
    }

    *num_procesos_out = json_array_size(procesos_json);
    *procesos_out = (Proceso*)calloc(*num_procesos_out, sizeof(Proceso));
   
    for (int i = 0; i < *num_procesos_out; i++) {
        json_t* p_json = json_array_get(procesos_json, i);
        Proceso* p = &(*procesos_out)[i];

        p->pid = get_json_int(p_json, "pid");
        p->llegada = get_json_int(p_json, "llegada");
        p->servicio = get_json_int(p_json, "servicio");
       
        // Inicialización de campos de la simulación
        p->restante = p->servicio;
        p->inicio = -1; // Marcador de que aún no ha iniciado
        p->fin = -1;
        p->tam_memoria = 0; // Se actualiza después
    }

    // 4. Cargar Solicitudes de Memoria (y actualizar los procesos correspondientes)
    json_t* sol_mem_json = json_object_get(root, "solicitudes_mem");
    if (json_is_array(sol_mem_json)) {
        int num_solicitudes = json_array_size(sol_mem_json);
        for (int i = 0; i < num_solicitudes; i++) {
            json_t* sol_json = json_array_get(sol_mem_json, i);
            int pid_sol = get_json_int(sol_json, "pid");
            int tam_sol = get_json_int(sol_json, "tam");

            // Buscar el proceso y actualizar su campo de memoria
            for (int j = 0; j < *num_procesos_out; j++) {
                if ((*procesos_out)[j].pid == pid_sol) {
                    (*procesos_out)[j].tam_memoria = tam_sol;
                    break;
                }
            }
        }
    }

    json_decref(root); // Liberar el objeto JSON
    return 1;
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <archivo_json_config>\n", argv[0]);
        return 1;
    }

    Proceso* procesos = NULL;
    int num_procesos = 0;
    Configuracion conf;

    if (cargar_configuracion(argv[1], &procesos, &num_procesos, &conf)) {
       
        // CORRECCIÓN: Se comentan todos los mensajes de configuración para ajustarse al formato de salida
        /*
        printf("Configuración Cargada con Éxito:\n");
        printf("  CPU: %s (Quantum: %d)\n", conf.algoritmo_cpu, conf.quantum);
        printf("  Memoria: %d MiB (Estrategia: %s)\n", conf.tam_memoria / 1024 / 1024, conf.estrategia_mem);
        printf("  Total Procesos: %d\n", num_procesos);
       
        // Muestra de datos cargados:
        for (int i = 0; i < num_procesos; i++) {
            printf("  P%d: Llega=%d, Servicio=%d, Memoria=%d\n",
                       procesos[i].pid,
                       procesos[i].llegada,
                       procesos[i].servicio,
                       procesos[i].tam_memoria);
        }
        */

        // Llamar a la función de simulación
        preparar_simulacion(procesos, num_procesos, conf);

        free(procesos);
    }

    return 0;
}
