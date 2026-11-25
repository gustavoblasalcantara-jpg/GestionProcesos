// =====================================================
// UNIVERSIDAD CONTINENTAL - ESTRUCTURA DE DATOS
// PROYECTO: SISTEMA DE GESTION DE PROCESOS
//
// Estructuras utilizadas (implementadas desde cero):
//   - Lista enlazada   -> Gestor de procesos
//   - Cola de prioridad -> Planificador de CPU
//   - Pila             -> Gestor de memoria
// =====================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <windows.h>
#include <cstdlib>
#include <conio.h>

using namespace std;

const int MEMORIA_TOTAL = 1024;   // memoria total del sistema en MB

// =====================================================
// ESTRUCTURAS
// =====================================================

// Nodo de la LISTA ENLAZADA (cada proceso registrado)
struct Proceso {
    int id;
    string nombre;
    int prioridad;        // 1 = mas alta ... 5 = mas baja
    int rafaga;           // tiempo que necesita la CPU
    string estado;        // NUEVO, LISTO, EJECUCION, BLOQUEADO, TERMINADO
    Proceso *siguiente;
};

// Nodo de la COLA DE PRIORIDAD (procesos listos para usar la CPU)
struct NodoCola {
    int id;
    string nombre;
    int prioridad;
    NodoCola *siguiente;
};

// Nodo de la PILA (bloques de memoria asignados)
struct BloqueMemoria {
    int idProceso;
    int tamanio;          // en MB
    BloqueMemoria *siguiente;
};

// =====================================================
// COLORES Y PANTALLA
// =====================================================

void color(int c) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void limpiar() {
    system("cls");
}

void pausa() {
    cout << "\n";
    color(8);
    cout << "Presione una tecla para continuar...";
    color(7);
    getch();
}

void encabezado(string titulo) {

    color(11);
    cout << "\n";
    cout << "==============================================================\n";

    color(14);
    cout << "     GESTION DE PROCESOS - " << titulo << "\n";

    color(11);
    cout << "==============================================================\n";

    color(7);
}

// =====================================================
// MENSAJES
// =====================================================

void mensajeExito(string mensaje) {

    color(10);
    cout << "\n";
    cout << "+------------------------------------------+\n";
    cout << "|             OPERACION EXITOSA            |\n";
    cout << "+------------------------------------------+\n";
    cout << "| " << setw(40) << left << mensaje << " |\n";
    cout << "+------------------------------------------+\n";
    color(7);
}

void mensajeError(string mensaje) {

    color(12);
    cout << "\n";
    cout << "+------------------------------------------+\n";
    cout << "|                  ERROR                   |\n";
    cout << "+------------------------------------------+\n";
    cout << "| " << setw(40) << left << mensaje << " |\n";
    cout << "+------------------------------------------+\n";
    color(7);
}

void mensajeAdvertencia(string mensaje) {

    color(14);
    cout << "\n";
    cout << "+------------------------------------------+\n";
    cout << "|               ADVERTENCIA                |\n";
    cout << "+------------------------------------------+\n";
    cout << "| " << setw(40) << left << mensaje << " |\n";
    cout << "+------------------------------------------+\n";
    color(7);
}

// =====================================================
// LEER NUMERO (validacion)
// Si el usuario escribe letras, cin falla: se limpia el
// error, se descarta lo escrito y se vuelve a pedir.
// =====================================================

int leerNumero() {

    int numero;
    cin >> numero;                       // intenta leer un numero entero

    while (cin.fail()) {                 // fallo: el usuario escribio letras
        cin.clear();                     // quita el estado de error de cin
        cin.ignore(10000, '\n');         // descarta lo que se escribio
        color(12);
        cout << "  Error: ingrese solo numeros. Intente de nuevo: ";
        color(7);
        cin >> numero;                   // vuelve a pedir el numero
    }

    cin.ignore(10000, '\n');             // limpia el ENTER que queda en el buffer
    return numero;                       // devuelve un numero valido
}

// =====================================================
// LISTA ENLAZADA - GESTOR DE PROCESOS
// =====================================================

// Busca un proceso por su ID (busqueda secuencial)
Proceso *buscarProceso(Proceso *cabeza, int id) {

    Proceso *aux = cabeza;               // empieza desde el primer nodo

    while (aux != NULL) {                // recorre hasta el final de la lista
        if (aux->id == id)               // si el ID coincide...
            return aux;                  // ...devuelve el nodo encontrado
        aux = aux->siguiente;            // avanza al siguiente nodo
    }

    return NULL;                         // no se encontro el proceso
}

// Inserta un proceso al final de la lista
void insertarProceso(Proceso *&cabeza, int id, string nombre,
                     int prioridad, int rafaga, string estado) {

    Proceso *nuevo = new Proceso;        // crea un nuevo nodo en memoria
    nuevo->id = id;                      // guarda los datos del proceso
    nuevo->nombre = nombre;
    nuevo->prioridad = prioridad;
    nuevo->rafaga = rafaga;
    nuevo->estado = estado;
    nuevo->siguiente = NULL;             // sera el ultimo: no apunta a nadie

    if (cabeza == NULL) {                // Caso 1: la lista esta vacia
        cabeza = nuevo;                  // el nuevo nodo es el primero
    } else {                             // Caso 2: ya hay nodos
        Proceso *aux = cabeza;
        while (aux->siguiente != NULL) { // busca el ultimo nodo
            aux = aux->siguiente;
        }
        aux->siguiente = nuevo;          // el ultimo ahora apunta al nuevo
    }
}

// Elimina un proceso de la lista por su ID
bool eliminarDeLista(Proceso *&cabeza, int id) {

    Proceso *aux = cabeza;               // nodo que se revisa
    Proceso *anterior = NULL;            // nodo que esta antes de aux

    while (aux != NULL && aux->id != id) {   // busca el nodo a eliminar
        anterior = aux;                  // guarda el anterior
        aux = aux->siguiente;            // avanza
    }

    if (aux == NULL)                     // llego al final sin encontrarlo
        return false;                    // no existe

    if (anterior == NULL)                // era el primero de la lista
        cabeza = aux->siguiente;         // la cabeza pasa al segundo nodo
    else                                 // estaba en medio o al final
        anterior->siguiente = aux->siguiente;   // el anterior "salta" el nodo

    delete aux;                          // libera la memoria del nodo
    return true;                         // se elimino correctamente
}

