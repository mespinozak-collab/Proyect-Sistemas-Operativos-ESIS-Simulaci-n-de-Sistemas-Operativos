// simulador.h - ARCHIVO MODIFICADO

#ifndef SIMULADOR_H
#define SIMULADOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

// =======================================================
// ESTRUCTURA DE PROCESO (PCB)
// =======================================================
typedef struct {
    int pid;
    int llegada;      // Tiempo de Arribo
    int servicio;     // CPU total requerido
    int tam_memoria;  // Memoria solicitada
   
    // Tiempos de ejecución y estado
    int restante;     // CPU restante
    int inicio;       // Primer Despacho
    int fin;          // Tiempo de Terminación
   
    // Métricas calculadas
    int respuesta;    
    int espera;      
    int retorno;      
   
    // Gestión de Memoria
    int dir_inicio_mem; // Dirección base asignada
    int tam_bloque_mem; // Tamaño del bloque asignado
} Proceso;

// =======================================================
// ESTRUCTURA DE CONFIGURACIÓN DEL SISTEMA
// =======================================================
typedef struct {
    char algoritmo_cpu[10];
    int quantum;
    int tam_memoria;        // Tamaño total de la memoria (en bytes)
    char estrategia_mem[10];
} Configuracion;

// =======================================================
// ESTRUCTURAS DE LISTAS
// =======================================================

// NODO PARA LA COLA DE LISTOS (Ready Queue)
typedef struct NodoProceso {
    Proceso* pcb;                  
    struct NodoProceso* siguiente;
} NodoProceso;

// Definición de un Bloque de Memoria
typedef struct BloqueMemoria {
    int inicio;            // Dirección de inicio del bloque
    int tamano;            // Tamaño actual del bloque
    int id_proceso;        // PID del proceso asignado, o -1 si está LIBRE
    struct BloqueMemoria* siguiente;
} BloqueMemoria;


// =======================================================
// DECLARACIÓN DE FUNCIONES GLOBALES (PROTOTIPOS)
// =======================================================

// Módulo 1: Lectura JSON (main.c)
int cargar_configuracion(const char* archivo, Proceso** procesos_out, int* num_procesos_out, Configuracion* conf_out);

// Módulo 2: Planificación y Métricas (simulador.c)
void iniciar_simulacion(Proceso procesos[], int num_procesos, Configuracion conf, BloqueMemoria** memoria_ptr);
void calcular_y_mostrar_metricas(NodoProceso* lista_terminados, int num_procesos, int tiempo_total);

// Módulo de Colas (simulador_colas.c)
NodoProceso* crear_nodo(Proceso* pcb);
void encolar(NodoProceso** cola_ptr, Proceso* pcb);
Proceso* desencolar(NodoProceso** cola_ptr);
void insertar_spn(NodoProceso** cola_ptr, Proceso* pcb);

// Módulo de Memoria (simulador_memoria.c)
BloqueMemoria* crear_bloque(int inicio, int tamano, int pid);
BloqueMemoria* inicializar_memoria(int tam_total);
int asignar_memoria(BloqueMemoria** memoria_ptr, Proceso* p, const char* estrategia);
void desasignar_memoria(BloqueMemoria** memoria_ptr, Proceso* p);

#endif // SIMULADOR_H
