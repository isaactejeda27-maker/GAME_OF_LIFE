#include "game.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <cstdlib>
#include <cstdio>

int cellSize = 7;
int COLS = SCREEN_W / cellSize;
int ROWS = GAME_H / cellSize;
EstadoSistema estado_actual = MENU;

bool cells[MAX_ROWS][MAX_COLS];
bool next_cells[MAX_ROWS][MAX_COLS];
int generaciones = 0;
char saveFiles[MAX_SAVE_FILES][64];
int saveFileCount = 0;
char lastMessage[128] = "";

void updateGridDimensions() {
    ROWS = GAME_H / cellSize;
    COLS = SCREEN_W / cellSize;
}

int countNeighbors(int row, int col) {
    int count = 0;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            if (x == 0 && y == 0) continue;
            int ny = row + y;
            int nx = col + x;
            if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS) {
                count += cells[ny][nx];
            }
        }
    }
    return count;
}

int contarCelulasVivas() {
    int vivas = 0;
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (cells[row][col]) vivas++;
        }
    }
    return vivas;
}

void updateGame() {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            int neighbors = countNeighbors(row, col);
            if (cells[row][col]) {
                next_cells[row][col] = (neighbors == 2 || neighbors == 3);
            } else {
                next_cells[row][col] = (neighbors == 3);
            }
        }
    }
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            cells[row][col] = next_cells[row][col];
        }
    }
    generaciones++;
}

void inicializarTablero(bool aleatorio) {
    generaciones = 0;
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (aleatorio) {
                cells[row][col] = (std::rand() % 5 == 0);
            } else {
                cells[row][col] = false;
            }
        }
    }
}

void limpiarTablero() {
    generaciones = 0;
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            cells[row][col] = false;
        }
    }
    std::snprintf(lastMessage, sizeof(lastMessage), "Mapa vaciado.");
}

bool cargarListaGuardados() {
    saveFileCount = 0;
    FILE* arch = fopen("saved_maps_list.txt", "r");
    if (!arch) {
        return false;
    }
    while (saveFileCount < MAX_SAVE_FILES && fscanf(arch, "%63s", saveFiles[saveFileCount]) == 1) {
        saveFileCount++;
    }
    fclose(arch);
    return true;
}

bool guardarPatronActual() {
    if (saveFileCount >= MAX_SAVE_FILES) {
        std::snprintf(lastMessage, sizeof(lastMessage), "Limite de archivos guardados alcanzado.");
        return false;
    }
    char fileName[64];
    std::snprintf(fileName, sizeof(fileName), "guardado_%02d.txt", saveFileCount + 1);

    FILE* arch = fopen(fileName, "w");
    if (!arch) {
        std::snprintf(lastMessage, sizeof(lastMessage), "Error al abrir %s para guardar.", fileName);
        return false;
    }

    fprintf(arch, "%d %d %d\n", cellSize, ROWS, COLS);
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            fprintf(arch, "%d", cells[row][col] ? 1 : 0);
        }
        fprintf(arch, "\n");
    }
    fclose(arch);

    FILE* lista = fopen("saved_maps_list.txt", "a");
    if (lista) {
        fprintf(lista, "%s\n", fileName);
        fclose(lista);
    }

    std::snprintf(saveFiles[saveFileCount], sizeof(saveFiles[saveFileCount]), "%s", fileName);
    saveFileCount++;
    std::snprintf(lastMessage, sizeof(lastMessage), "Guardado en %s", fileName);
    return true;
}

bool cargarPatron(const char* fileName) {
    FILE* arch = fopen(fileName, "r");
    if (!arch) {
        std::snprintf(lastMessage, sizeof(lastMessage), "No se pudo abrir %s.", fileName);
        return false;
    }
    int savedSize = 0;
    int savedRows = 0;
    int savedCols = 0;
    if (fscanf(arch, "%d %d %d", &savedSize, &savedRows, &savedCols) != 3) {
        fclose(arch);
        std::snprintf(lastMessage, sizeof(lastMessage), "Formato incorrecto en %s.", fileName);
        return false;
    }
    cellSize = savedSize;
    updateGridDimensions();
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            cells[row][col] = false;
        }
    }
    for (int row = 0; row < savedRows && row < ROWS; row++) {
        for (int col = 0; col < savedCols && col < COLS; col++) {
            int valor = 0;
            if (fscanf(arch, "%1d", &valor) != 1) {
                valor = 0;
            }
            cells[row][col] = (valor == 1);
        }
        int ch;
        while ((ch = fgetc(arch)) != EOF && ch != '\n');
    }
    fclose(arch);
    generaciones = 0;
    std::snprintf(lastMessage, sizeof(lastMessage), "Cargado %s", fileName);
    return true;
}