// Cuenta cuantos procesos hay en la lista
int contarProcesos(Proceso *cabeza) {

    int total = 0;                       // contador de nodos
    Proceso *aux = cabeza;

    while (aux != NULL) {                // recorre toda la lista
        total++;                         // cuenta el nodo actual
        aux = aux->siguiente;
    }

    return total;
}

// =====================================================
// COLA DE PRIORIDAD - PLANIFICADOR DE CPU
// El frente siempre tiene la prioridad mas alta (1).
// Si tienen la misma prioridad, se respeta el orden
// de llegada.
// =====================================================

void encolar(NodoCola *&frente, NodoCola *&final,
             int id, string nombre, int prioridad) {

    NodoCola *nuevo = new NodoCola;      // crea el nodo de la cola
    nuevo->id = id;                      // copia los datos del proceso
    nuevo->nombre = nombre;
    nuevo->prioridad = prioridad;
    nuevo->siguiente = NULL;

    // Caso 1: cola vacia
    if (frente == NULL) {
        frente = nuevo;                  // es el primero...
        final = nuevo;                   // ...y tambien el ultimo
        return;
    }

    // Caso 2: tiene mas prioridad que el frente -> va primero
    if (prioridad < frente->prioridad) {
        nuevo->siguiente = frente;       // el nuevo apunta al antiguo frente
        frente = nuevo;                  // el nuevo pasa a ser el frente
        return;
    }

    // Caso 3: buscar su lugar en medio o al final
    NodoCola *aux = frente;
    while (aux->siguiente != NULL &&
           aux->siguiente->prioridad <= prioridad) {   // avanza mientras el siguiente
        aux = aux->siguiente;                          // tenga igual o mayor prioridad
    }

    nuevo->siguiente = aux->siguiente;   // el nuevo apunta al que seguia
    aux->siguiente = nuevo;              // aux ahora apunta al nuevo

    if (nuevo->siguiente == NULL)        // si no hay nadie despues...
        final = nuevo;                   // ...el nuevo es el ultimo
}

// Saca el proceso del frente y devuelve su ID
bool desencolar(NodoCola *&frente, NodoCola *&final, int &id) {

    if (frente == NULL)                  // cola vacia
        return false;                    // no hay nada que sacar

    NodoCola *aux = frente;              // guarda el nodo del frente
    id = aux->id;                        // devuelve su ID por referencia
    frente = frente->siguiente;          // el frente pasa al siguiente

    if (frente == NULL)                  // si la cola quedo vacia...
        final = NULL;                    // ...final tambien es NULL

    delete aux;                          // libera el nodo atendido
    return true;
}

// Retira un proceso especifico de la cola
bool quitarDeCola(NodoCola *&frente, NodoCola *&final, int id) {

    NodoCola *aux = frente;
    NodoCola *anterior = NULL;

    while (aux != NULL && aux->id != id) {   // busca el proceso en la cola
        anterior = aux;
        aux = aux->siguiente;
    }

    if (aux == NULL)                     // no esta en la cola
        return false;

    if (anterior == NULL)                // era el frente
        frente = aux->siguiente;
    else                                 // estaba en medio o al final
        anterior->siguiente = aux->siguiente;

    if (aux == final)                    // si era el ultimo...
        final = anterior;                // ...el anterior pasa a ser el final

    delete aux;                          // libera el nodo
    return true;
}

// Cuenta cuantos procesos hay en la cola
int contarCola(NodoCola *frente) {

    int total = 0;
    NodoCola *aux = frente;

    while (aux != NULL) {                // recorre la cola desde el frente
        total++;
        aux = aux->siguiente;
    }

    return total;
}

// Devuelve la posicion de un proceso en la cola (0 si no esta)
int posicionEnCola(NodoCola *frente, int id) {

    int posicion = 1;                    // el frente es la posicion 1
    NodoCola *aux = frente;

    while (aux != NULL) {
        if (aux->id == id)               // lo encontro
            return posicion;
        aux = aux->siguiente;            // avanza y suma una posicion
        posicion++;
    }

    return 0;                            // no esta en la cola
}

// =====================================================
// PILA - GESTOR DE MEMORIA
// Solo se trabaja por el TOPE.
// =====================================================

bool pilaVacia(BloqueMemoria *tope) {
    return tope == NULL;                 // vacia si el tope no apunta a nada
}

// PUSH: agrega un bloque en el tope
void push(BloqueMemoria *&tope, int idProceso, int tamanio) {

    BloqueMemoria *nuevo = new BloqueMemoria;   // crea el bloque
    nuevo->idProceso = idProceso;        // proceso duenio del bloque
    nuevo->tamanio = tamanio;            // tamanio en MB
    nuevo->siguiente = tope;             // el nuevo apunta al antiguo tope
    tope = nuevo;                        // el nuevo pasa a ser el tope
}

// POP: quita el bloque del tope y devuelve sus datos
bool pop(BloqueMemoria *&tope, int &idProceso, int &tamanio) {

    if (pilaVacia(tope))                 // no hay bloques
        return false;

    BloqueMemoria *aux = tope;           // guarda el bloque del tope
    idProceso = aux->idProceso;          // devuelve sus datos por referencia
    tamanio = aux->tamanio;
    tope = tope->siguiente;              // el tope baja al siguiente bloque

    delete aux;                          // libera el bloque retirado
    return true;
}

// Suma la memoria de todos los bloques
int memoriaUsada(BloqueMemoria *tope) {

    int total = 0;
    BloqueMemoria *aux = tope;

    while (aux != NULL) {                // recorre la pila desde el tope
        total += aux->tamanio;           // acumula el tamanio de cada bloque
        aux = aux->siguiente;
    }

    return total;
}

