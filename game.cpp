#include "game.h"

#include <allegro5/allegro_primitives.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>

using namespace std;

Tablero tablero = {
    7,        
    0, 0, 0,
    NULL, NULL
};

ModoJuego estado = MENU;

char listaGuardados[MAX_GUARDADOS][64];
int cantGuardados = 0;
char mensaje[128] = "";

//inicializa el tablero, libera el anterior si existia
bool inicializarTablero(int tamCel) {
    if (tamCel < CEL_MIN || tamCel > CEL_MAX) {
        snprintf(mensaje, sizeof(mensaje), "Tamano de celda fuera de rango.");
        return false;
    }

    int filas = ALTO_JUEGO / tamCel;
    int cols = ANCHO / tamCel;
    int total = filas * cols;

    //pedimos memoria para el estado actual y el siguiente
    unsigned char* cel = (unsigned char*) malloc(total);
    unsigned char* sig = (unsigned char*) malloc(total);

    //si falla la reserva, soltamos lo que hayamos conseguido
    if (!cel || !sig) {
        free(cel);
        free(sig);
        snprintf(mensaje, sizeof(mensaje), "No se pudo reservar memoria.");
        return false;
    }

    memset(cel, 0, total);
    memset(sig, 0, total);

    free(tablero.celulas);
    free(tablero.siguiente);

    tablero.tamCel = tamCel;
    tablero.filas = filas;
    tablero.cols = cols;
    tablero.gen = 0;
    tablero.celulas = cel;
    tablero.siguiente = sig;

    return true;
}

//libera la memoria del tablero
void liberarTablero() {
    free(tablero.celulas);
    free(tablero.siguiente);
    tablero.celulas = NULL;
    tablero.siguiente = NULL;
}

//pone todas las celulas a 0 y reinicia generacion
void vaciarTablero() {
    int total = tablero.filas * tablero.cols;
    for (int i = 0; i < total; i++) {
        tablero.celulas[i] = 0;
        tablero.siguiente[i] = 0;
    }
    tablero.gen = 0;
    snprintf(mensaje, sizeof(mensaje), "Mapa vaciado.");
}

//deja el tablero listo, vacio o con celdas al azar
void prepararTablero(bool azar) {
    vaciarTablero();

    if (azar) {
        int total = tablero.filas * tablero.cols;
        for (int i = 0; i < total; i++) {
            //20% de probabilidad de empezar viva
            tablero.celulas[i] = (rand() % 5 == 0);
        }
        snprintf(mensaje, sizeof(mensaje), "Mapa aleatorio creado.");
    } else {
        snprintf(mensaje, sizeof(mensaje), "Mapa vacio creado.");
    }
}

// cuenta los vecinos vivos de una posicion (f,c)
int vecinosVivos(int f, int c) {
    int total = 0;
    //miramos cada celda a su alrdedor
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            if (x == 0 && y == 0) continue;

            int nf = f + y;
            int nc = c + x;

            //que no se salga del mapa
            if (nf >= 0 && nf < tablero.filas && nc >= 0 && nc < tablero.cols) {
                total += tablero.celulas[pos(nf, nc)];
            }
        }
    }
    return total;
}

//devuelve la cantidad de celulas vivas en total
int totalVivas() {
    int vivas = 0;
    int total = tablero.filas * tablero.cols;
    for (int i = 0; i < total; i++) {
        vivas += tablero.celulas[i];
    }
    return vivas;
}

//aplica las reglas del juego de la vida
void actualizar() {
    for (int f = 0; f < tablero.filas; f++) {
        for (int c = 0; c < tablero.cols; c++) {
            int i = pos(f, c);
            int vec = vecinosVivos(f, c);

            if (tablero.celulas[i]) {
                tablero.siguiente[i] = (vec == 2 || vec == 3);
            } else {
                tablero.siguiente[i] = (vec == 3);
            }
        }
    }

    //Investigando, vimos que en vez de copiar el contenido de un arreglo al siguiente
    //es mas rapido intercabiar los punteros y cosume menos memoria
    unsigned char* aux = tablero.celulas;
    tablero.celulas = tablero.siguiente;
    tablero.siguiente = aux;

    tablero.gen++;
}

//lee la lista de archivos guardados que tenemos
bool cargarListaGuardados() {
    cantGuardados = 0;
    FILE* arch = fopen("saved_maps_list.txt", "r");
    if (!arch) {
        return false;
    }

    while (cantGuardados < MAX_GUARDADOS &&
           fscanf(arch, "%63s", listaGuardados[cantGuardados]) == 1) {
        cantGuardados++;
    }
    fclose(arch);
    return true;
}

