#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

//medidas de la pantalla
const int ANCHO = 900;
const int ALTO_JUEGO = 600;
const int BARRA_Y = 600;
const int ALTO = 660;

//tamano maximo y minimo de las celdas
const int CEL_MIN = 3;
const int CEL_MAX = 20;

// aximo de archivos que se pueden guardar/cargar
const int MAX_GUARDADOS = 9;

//estados en los que puede estar el programa
enum ModoJuego {
 
    MENU,
    JUEGO,
    CARGAR
};

//estructura del tablero
struct Tablero {
    
    int tamCel;    
    int filas;    
    int cols;      
    int gen;  

    unsigned char* celulas;    //estado actual de las celdas (1 vivo, 0 muerto)
    unsigned char* siguiente;  //buffer para la siguiente generacion
};

//Variables globales, sabemos que en teoria hay que evitarlas
//pero para este proyecto lo hicimos asi porque de otra forma no pudimos
extern Tablero tablero;
extern ModoJuego estado;

extern char listaGuardados[MAX_GUARDADOS][64];
extern int cantGuardados;
extern char mensaje[128];

//pasa las coordenadas de cada fila y columna a la posicion del arreglo de celdas
inline int pos(int f, int c) {
    
    return f * tablero.cols + c;
}

//funciones del juego
bool inicializarTablero(int tamCel);
void liberarTablero();
void prepararTablero(bool azar);
void vaciarTablero();

//logica del juego
int vecinosVivos(int f, int c);
int totalVivas();
void actualizar();

//guardado y carga de mapas
bool cargarListaGuardados();
bool guardarMapaActual();
bool cargarMapa(const char* nombre);

//dibujado de las distintas pantallas
void dibujarMenu(ALLEGRO_FONT* fuente);
void dibujarCarga(ALLEGRO_FONT* fuente);
void dibujarTablero(ALLEGRO_FONT* fuente, bool pausa);