// Suma la memoria que tiene un proceso
int memoriaDeProceso(BloqueMemoria *tope, int idProceso) {

    int total = 0;
    BloqueMemoria *aux = tope;

    while (aux != NULL) {
        if (aux->idProceso == idProceso) // solo los bloques de ese proceso
            total += aux->tamanio;
        aux = aux->siguiente;
    }

    return total;
}

// Libera todos los bloques de un proceso usando una PILA AUXILIAR:
// 1) se hace pop de toda la pila; los bloques del proceso se
//    descartan y los demas se guardan en la pila auxiliar.
// 2) se devuelven los bloques de la auxiliar a la pila original.
int liberarMemoriaDeProceso(BloqueMemoria *&tope, int idProceso) {

    BloqueMemoria *auxiliar = NULL;      // pila auxiliar (empieza vacia)
    int id, tamanio;
    int liberado = 0;                    // MB liberados

    while (pop(tope, id, tamanio)) {     // 1) saca todos los bloques
        if (id == idProceso)             // es del proceso: se descarta
            liberado += tamanio;
        else                             // es de otro: se guarda
            push(auxiliar, id, tamanio);
    }

    while (pop(auxiliar, id, tamanio)) { // 2) devuelve los bloques
        push(tope, id, tamanio);         //    en el mismo orden original
    }

    return liberado;
}

// =====================================================
// ESTADOS DEL PROCESO
// =====================================================

// Indica si se permite pasar del estado actual al nuevo
bool transicionValida(string estadoActual, string nuevo) {

    if (estadoActual == "NUEVO")         // NUEVO -> LISTO o TERMINADO
        return nuevo == "LISTO" || nuevo == "TERMINADO";

    if (estadoActual == "LISTO")         // LISTO -> EJECUCION, BLOQUEADO o TERMINADO
        return nuevo == "EJECUCION" || nuevo == "BLOQUEADO" || nuevo == "TERMINADO";

    if (estadoActual == "EJECUCION")     // EJECUCION -> TERMINADO
        return nuevo == "TERMINADO";

    if (estadoActual == "BLOQUEADO")     // BLOQUEADO -> LISTO o TERMINADO
        return nuevo == "LISTO" || nuevo == "TERMINADO";

    return false;                        // TERMINADO no cambia
}

// Cambia el estado y actualiza la cola y la pila
bool cambiarEstado(Proceso *p, string nuevo, NodoCola *&frente,
                   NodoCola *&final, BloqueMemoria *&tope) {

    if (!transicionValida(p->estado, nuevo)) {   // cambio no permitido
        mensajeError("Cambio de estado no permitido.");
        cout << "  " << p->estado << " -> " << nuevo << "\n";
        return false;
    }

    // Si sale de LISTO (y no es para ejecutarse), se quita de la cola
    if (p->estado == "LISTO" && nuevo != "EJECUCION")
        quitarDeCola(frente, final, p->id);

    // Si entra a LISTO, se encola segun su prioridad
    if (nuevo == "LISTO")
        encolar(frente, final, p->id, p->nombre, p->prioridad);

    // Si termina, se libera su memoria
    if (nuevo == "TERMINADO") {
        int liberado = liberarMemoriaDeProceso(tope, p->id);
        if (liberado > 0)
            cout << "\n  Memoria liberada: " << liberado << " MB\n";
    }

    cout << "\n  Estado del proceso " << p->id << ": "
         << p->estado << " -> " << nuevo << "\n";

    p->estado = nuevo;                   // guarda el nuevo estado
    return true;
}

// =====================================================
// MOSTRAR DATOS
// =====================================================

void cabeceraTabla() {

    color(11);
    cout << "+------+----------------------+-----------+--------+-----------+---------+\n";
    cout << "| ID   | NOMBRE               | PRIORIDAD | RAFAGA | ESTADO    | MEMORIA |\n";
    cout << "+------+----------------------+-----------+--------+-----------+---------+\n";
    color(7);
}

void filaProceso(Proceso *p, BloqueMemoria *tope) {

    if (p->estado == "TERMINADO")
        color(8);
    else if (p->estado == "BLOQUEADO")
        color(12);
    else
        color(7);

    cout << "| " << setw(4) << left << p->id
         << " | " << setw(20) << left << p->nombre.substr(0, 20)
         << " | " << setw(9) << right << p->prioridad
         << " | " << setw(6) << right << p->rafaga
         << " | " << setw(9) << left << p->estado
         << " | " << setw(4) << right << memoriaDeProceso(tope, p->id) << " MB"
         << " |\n";

    color(7);
}

void lineaTabla() {
    color(11);
    cout << "+------+----------------------+-----------+--------+-----------+---------+\n";
    color(7);
}

// =====================================================
// OPCIONES DEL GESTOR DE PROCESOS
// =====================================================

void registrarProceso(Proceso *&cabeza, NodoCola *&frente,
                      NodoCola *&final, BloqueMemoria *&tope) {

    limpiar();
    encabezado("REGISTRAR PROCESO");

    int id, prioridad, rafaga;
    string nombre;

    cout << "\n";
    color(14);
    cout << "ID del proceso: ";
    color(7);
    id = leerNumero();                   // lee el ID validando que sea numero

    if (id <= 0) {                       // el ID debe ser positivo
        mensajeError("El ID debe ser mayor que cero.");
        pausa();
        return;
    }

    if (buscarProceso(cabeza, id) != NULL) {   // el ID no se puede repetir
        mensajeError("El ID ya se encuentra registrado.");
        pausa();
        return;
    }

    cout << "Nombre del proceso: ";
    getline(cin, nombre);                // getline permite nombres con espacios

    if (nombre == "") {                  // el nombre no puede estar vacio
        mensajeError("El nombre no puede estar vacio.");
        pausa();
        return;
    }

    cout << "Prioridad (1 = mas alta ... 5 = mas baja): ";
    prioridad = leerNumero();

    if (prioridad < 1 || prioridad > 5) {   // rango permitido: 1 a 5
        mensajeError("La prioridad debe ser de 1 a 5.");
        pausa();
        return;
    }

    cout << "Rafaga de CPU (1 a 50): ";
    rafaga = leerNumero();

    if (rafaga < 1 || rafaga > 50) {     // rango permitido: 1 a 50
        mensajeError("La rafaga debe ser de 1 a 50.");
        pausa();
        return;
    }

    insertarProceso(cabeza, id, nombre, prioridad, rafaga, "NUEVO");   // inserta al final de la lista
    mensajeExito("Proceso registrado como NUEVO.");

    cout << "\nDesea enviarlo a la cola de listos? (1 = Si, 0 = No): ";
    int respuesta = leerNumero();

    if (respuesta == 1) {                // se envia a la cola de listos
        Proceso *p = buscarProceso(cabeza, id);
        cambiarEstado(p, "LISTO", frente, final, tope);   // NUEVO -> LISTO (se encola)
        cout << "  Posicion en la cola: " << posicionEnCola(frente, id) << "\n";
    }

    pausa();
}

