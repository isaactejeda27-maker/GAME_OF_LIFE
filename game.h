#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

const int SCREEN_W = 900;
const int GAME_H = 600;
const int HUD_H = 600;
const int SCREEN_H = 660; // 60 píxeles extra para la barra de datos inferior
const int MIN_CELL_SIZE = 3;
const int MAX_CELL_SIZE = 20;
extern int cellSize;
extern int COLS;
extern int ROWS;
const int MAX_ROWS = GAME_H / MIN_CELL_SIZE;
const int MAX_COLS = SCREEN_W / MIN_CELL_SIZE;

enum EstadoSistema { MENU, JUEGO, CARGAR };
extern EstadoSistema estado_actual;

extern bool cells[MAX_ROWS][MAX_COLS];
extern bool next_cells[MAX_ROWS][MAX_COLS];
extern int generaciones;
const int MAX_SAVE_FILES = 10;
extern char saveFiles[MAX_SAVE_FILES][64];
extern int saveFileCount;
extern char lastMessage[128];

void updateGridDimensions();
int countNeighbors(int row, int col);
int contarCelulasVivas();
void updateGame();
void inicializarTablero(bool aleatorio);
void limpiarTablero();
bool cargarListaGuardados();
bool guardarPatronActual();
bool cargarPatron(const char* fileName);
void drawMenu(ALLEGRO_FONT* font);
void drawLoadMenu(ALLEGRO_FONT* font);
void drawGrid(ALLEGRO_FONT* font, bool pausa);