//guarda el mapa actual en un archivo nuevo
bool guardarMapaActual() {
    if (cantGuardados >= MAX_GUARDADOS) {
        snprintf(mensaje, sizeof(mensaje), "Limite de guardados alcanzado.");
        return false;
    }

    char nombre[64];
    snprintf(nombre, sizeof(nombre), "guardado_%02d.txt", cantGuardados + 1);

    FILE* arch = fopen(nombre, "w");
    if (!arch) {
        snprintf(mensaje, sizeof(mensaje), "Error al guardar.");
        return false;
    }

    //guardamos tamCel, filas, cols
    fprintf(arch, "%d %d %d\n", tablero.tamCel, tablero.filas, tablero.cols);

    //guardamos las celdas como 0 y 1
    for (int f = 0; f < tablero.filas; f++) {
        for (int c = 0; c < tablero.cols; c++) {
            fprintf(arch, "%d", tablero.celulas[pos(f, c)] ? 1 : 0);
        }
        fprintf(arch, "\n");
    }
    fclose(arch);

    //agregamos el nombre a la lista de guardados
    FILE* lista = fopen("saved_maps_list.txt", "a");
    if (lista) {
        fprintf(lista, "%s\n", nombre);
        fclose(lista);
    }

    snprintf(listaGuardados[cantGuardados], sizeof(listaGuardados[cantGuardados]), "%s", nombre);
    cantGuardados++;
    snprintf(mensaje, sizeof(mensaje), "Guardado en %s", nombre);
    return true;
}

//carga un mapa desde un archivo
bool cargarMapa(const char* nombre) {
    FILE* arch = fopen(nombre, "r");
    if (!arch) {
        snprintf(mensaje, sizeof(mensaje), "No se pudo abrir %s.", nombre);
        return false;
    }

    int tam, filas, cols;
    //leemos tamCel, filas, cols
    if (fscanf(arch, "%d %d %d", &tam, &filas, &cols) != 3) {
        fclose(arch);
        snprintf(mensaje, sizeof(mensaje), "Archivo invalido.");
        return false;
    }

    if (!inicializarTablero(tam)) {
        fclose(arch);
        return false;
    }

    for (int f = 0; f < filas; f++) {
        for (int c = 0; c < cols; c++) {
            int valor = 0;
            fscanf(arch, " %1d", &valor);
            if (f < tablero.filas && c < tablero.cols) {
                tablero.celulas[pos(f, c)] = valor;
            }
        }
    }
    fclose(arch);
    tablero.gen = 0;
    snprintf(mensaje, sizeof(mensaje), "Cargado %s", nombre);
    return true;
}

//pantalla del menu principal
void dibujarMenu(ALLEGRO_FONT* fuente) {
    al_clear_to_color(al_map_rgb(20, 20, 25));

    al_draw_text(fuente, al_map_rgb(0, 220, 0), ANCHO / 2, 120,
                 ALLEGRO_ALIGN_CENTER, "JUEGO DE LA VIDA DE CONWAY");

    al_draw_text(fuente, al_map_rgb(255, 255, 255), ANCHO / 2, 200,
                 ALLEGRO_ALIGN_CENTER, "[ V ] Mapa vacio");

    al_draw_text(fuente, al_map_rgb(255, 255, 255), ANCHO / 2, 240,
                 ALLEGRO_ALIGN_CENTER, "[ A ] Mapa aleatorio");

    al_draw_text(fuente, al_map_rgb(255, 255, 255), ANCHO / 2, 280,
                 ALLEGRO_ALIGN_CENTER, "[ L ] Cargar mapa guardado");

    al_draw_text(fuente, al_map_rgb(255, 255, 255), ANCHO / 2, 320,
                 ALLEGRO_ALIGN_CENTER, "+ / - o flechas para cambiar zoom");

    char texto[100];
    snprintf(texto, sizeof(texto),
                  "Celda: %d px | Columnas: %d | Filas: %d",
                  tablero.tamCel, tablero.cols, tablero.filas);

    al_draw_text(fuente, al_map_rgb(180, 180, 180), ANCHO / 2, 360,
                 ALLEGRO_ALIGN_CENTER, texto);

    al_draw_text(fuente, al_map_rgb(120, 120, 120), ANCHO / 2, 520,
                 ALLEGRO_ALIGN_CENTER, "ESC para salir");
}