void mostrarProcesos(Proceso *cabeza, BloqueMemoria *tope) {

    limpiar();
    encabezado("LISTA DE PROCESOS");

    if (cabeza == NULL) {
        mensajeAdvertencia("No existen procesos registrados.");
        pausa();
        return;
    }

    cabeceraTabla();

    Proceso *aux = cabeza;               // recorre la lista desde la cabeza
    while (aux != NULL) {
        filaProceso(aux, tope);          // muestra una fila por proceso
        aux = aux->siguiente;
    }

    lineaTabla();

    cout << "\nTotal de procesos: " << contarProcesos(cabeza) << "\n";

    // Dibujo de los nodos enlazados
    color(11);
    cout << "\ncabeza -> ";
    aux = cabeza;
    while (aux != NULL) {
        cout << "[" << aux->id << "] -> ";
        aux = aux->siguiente;
    }
    cout << "NULL\n";
    color(7);

    pausa();
}

void buscarPorId(Proceso *cabeza, NodoCola *frente, BloqueMemoria *tope) {

    limpiar();
    encabezado("BUSCAR PROCESO POR ID");

    cout << "\nIngrese ID: ";
    int id = leerNumero();

    Proceso *p = buscarProceso(cabeza, id);

    if (p == NULL) {
        mensajeError("Proceso no encontrado.");
    } else {
        cout << "\n";
        cabeceraTabla();
        filaProceso(p, tope);
        lineaTabla();

        int pos = posicionEnCola(frente, id);
        if (pos > 0)
            cout << "\nEsta en la cola de listos, posicion " << pos << ".\n";
    }

    pausa();
}

void buscarPorNombre(Proceso *cabeza, BloqueMemoria *tope) {

    limpiar();
    encabezado("BUSCAR PROCESO POR NOMBRE");

    string texto;
    cout << "\nIngrese el nombre o parte del nombre: ";
    getline(cin, texto);

    bool encontrado = false;
    Proceso *aux = cabeza;

    while (aux != NULL) {

        if (aux->nombre.find(texto) != string::npos) {   // el texto esta dentro del nombre
            if (!encontrado) {
                cout << "\n";
                cabeceraTabla();
            }
            filaProceso(aux, tope);
            encontrado = true;
        }

        aux = aux->siguiente;
    }

    if (encontrado)
        lineaTabla();
    else
        mensajeAdvertencia("No se encontraron coincidencias.");

    pausa();
}

void modificarPrioridad(Proceso *cabeza, NodoCola *&frente, NodoCola *&final) {

    limpiar();
    encabezado("MODIFICAR PRIORIDAD");

    cout << "\nIngrese ID: ";
    int id = leerNumero();

    Proceso *p = buscarProceso(cabeza, id);

    if (p == NULL) {
        mensajeError("Proceso no encontrado.");
        pausa();
        return;
    }

    if (p->estado == "TERMINADO") {
        mensajeError("El proceso ya termino.");
        pausa();
        return;
    }

    cout << "Prioridad actual: " << p->prioridad << "\n";
    cout << "Nueva prioridad (1 a 5): ";
    int nueva = leerNumero();

    if (nueva < 1 || nueva > 5) {
        mensajeError("La prioridad debe ser de 1 a 5.");
        pausa();
        return;
    }

    p->prioridad = nueva;                // actualiza el dato en la lista

    // Si esta en la cola, se saca y se vuelve a encolar en su nuevo lugar
    if (p->estado == "LISTO") {
        quitarDeCola(frente, final, id); // se saca de la cola...
        encolar(frente, final, id, p->nombre, nueva);   // ...y se vuelve a encolar en su nuevo lugar
        cout << "\nNueva posicion en la cola: " << posicionEnCola(frente, id) << "\n";
    }

    mensajeExito("Prioridad actualizada.");
    pausa();
}

void opcionCambiarEstado(Proceso *cabeza, NodoCola *&frente,
                         NodoCola *&final, BloqueMemoria *&tope) {

    limpiar();
    encabezado("CAMBIAR ESTADO");

    cout << "\nIngrese ID: ";
    int id = leerNumero();

    Proceso *p = buscarProceso(cabeza, id);

    if (p == NULL) {
        mensajeError("Proceso no encontrado.");
        pausa();
        return;
    }

    cout << "Estado actual: " << p->estado << "\n\n";
    cout << "1. LISTO      (entra a la cola)\n";
    cout << "2. BLOQUEADO  (sale de la cola)\n";
    cout << "3. TERMINADO  (libera su memoria)\n";
    cout << "\nNuevo estado: ";
    int opcion = leerNumero();

    switch (opcion) {
        case 1: cambiarEstado(p, "LISTO", frente, final, tope);     break;
        case 2: cambiarEstado(p, "BLOQUEADO", frente, final, tope); break;
        case 3: cambiarEstado(p, "TERMINADO", frente, final, tope); break;
        default: mensajeError("Opcion no valida.");
    }

    pausa();
}