void drawMenu(ALLEGRO_FONT* font) {
    al_clear_to_color(al_map_rgb(20, 20, 25));
    al_draw_text(font, al_map_rgb(0, 220, 0), SCREEN_W / 2, 120, ALLEGRO_ALIGN_CENTER, "CONWAY'S GAME OF LIFE");
    al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_W / 2, 200, ALLEGRO_ALIGN_CENTER, "Presiona [ V ] para Mapa VACIO");
    al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_W / 2, 240, ALLEGRO_ALIGN_CENTER, "Presiona [ A ] para Mapa ALEATORIO");
    al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_W / 2, 280, ALLEGRO_ALIGN_CENTER, "Presiona [ L ] para Cargar mapa guardado");
    al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_W / 2, 320, ALLEGRO_ALIGN_CENTER, "Presiona + / - o Flechas ARRIBA/ABAJO para cambiar el zoom");
    char texto_zoom[60];
    std::snprintf(texto_zoom, sizeof(texto_zoom), "Tamaño de celda: %d px  (Columnas: %d, Filas: %d)", cellSize, COLS, ROWS);
    al_draw_text(font, al_map_rgb(180, 180, 180), SCREEN_W / 2, 360, ALLEGRO_ALIGN_CENTER, texto_zoom);
    al_draw_text(font, al_map_rgb(120, 120, 120), SCREEN_W / 2, 520, ALLEGRO_ALIGN_CENTER, "Presiona ESC para salir");
}

void drawLoadMenu(ALLEGRO_FONT* font) {
    al_clear_to_color(al_map_rgb(20, 20, 25));
    al_draw_text(font, al_map_rgb(0, 220, 0), SCREEN_W / 2, 100, ALLEGRO_ALIGN_CENTER, "CARGAR MAPA GUARDADO");
    if (saveFileCount == 0) {
        al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_W / 2, 240, ALLEGRO_ALIGN_CENTER, "No hay archivos guardados.");
    } else {
        for (int i = 0; i < saveFileCount; i++) {
            char line[128];
            std::snprintf(line, sizeof(line), "%d) %s", i + 1, saveFiles[i]);
            al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_W / 2, 220 + i * 30, ALLEGRO_ALIGN_CENTER, line);
        }
    }
    al_draw_text(font, al_map_rgb(180, 180, 180), SCREEN_W / 2, 520, ALLEGRO_ALIGN_CENTER, "Presiona B para volver al menu principal");
}

void drawGrid(ALLEGRO_FONT* font, bool pausa) {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            int x1 = col * cellSize;
            int y1 = row * cellSize;
            if (cells[row][col]) {
                al_draw_filled_rectangle(x1 + 1, y1 + 1, x1 + cellSize - 1, y1 + cellSize - 1, al_map_rgb(0, 200, 0));
            }
            al_draw_rectangle(x1, y1, x1 + cellSize, y1 + cellSize, al_map_rgb(45, 45, 45), 1);
        }
    }
    al_draw_filled_rectangle(0, HUD_H, SCREEN_W, SCREEN_H, al_map_rgb(25, 25, 25));
    al_draw_line(0, HUD_H, SCREEN_W, HUD_H, al_map_rgb(100, 100, 100), 2);
    char texto_gen[50];
    char texto_vivas[50];
    std::snprintf(texto_gen, sizeof(texto_gen), "Generacion: %d", generaciones);
    std::snprintf(texto_vivas, sizeof(texto_vivas), "Celulas Vivas: %d", contarCelulasVivas());
    al_draw_text(font, al_map_rgb(255, 255, 255), 20, HUD_H + 20, ALLEGRO_ALIGN_LEFT, texto_gen);
    al_draw_text(font, al_map_rgb(255, 255, 255), 220, HUD_H + 20, ALLEGRO_ALIGN_LEFT, texto_vivas);
    if (pausa) {
        al_draw_text(font, al_map_rgb(230, 50, 50), 410, HUD_H + 20, ALLEGRO_ALIGN_LEFT, "[ PAUSA - Espacio/Enter ]");
        al_draw_text(font, al_map_rgb(200, 200, 50), 410, HUD_H + 40, ALLEGRO_ALIGN_LEFT, "Presiona [ S ] para guardar el patron");
    } else {
        al_draw_text(font, al_map_rgb(50, 200, 50), 410, HUD_H + 20, ALLEGRO_ALIGN_LEFT, "[ CORRIENDO ]");
    }
    al_draw_text(font, al_map_rgb(200, 200, 50), 410, HUD_H + 50, ALLEGRO_ALIGN_LEFT, "Presiona [ D ] para vaciar el mapa");
    if (lastMessage[0] != '\0') {
        al_draw_text(font, al_map_rgb(180, 180, 180), 20, HUD_H + 45, ALLEGRO_ALIGN_LEFT, lastMessage);
    }
    al_draw_filled_rectangle(520, HUD_H + 15, 640, HUD_H + 45, al_map_rgb(40, 90, 170));
    al_draw_rectangle(520, HUD_H + 15, 640, HUD_H + 45, al_map_rgb(80, 160, 240), 2);
    al_draw_text(font, al_map_rgb(255, 255, 255), 580, HUD_H + 23, ALLEGRO_ALIGN_CENTER, "Menu (C)");
    al_draw_filled_rectangle(660, HUD_H + 15, 780, HUD_H + 45, al_map_rgb(120, 20, 20));
    al_draw_rectangle(660, HUD_H + 15, 780, HUD_H + 45, al_map_rgb(230, 50, 50), 2);
    al_draw_text(font, al_map_rgb(255, 255, 255), 720, HUD_H + 23, ALLEGRO_ALIGN_CENTER, "Salir (ESC)");
}