//menu de carga de mapas guardados
void dibujarCarga(ALLEGRO_FONT* fuente) {
    al_clear_to_color(al_map_rgb(20, 20, 25));

    al_draw_text(fuente, al_map_rgb(0, 220, 0), ANCHO / 2, 100,
                 ALLEGRO_ALIGN_CENTER, "CARGAR MAPA");

    if (cantGuardados == 0) {
        al_draw_text(fuente, al_map_rgb(255, 255, 255), ANCHO / 2, 240,
                     ALLEGRO_ALIGN_CENTER, "No hay archivos guardados.");
    } else {
        for (int i = 0; i < cantGuardados; i++) {
            char texto[100];
            snprintf(texto, sizeof(texto), "%d) %s", i + 1, listaGuardados[i]);
            al_draw_text(fuente, al_map_rgb(255, 255, 255), ANCHO / 2, 220 + i * 30,
                         ALLEGRO_ALIGN_CENTER, texto);
        }
    }

    al_draw_text(fuente, al_map_rgb(180, 180, 180), ANCHO / 2, 520,
                 ALLEGRO_ALIGN_CENTER, "[ B ] Volver al menu");
}

//renderiza el tablero del juego y muestra todos los datos
void dibujarTablero(ALLEGRO_FONT* fuente, bool pausa) {
    al_clear_to_color(al_map_rgb(0, 0, 0));

    //dibuja las celdas vivas
    for (int f = 0; f < tablero.filas; f++) {
        for (int c = 0; c < tablero.cols; c++) {
            int x = c * tablero.tamCel;
            int y = f * tablero.tamCel;

            if (tablero.celulas[pos(f, c)]) {
                al_draw_filled_rectangle(
                    x + 1, y + 1,
                    x + tablero.tamCel - 1, y + tablero.tamCel - 1,
                    al_map_rgb(0, 200, 0)
                );
            }

            al_draw_rectangle(
                x, y,
                x + tablero.tamCel, y + tablero.tamCel,
                al_map_rgb(45, 45, 45), 1
            );
        }
    }

    //barra gris de abajo
    al_draw_filled_rectangle(0, BARRA_Y, ANCHO, ALTO, al_map_rgb(25, 25, 25));
    al_draw_line(0, BARRA_Y, ANCHO, BARRA_Y, al_map_rgb(100, 100, 100), 2);

    char gen[50], vivas[50];
    snprintf(gen, sizeof(gen), "Generacion: %d", tablero.gen);
    snprintf(vivas, sizeof(vivas), "Vivas: %d", totalVivas());

    al_draw_text(fuente, al_map_rgb(255, 255, 255), 20, BARRA_Y + 20,
                 ALLEGRO_ALIGN_LEFT, gen);
    al_draw_text(fuente, al_map_rgb(255, 255, 255), 180, BARRA_Y + 20,
                 ALLEGRO_ALIGN_LEFT, vivas);

    if (pausa) {
        al_draw_text(fuente, al_map_rgb(230, 50, 50), 310, BARRA_Y + 20,
                     ALLEGRO_ALIGN_LEFT, "[ PAUSA ]");
        al_draw_text(fuente, al_map_rgb(200, 200, 50), 410, BARRA_Y + 20,
                     ALLEGRO_ALIGN_LEFT, "[ S ] Guardar");
    } else {
        al_draw_text(fuente, al_map_rgb(50, 200, 50), 310, BARRA_Y + 20,
                     ALLEGRO_ALIGN_LEFT, "[ CORRIENDO ]");
    }

    al_draw_text(fuente, al_map_rgb(200, 200, 50), 410, BARRA_Y + 45,
                 ALLEGRO_ALIGN_LEFT, "[ D ] Vaciar");

    if (mensaje[0] != '\0') {
        al_draw_text(fuente, al_map_rgb(180, 180, 180), 20, BARRA_Y + 45,
                     ALLEGRO_ALIGN_LEFT, mensaje);
    }

    //boton de menu
    al_draw_filled_rectangle(520, BARRA_Y + 15, 640, BARRA_Y + 45,
                             al_map_rgb(40, 90, 170));
    al_draw_rectangle(520, BARRA_Y + 15, 640, BARRA_Y + 45,
                      al_map_rgb(80, 160, 240), 2);
    al_draw_text(fuente, al_map_rgb(255, 255, 255), 580, BARRA_Y + 23,
                 ALLEGRO_ALIGN_CENTER, "Menu");

    //boton para cerrar
    al_draw_filled_rectangle(660, BARRA_Y + 15, 780, BARRA_Y + 45,
                             al_map_rgb(120, 20, 20));
    al_draw_rectangle(660, BARRA_Y + 15, 780, BARRA_Y + 45,
                      al_map_rgb(230, 50, 50), 2);
    al_draw_text(fuente, al_map_rgb(255, 255, 255), 720, BARRA_Y + 23,
                 ALLEGRO_ALIGN_CENTER, "Salir");
}