void eliminarProceso(Proceso *&cabeza, NodoCola *&frente,
                     NodoCola *&final, BloqueMemoria *&tope) {

    limpiar();
    encabezado("ELIMINAR PROCESO");

    cout << "\nIngrese ID: ";
    int id = leerNumero();

    Proceso *p = buscarProceso(cabeza, id);

    if (p == NULL) {
        mensajeError("Proceso no encontrado.");
        pausa();
        return;
    }

    cout << "Proceso: " << p->nombre << " (" << p->estado << ")\n";
    cout << "Confirmar eliminacion (1 = Si, 0 = No): ";

    if (leerNumero() != 1) {
        mensajeAdvertencia("Operacion cancelada.");
        pausa();
        return;
    }

    // Se elimina de las tres estructuras
    quitarDeCola(frente, final, id);                  // 1) sale de la cola
    int liberado = liberarMemoriaDeProceso(tope, id); // 2) libera su memoria
    eliminarDeLista(cabeza, id);                      // 3) se borra de la lista

    if (liberado > 0)
        cout << "\nMemoria liberada: " << liberado << " MB\n";

    mensajeExito("Proceso eliminado.");
    pausa();
}

// =====================================================
// OPCIONES DEL PLANIFICADOR DE CPU
// =====================================================

void mostrarCola(NodoCola *frente) {

    limpiar();
    encabezado("COLA DE LISTOS");

    if (frente == NULL) {
        mensajeAdvertencia("La cola esta vacia.");
        pausa();
        return;
    }

    color(11);
    cout << "+-----+------+----------------------+-----------+\n";
    cout << "| POS | ID   | NOMBRE               | PRIORIDAD |\n";
    cout << "+-----+------+----------------------+-----------+\n";
    color(7);

    int pos = 1;
    NodoCola *aux = frente;

    while (aux != NULL) {
        cout << "| " << setw(3) << right << pos
             << " | " << setw(4) << left << aux->id
             << " | " << setw(20) << left << aux->nombre.substr(0, 20)
             << " | " << setw(9) << right << aux->prioridad
             << " |\n";
        aux = aux->siguiente;
        pos++;
    }

    color(11);
    cout << "+-----+------+----------------------+-----------+\n";

    cout << "\nfrente -> ";
    aux = frente;
    while (aux != NULL) {
        cout << "[" << aux->id << "|P" << aux->prioridad << "] -> ";
        aux = aux->siguiente;
    }
    cout << "NULL\n";
    color(7);

    pausa();
}

void encolarProceso(Proceso *cabeza, NodoCola *&frente,
                    NodoCola *&final, BloqueMemoria *&tope) {

    limpiar();
    encabezado("ENCOLAR PROCESO");

    cout << "\nIngrese ID: ";
    int id = leerNumero();

    Proceso *p = buscarProceso(cabeza, id);

    if (p == NULL) {
        mensajeError("Proceso no encontrado.");
        pausa();
        return;
    }

    if (p->estado == "LISTO") {
        mensajeAdvertencia("El proceso ya esta en la cola.");
        pausa();
        return;
    }

    if (cambiarEstado(p, "LISTO", frente, final, tope))
        cout << "  Posicion en la cola: " << posicionEnCola(frente, id) << "\n";

    pausa();
}

void buscarEnCola(Proceso *cabeza, NodoCola *frente) {

    limpiar();
    encabezado("BUSCAR EN LA COLA");

    cout << "\nIngrese ID: ";
    int id = leerNumero();

    int pos = posicionEnCola(frente, id);

    if (pos == 0) {
        mensajeError("El proceso no esta en la cola.");
    } else {
        Proceso *p = buscarProceso(cabeza, id);
        cout << "\n" << p->nombre << " esta en la posicion " << pos << ".\n";
        cout << "Se ejecutara despues de " << pos - 1 << " proceso(s).\n";
    }

    pausa();
}

// Desencola el proceso de mayor prioridad y lo ejecuta
void ejecutarSiguiente(Proceso *cabeza, NodoCola *&frente,
                       NodoCola *&final, BloqueMemoria *&tope) {

    limpiar();
    encabezado("EJECUTAR SIGUIENTE PROCESO");

    int id;

    if (!desencolar(frente, final, id)) {   // saca el de mayor prioridad
        mensajeAdvertencia("No hay procesos en la cola.");
        pausa();
        return;
    }

    Proceso *p = buscarProceso(cabeza, id); // busca sus datos en la lista
    p->estado = "EJECUCION";             // LISTO -> EJECUCION

    cout << "\nEjecutando: [" << p->id << "] " << p->nombre
         << "  (prioridad " << p->prioridad << ")\n\n";

    color(10);
    cout << "CPU: ";
    for (int i = 1; i <= p->rafaga; i++) {   // una marca por unidad de tiempo
        cout << "|";
    }
    cout << " " << p->rafaga << " unidades de tiempo\n";
    color(7);

    cambiarEstado(p, "TERMINADO", frente, final, tope);   // termina y libera su memoria
    mensajeExito("Proceso ejecutado y terminado.");
    pausa();
}

