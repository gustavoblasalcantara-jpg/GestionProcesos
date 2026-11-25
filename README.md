# Sistema de Gestión de Procesos en un Sistema Operativo

Proyecto grupal del curso **Estructura de Datos** – Universidad Continental (Huancayo, 2026).
NRC **30967** · Docente: **Ing. Miguel Angel Casimiro Bravo**

Simulador por consola que administra procesos como lo haría un sistema operativo, usando
**solo estructuras de datos dinámicas lineales programadas desde cero** (sin STL):

| Módulo | Estructura | Operaciones |
|---|---|---|
| Gestor de procesos | Lista enlazada simple | insertar, buscar por PID o nombre, modificar prioridad, cambiar estado, eliminar |
| Planificador de CPU | Cola de prioridad (lista ordenada) | encolar por prioridad, desencolar y ejecutar, simulación con tiempos de espera y retorno, buscar, bloquear |
| Gestor de memoria | Pila (LIFO) | push, pop, consultar tope, buscar memoria de un proceso, liberar con pila auxiliar |

Además: modelo de 5 estados (NUEVO, LISTO, EJECUCIÓN, BLOQUEADO, TERMINADO), validación de todas las
entradas y **persistencia** en `procesos.txt` y `memoria.txt`. Usa solo temas del curso (registros, cadenas, punteros, listas, colas y pilas).

## Cómo ejecutarlo

1. Abrir `codigo/SistemaGestionProcesos.cpp` en **Dev-C++**.
2. Presionar **F11** (Compilar y ejecutar).
3. En el menú principal, la opción **5** carga datos de ejemplo.


## Estructura del repositorio

```
codigo/        Código fuente en C++
docs/          Informe técnico (PDF) y diagramas (PNG + fuente Graphviz)
presentacion/  Diapositivas de la exposición
```

## Integrantes y aportes

| Integrante | Rol | Rama |
|---|---|---|
| Gustavo Blas Alcántara | Coordinador · Lista enlazada | `feature/gestor-procesos` |
| Jhanpool Bueno Yancce Taype | Planificador de CPU | `feature/planificador-cpu` |
| Manuel Jeanpier Lule Luna | Gestor de memoria | `feature/gestor-memoria` |
| Gustavo Brayan Mallasca Huamani | Integración, pruebas y documentación | `feature/persistencia-docs` |
