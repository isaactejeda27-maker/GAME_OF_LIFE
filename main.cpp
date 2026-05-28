#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <cstdlib>
#include <ctime>

const int SCREEN_W = 800;
const int SCREEN_H = 660;
const int CELL_SIZE = 5;
const int COLS = SCREEN_W / CELL_SIZE;
const int ROWS = SCREEN_H / CELL_SIZE;

bool cells[ROWS][COLS];
bool next_cells[ROWS][COLS];

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
}

void drawGrid() {
    al_clear_to_color(al_map_rgb(0, 0, 0));

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            int x1 = col * CELL_SIZE;
            int y1 = row * CELL_SIZE;
            int x2 = x1 + CELL_SIZE;
            int y2 = y1 + CELL_SIZE;

            if (cells[row][col]) {
                al_draw_filled_rectangle(x1 + 1, y1 + 1, x2 - 1, y2 - 1, al_map_rgb(0, 200, 0));
            }
            al_draw_rectangle(x1, y1, x2, y2, al_map_rgb(70, 70, 70), 1);
        }
    }
}

int main() {
    if (!al_init()) return -1;
    if (!al_init_primitives_addon()) return -1;
    if (!al_install_mouse()) return -1;
    
    // 1. INICIALIZAR EL TECLADO
    if (!al_install_keyboard()) return -1;

    ALLEGRO_DISPLAY* display = al_create_display(SCREEN_W, SCREEN_H);
    if (!display) return -1;

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 10.0);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    if (!timer || !queue) {
        al_destroy_display(display);
        return -1;
    }

    // 2. REGISTRAR FUENTES DE EVENTOS (DISPLAY, TIMER, MOUSE Y TECLADO)
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_keyboard_event_source()); // Eventos de teclado

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            cells[row][col] = (std::rand() % 5 == 0);
        }
    }

    al_start_timer(timer);
    bool running = true;
    bool redraw = true;
    
    // VARIABLE DE CONTROL: Inicia en falso para que la simulación corra de inmediato
    bool pausa = false; 

    while (running) {
        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        } 
        // 3. DETECTAR PRESIÓN DE TECLAS
        else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (event.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                pausa = !pausa; // Invierte el estado: si es true pasa a false y viceversa
            }
            else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                running = false; // Permite cerrar el juego con la tecla ESC
            }
        }
        // 4. CONTROLAR LA ACTUALIZACIÓN CON LA PAUSA
        else if (event.type == ALLEGRO_EVENT_TIMER) {
            if (!pausa) { 
                updateGame(); // Solo avanza la lógica si NO está pausado
            }
            redraw = true; // Se sigue redibujando para poder ver lo que pintas con el mouse
        } 
        // 5. CAMBIAR CÉLULAS CON EL MOUSE (Funciona tanto en pausa como corriendo)
        else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || event.type == ALLEGRO_EVENT_MOUSE_AXES) {
            ALLEGRO_MOUSE_STATE mouse_state;
            al_get_mouse_state(&mouse_state);

            if (mouse_state.buttons & 1) { // Clic izquierdo
                int col = mouse_state.x / CELL_SIZE;
                int row = mouse_state.y / CELL_SIZE;

                if (col >= 0 && col < COLS && row >= 0 && row < ROWS) {
                    cells[row][col] = true; 
                    redraw = true;          
                }
            }
        }

        if (redraw && al_is_event_queue_empty(queue)) {
            drawGrid();
            al_flip_display();
            redraw = false;
        }
    }

    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);
    return 0;
}