// Ejecuta toda la cola y calcula tiempos de espera y retorno
// Tiempo de espera  = suma de las rafagas anteriores
// Tiempo de retorno = tiempo de espera + rafaga
void ejecutarTodos(Proceso *cabeza, NodoCola *&frente,
                   NodoCola *&final, BloqueMemoria *&tope) {

    limpiar();
    encabezado("SIMULACION DEL PLANIFICADOR");

    if (frente == NULL) {
        mensajeAdvertencia("No hay procesos en la cola.");
        pausa();
        return;
    }

    int id;
    int tiempo = 0;                      // reloj de la CPU
    int cantidad = 0;
    int sumaEspera = 0;                  // suma de tiempos de espera
    int sumaRetorno = 0;                 // suma de tiempos de retorno

    color(11);
    cout << "+-----+------+----------------------+-----+--------+--------+---------+\n";
    cout << "| ORD | ID   | NOMBRE               | PRI | RAFAGA | ESPERA | RETORNO |\n";
    cout << "+-----+------+----------------------+-----+--------+--------+---------+\n";
    color(7);

    while (desencolar(frente, final, id)) {   // atiende la cola en orden de prioridad

        Proceso *p = buscarProceso(cabeza, id);

        int espera = tiempo;             // espero lo que duraron los anteriores
        tiempo = tiempo + p->rafaga;     // usa la CPU durante su rafaga
        int retorno = tiempo;            // retorno = espera + rafaga

        sumaEspera += espera;
        sumaRetorno += retorno;
        cantidad++;

        cout << "| " << setw(3) << right << cantidad
             << " | " << setw(4) << left << p->id
             << " | " << setw(20) << left << p->nombre.substr(0, 20)
             << " | " << setw(3) << right << p->prioridad
             << " | " << setw(6) << right << p->rafaga
             << " | " << setw(6) << right << espera
             << " | " << setw(7) << right << retorno
             << " |\n";

        p->estado = "TERMINADO";         // el proceso termina
        liberarMemoriaDeProceso(tope, p->id);   // y libera su memoria
    }

    color(11);
    cout << "+-----+------+----------------------+-----+--------+--------+---------+\n";
    color(14);

    cout << "\nProcesos ejecutados        : " << cantidad << "\n";
    cout << "Tiempo total de CPU        : " << tiempo << "\n";
    cout << "Tiempo de espera promedio  : " << fixed << setprecision(2)
         << (double)sumaEspera / cantidad << "\n";    // (double): division con decimales
    cout << "Tiempo de retorno promedio : " << fixed << setprecision(2)
         << (double)sumaRetorno / cantidad << "\n";
    color(7);

    mensajeExito("Todos los procesos terminaron.");
    pausa();
}

void bloquearProceso(Proceso *cabeza, NodoCola *&frente,
                     NodoCola *&final, BloqueMemoria *&tope) {

    limpiar();
    encabezado("BLOQUEAR PROCESO");

    cout << "\nIngrese ID: ";
    int id = leerNumero();

    Proceso *p = buscarProceso(cabeza, id);

    if (p == NULL || p->estado != "LISTO") {
        mensajeError("El proceso no esta en la cola.");
        pausa();
        return;
    }

    cambiarEstado(p, "BLOQUEADO", frente, final, tope);
    pausa();
}

// =====================================================
// OPCIONES DEL GESTOR DE MEMORIA
// =====================================================

void mostrarPila(BloqueMemoria *tope) {

    limpiar();
    encabezado("ESTADO DE LA MEMORIA");

    if (pilaVacia(tope)) {
        mensajeAdvertencia("La memoria esta libre.");
    } else {
        BloqueMemoria *aux = tope;

        color(11);
        cout << "\n           +-------------------------+\n";
        while (aux != NULL) {

            if (aux == tope) {
                color(14);
                cout << "  TOPE ->  ";
            } else {
                cout << "           ";
            }

            color(15);
            cout << "| Proceso " << setw(4) << left << aux->idProceso
                 << " | " << setw(4) << right << aux->tamanio << " MB |\n";
            color(11);
            cout << "           +-------------------------+\n";

            aux = aux->siguiente;
        }
        color(7);
    }

    int usada = memoriaUsada(tope);

    cout << "\nMemoria usada : " << usada << " / " << MEMORIA_TOTAL << " MB\n";
    cout << "Memoria libre : " << MEMORIA_TOTAL - usada << " MB\n";

    pausa();
}

void asignarMemoria(Proceso *cabeza, BloqueMemoria *&tope) {

    limpiar();
    encabezado("ASIGNAR MEMORIA (PUSH)");

    int libre = MEMORIA_TOTAL - memoriaUsada(tope);   // memoria disponible

    if (libre == 0) {
        mensajeError("La memoria esta llena.");
        pausa();
        return;
    }

    cout << "\nID del proceso: ";
    int id = leerNumero();

    Proceso *p = buscarProceso(cabeza, id);

    if (p == NULL) {
        mensajeError("Proceso no encontrado.");
        pausa();
        return;
    }

    if (p->estado == "TERMINADO") {
        mensajeError("El proceso ya termino.");
        pausa();
        return;
    }

    cout << "Memoria libre: " << libre << " MB\n";
    cout << "Tamanio del bloque (MB): ";
    int tamanio = leerNumero();

    if (tamanio <= 0 || tamanio > libre) {   // no puede superar la memoria libre
        mensajeError("Tamanio no valido.");
        pausa();
        return;
    }

    push(tope, id, tamanio);             // apila el bloque en el tope
    mensajeExito("Bloque asignado en el tope.");
    pausa();
}

void liberarUltimoBloque(BloqueMemoria *&tope) {

    limpiar();
    encabezado("LIBERAR MEMORIA (POP)");

    int id, tamanio;

    if (!pop(tope, id, tamanio)) {       // pila vacia: no hay nada que liberar
        mensajeError("La pila esta vacia.");
        pausa();
        return;
    }

    cout << "\nSe libero el bloque del proceso " << id
         << " (" << tamanio << " MB)\n";

    mensajeExito("Bloque liberado.");
    pausa();
}

void consultarTope(BloqueMemoria *tope) {

    limpiar();
    encabezado("CONSULTAR TOPE DE LA PILA");

    if (pilaVacia(tope)) {
        mensajeAdvertencia("La pila esta vacia.");
    } else {
        cout << "\nBloque en el tope: proceso " << tope->idProceso
             << ", " << tope->tamanio << " MB\n";
        cout << "Es el proximo bloque que se libera con pop.\n";
    }

    pausa();
}

void buscarMemoria(BloqueMemoria *tope) {

    limpiar();
    encabezado("BUSCAR MEMORIA DE UN PROCESO");

    cout << "\nID del proceso: ";
    int id = leerNumero();

    int total = memoriaDeProceso(tope, id);

    if (total == 0)
        mensajeAdvertencia("El proceso no tiene memoria.");
    else
        cout << "\nEl proceso " << id << " tiene " << total << " MB asignados.\n";

    pausa();
}

