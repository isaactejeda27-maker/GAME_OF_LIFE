#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>  // Requerido para texto
#include <allegro5/allegro_ttf.h>   // Requerido para fuentes tipográficas
#include <cstdlib>
#include <ctime>
#include <cstdio> // Requerido para sprintf o snprintf

const int SCREEN_W = 900;
const int GAME_H = 600;      // Altura del área de juego
const int HUD_H = 600;       // Posición Y donde empieza la interfaz de datos
const int SCREEN_H = 660;    // Altura TOTAL de la ventana (60px extras para texto)
const int CELL_SIZE = 5;
const int COLS = SCREEN_W / CELL_SIZE;
const int ROWS = GAME_H / CELL_SIZE;

bool cells[ROWS][COLS];
bool next_cells[ROWS][COLS];
int generaciones = 0; // CONTADOR GLOBAL

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

// Cuenta cuántas células vivas hay actualmente en el mapa
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
    generaciones++; // Aumentar generación en cada ciclo
}

// Dibujar el mapa y la interfaz
void drawGrid(ALLEGRO_FONT* font, bool pausa) {
    al_clear_to_color(al_map_rgb(0, 0, 0));

    // 1. Dibujar el mapa de células
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            int x1 = col * CELL_SIZE;
            int y1 = row * CELL_SIZE;
            int x2 = x1 + CELL_SIZE;
            int y2 = y1 + CELL_SIZE;

            if (cells[row][col]) {
                al_draw_filled_rectangle(x1 + 1, y1 + 1, x2 - 1, y2 - 1, al_map_rgb(0, 200, 0));
            }
            al_draw_rectangle(x1, y1, x2, y2, al_map_rgb(45, 45, 45), 1);
        }
    }

    // 2. Dibujar la barra de fondo de la interfaz (Franja gris inferior)
    al_draw_filled_rectangle(0, HUD_H, SCREEN_W, SCREEN_H, al_map_rgb(25, 25, 25));
    al_draw_line(0, HUD_H, SCREEN_W, HUD_H, al_map_rgb(100, 100, 100), 2); // Línea divisoria

    // 3. Formatear y renderizar los textos
    char texto_gen[50];
    char texto_vivas[50];
    std::snprintf(texto_gen, sizeof(texto_gen), "Generacion: %d", generaciones);
    std::snprintf(texto_vivas, sizeof(texto_vivas), "Celulas Vivas: %d", contarCelulasVivas());

    // Renderizado de datos en la pantalla
    al_draw_text(font, al_map_rgb(255, 255, 255), 20, HUD_H + 20, ALLEGRO_ALIGN_LEFT, texto_gen);
    al_draw_text(font, al_map_rgb(255, 255, 255), 250, HUD_H + 20, ALLEGRO_ALIGN_LEFT, texto_vivas);

    // Estado del juego (Texto condicional de Pausa)
    if (pausa) {
        al_draw_text(font, al_map_rgb(230, 50, 50), SCREEN_W - 150, HUD_H + 20, ALLEGRO_ALIGN_LEFT, "[ PAUSADO ]");
    } else {
        al_draw_text(font, al_map_rgb(50, 200, 50), SCREEN_W - 150, HUD_H + 20, ALLEGRO_ALIGN_LEFT, "[ CORRIENDO ]");
    }
}

int main() {
    if (!al_init()) return -1;
    if (!al_init_primitives_addon()) return -1;
    if (!al_install_mouse()) return -1;
    if (!al_install_keyboard()) return -1;

    // INICIALIZAR TEXTO
    al_init_font_addon();
    if (!al_init_ttf_addon()) return -1;

    ALLEGRO_DISPLAY* display = al_create_display(SCREEN_W, SCREEN_H); // Ventana más grande
    if (!display) return -1;

    // Crear la fuente integrada de Allegro
    ALLEGRO_FONT* font = al_create_builtin_font();
    if (!font) return -1;

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 10.0);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();

    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_keyboard_event_source());

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            cells[row][col] = (std::rand() % 5 == 0);
        }
    }

    al_start_timer(timer);
    bool running = true;
    bool redraw = true;
    bool pausa = false;

    while (running) {
        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        } 
        else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (event.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                pausa = !pausa;
            }
            else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                running = false;
            }
            // Tecla 'C' para limpiar el mapa y reiniciar generaciones
            else if (event.keyboard.keycode == ALLEGRO_KEY_C) {
                generaciones = 0;
                for (int row = 0; row < ROWS; row++) {
                    for (int col = 0; col < COLS; col++) {
                        cells[row][col] = false;
                    }
                }
                redraw = true;
            }
        }
        else if (event.type == ALLEGRO_EVENT_TIMER) {
            if (!pausa) {
                updateGame();
            }
            redraw = true;
        } 
        else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || event.type == ALLEGRO_EVENT_MOUSE_AXES) {
            ALLEGRO_MOUSE_STATE mouse_state;
            al_get_mouse_state(&mouse_state);

            if (mouse_state.buttons & 1) {
                int col = mouse_state.x / CELL_SIZE;
                int row = mouse_state.y / CELL_SIZE;

                // Restringir el clic solo al área de juego (para no pintar sobre la interfaz)
                if (col >= 0 && col < COLS && row >= 0 && row < ROWS) {
                    cells[row][col] = true;
                    redraw = true;
                }
            }
        }

        if (redraw && al_is_event_queue_empty(queue)) {
            drawGrid(font, pausa); // Pasamos font y estado de pausa
            al_flip_display();
            redraw = false;
        }
    }

    al_destroy_font(font);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);
    return 0;
}