void liberarMemoriaProceso(BloqueMemoria *&tope) {

    limpiar();
    encabezado("LIBERAR MEMORIA DE UN PROCESO");

    cout << "\nID del proceso: ";
    int id = leerNumero();

    int liberado = liberarMemoriaDeProceso(tope, id);

    if (liberado == 0) {
        mensajeAdvertencia("El proceso no tiene memoria.");
    } else {
        cout << "\nSe liberaron " << liberado << " MB usando una pila auxiliar.\n";
        mensajeExito("Memoria liberada.");
    }

    pausa();
}

// =====================================================
// ARCHIVOS - GUARDAR Y CARGAR
// procesos.txt : id prioridad rafaga estado nombre
// memoria.txt  : idProceso tamanio (del tope al fondo)
// La cola se vuelve a armar con los procesos LISTO.
// =====================================================

void guardarDatos(Proceso *cabeza, BloqueMemoria *tope) {

    ofstream archivo("procesos.txt");    // crea (o reemplaza) el archivo

    Proceso *aux = cabeza;
    while (aux != NULL) {                // una linea por cada proceso
        archivo << aux->id << " " << aux->prioridad << " "
                << aux->rafaga << " " << aux->estado << " "
                << aux->nombre << "\n";
        aux = aux->siguiente;
    }
    archivo.close();                     // cierra el archivo

    ofstream archivoMemoria("memoria.txt");

    BloqueMemoria *bloque = tope;
    while (bloque != NULL) {             // un bloque por linea, del tope al fondo
        archivoMemoria << bloque->idProceso << " " << bloque->tamanio << "\n";
        bloque = bloque->siguiente;
    }
    archivoMemoria.close();
}

void cargarDatos(Proceso *&cabeza, NodoCola *&frente,
                 NodoCola *&final, BloqueMemoria *&tope) {

    ifstream archivo("procesos.txt");

    if (!archivo) {                      // si el archivo no existe
        return;                                // no hay datos guardados
    }

    int id, prioridad, rafaga;
    string estado, nombre;

    while (archivo >> id >> prioridad >> rafaga >> estado) {   // lee mientras haya datos

        getline(archivo, nombre);              // el resto de la linea
        if (nombre.length() > 0)
            nombre = nombre.substr(1);         // quita el espacio inicial

        insertarProceso(cabeza, id, nombre, prioridad, rafaga, estado);   // lo agrega a la lista

        if (estado == "LISTO")             // los LISTO vuelven a la cola
            encolar(frente, final, id, nombre, prioridad);
    }
    archivo.close();

    // memoria.txt esta del tope al fondo: se apila en una pila
    // auxiliar (queda invertida) y luego se pasa a la pila real.
    ifstream archivoMemoria("memoria.txt");
    BloqueMemoria *auxiliar = NULL;
    int tamanio;

    while (archivoMemoria >> id >> tamanio) {   // apila en la auxiliar
        push(auxiliar, id, tamanio);
    }
    archivoMemoria.close();

    while (pop(auxiliar, id, tamanio)) { // pasa a la pila real (queda en orden)
        push(tope, id, tamanio);
    }
}

// Libera todos los nodos antes de salir
void liberarTodo(Proceso *&cabeza, NodoCola *&frente,
                 NodoCola *&final, BloqueMemoria *&tope) {

    int id, tamanio;

    while (frente != NULL) {             // vacia la cola
        desencolar(frente, final, id);
    }

    while (tope != NULL) {               // vacia la pila
        pop(tope, id, tamanio);
    }

    while (cabeza != NULL) {             // vacia la lista
        eliminarDeLista(cabeza, cabeza->id);
    }
}

// Datos de ejemplo para la demostracion
void cargarEjemplo(Proceso *&cabeza, NodoCola *&frente,
                   NodoCola *&final, BloqueMemoria *&tope) {

    limpiar();
    encabezado("CARGAR DATOS DE EJEMPLO");

    liberarTodo(cabeza, frente, final, tope);

    insertarProceso(cabeza, 100, "explorer.exe",  1, 4,  "LISTO");
    insertarProceso(cabeza, 101, "chrome.exe",    3, 10, "LISTO");
    insertarProceso(cabeza, 102, "antivirus.exe", 2, 6,  "LISTO");
    insertarProceso(cabeza, 103, "spotify.exe",   5, 8,  "LISTO");
    insertarProceso(cabeza, 104, "word.exe",      3, 5,  "LISTO");
    insertarProceso(cabeza, 105, "devcpp.exe",    2, 7,  "NUEVO");

    encolar(frente, final, 100, "explorer.exe",  1);
    encolar(frente, final, 101, "chrome.exe",    3);
    encolar(frente, final, 102, "antivirus.exe", 2);
    encolar(frente, final, 103, "spotify.exe",   5);
    encolar(frente, final, 104, "word.exe",      3);

    push(tope, 100, 128);
    push(tope, 101, 256);
    push(tope, 102, 64);
    push(tope, 103, 96);
    push(tope, 104, 128);
    push(tope, 105, 160);

    mensajeExito("6 procesos de ejemplo cargados.");
    pausa();
}

// =====================================================
// MENUS
// =====================================================

int menuPrincipal(Proceso *cabeza, NodoCola *frente, BloqueMemoria *tope) {

    limpiar();

    color(11);
    cout << "\n";
    cout << "================================================================\n";
    color(14);
    cout << "               SISTEMA DE GESTION DE PROCESOS\n";
    color(11);
    cout << "================================================================\n";

    color(15);
    cout << "  Procesos: " << contarProcesos(cabeza)
         << "   |   En cola: " << contarCola(frente)
         << "   |   Memoria: " << memoriaUsada(tope) << "/" << MEMORIA_TOTAL << " MB\n";

    cout << "\n";
    cout << "     +------------------------------+\n";
    color(10);
    cout << "     |  1. GESTOR DE PROCESOS       |  (lista enlazada)\n";
    color(9);
    cout << "     |  2. PLANIFICADOR DE CPU      |  (cola de prioridad)\n";
    color(13);
    cout << "     |  3. GESTOR DE MEMORIA        |  (pila)\n";
    color(15);
    cout << "     +------------------------------+\n";
    cout << "     |  4. GUARDAR DATOS            |\n";
    cout << "     |  5. CARGAR DATOS DE EJEMPLO  |\n";
    color(8);
    cout << "     |  0. GUARDAR Y SALIR          |\n";
    color(15);
    cout << "     +------------------------------+\n";

    color(14);
    cout << "\n     Seleccione una opcion: ";
    color(15);

    int opcion = leerNumero();
    color(7);
    return opcion;
}

void menuProcesos(Proceso *&cabeza, NodoCola *&frente,
                  NodoCola *&final, BloqueMemoria *&tope) {

    int opcion;

    do {
        limpiar();
        encabezado("GESTOR DE PROCESOS (LISTA)");

        color(15);
        cout << "\n";
        cout << "  1. Registrar proceso\n";
        cout << "  2. Mostrar procesos\n";
        cout << "  3. Buscar por ID\n";
        cout << "  4. Buscar por nombre\n";
        cout << "  5. Modificar prioridad\n";
        cout << "  6. Cambiar estado\n";
        cout << "  7. Eliminar proceso\n";
        cout << "  0. Volver\n";

        color(14);
        cout << "\n  Seleccione una opcion: ";
        color(7);
        opcion = leerNumero();

        switch (opcion) {
            case 1: registrarProceso(cabeza, frente, final, tope);    break;
            case 2: mostrarProcesos(cabeza, tope);                  break;
            case 3: buscarPorId(cabeza, frente, tope);              break;
            case 4: buscarPorNombre(cabeza, tope);                  break;
            case 5: modificarPrioridad(cabeza, frente, final);        break;
            case 6: opcionCambiarEstado(cabeza, frente, final, tope); break;
            case 7: eliminarProceso(cabeza, frente, final, tope);     break;
            case 0: break;
            default:
                mensajeError("Opcion no valida.");
                pausa();
        }

    } while (opcion != 0);
}

void menuPlanificador(Proceso *cabeza, NodoCola *&frente,
                      NodoCola *&final, BloqueMemoria *&tope) {

    int opcion;

    do {
        limpiar();
        encabezado("PLANIFICADOR DE CPU (COLA)");

        color(15);
        cout << "\n";
        cout << "  1. Encolar proceso\n";
        cout << "  2. Ejecutar siguiente proceso\n";
        cout << "  3. Ejecutar todos (simulacion)\n";
        cout << "  4. Ver cola de listos\n";
        cout << "  5. Buscar proceso en la cola\n";
        cout << "  6. Bloquear proceso\n";
        cout << "  0. Volver\n";

        color(14);
        cout << "\n  Seleccione una opcion: ";
        color(7);
        opcion = leerNumero();

        switch (opcion) {
            case 1: encolarProceso(cabeza, frente, final, tope);    break;
            case 2: ejecutarSiguiente(cabeza, frente, final, tope); break;
            case 3: ejecutarTodos(cabeza, frente, final, tope);     break;
            case 4: mostrarCola(frente);                          break;
            case 5: buscarEnCola(cabeza, frente);                 break;
            case 6: bloquearProceso(cabeza, frente, final, tope);   break;
            case 0: break;
            default:
                mensajeError("Opcion no valida.");
                pausa();
        }

    } while (opcion != 0);
}

void menuMemoria(Proceso *cabeza, BloqueMemoria *&tope) {

    int opcion;

    do {
        limpiar();
        encabezado("GESTOR DE MEMORIA (PILA)");

        color(15);
        cout << "\n";
        cout << "  1. Asignar memoria (push)\n";
        cout << "  2. Liberar ultimo bloque (pop)\n";
        cout << "  3. Ver estado de la memoria\n";
        cout << "  4. Consultar tope de la pila\n";
        cout << "  5. Buscar memoria de un proceso\n";
        cout << "  6. Liberar memoria de un proceso\n";
        cout << "  0. Volver\n";

        color(14);
        cout << "\n  Seleccione una opcion: ";
        color(7);
        opcion = leerNumero();

        switch (opcion) {
            case 1: asignarMemoria(cabeza, tope);  break;
            case 2: liberarUltimoBloque(tope);     break;
            case 3: mostrarPila(tope);             break;
            case 4: consultarTope(tope);                 break;
            case 5: buscarMemoria(tope);           break;
            case 6: liberarMemoriaProceso(tope);   break;
            case 0: break;
            default:
                mensajeError("Opcion no valida.");
                pausa();
        }

    } while (opcion != 0);
}

// =====================================================
// PROGRAMA PRINCIPAL
// =====================================================

int main() {

    Proceso *cabeza = NULL;          // lista enlazada
    NodoCola *frente = NULL;         // cola de prioridad
    NodoCola *final = NULL;
    BloqueMemoria *tope = NULL;      // pila

    cargarDatos(cabeza, frente, final, tope);   // recupera la sesion anterior

    int opcion;

    do {                                 // repite el menu hasta elegir 0
        opcion = menuPrincipal(cabeza, frente, tope);

        switch (opcion) {

            case 1:
                menuProcesos(cabeza, frente, final, tope);
                break;

            case 2:
                menuPlanificador(cabeza, frente, final, tope);
                break;

            case 3:
                menuMemoria(cabeza, tope);
                break;

            case 4:
                guardarDatos(cabeza, tope);
                mensajeExito("Datos guardados.");
                pausa();
                break;

            case 5:
                cargarEjemplo(cabeza, frente, final, tope);
                break;

            case 0:
                guardarDatos(cabeza, tope);      // guarda antes de salir
                liberarTodo(cabeza, frente, final, tope);

                limpiar();
                color(10);
                cout << "\n";
                cout << "====================================================\n";
                cout << "   Datos guardados. Gracias por usar el sistema.\n";
                cout << "====================================================\n";
                color(7);
                break;

            default:
                mensajeError("Opcion no valida.");
                pausa();
        }

    } while (opcion != 0);

    return 0;
}
