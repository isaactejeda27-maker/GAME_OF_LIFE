#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <cstdlib>
#include <ctime>
#include <cstdio>

// Configuración de dimensiones
const int SCREEN_W = 900;
const int GAME_H = 600;      
const int HUD_H = 600;       
const int SCREEN_H = 660; // 60 píxeles extra para la barra de datos inferior
const int CELL_SIZE = 7;
const int COLS = SCREEN_W / CELL_SIZE;
const int ROWS = GAME_H / CELL_SIZE;

// Máquina de estados
enum EstadoSistema { MENU, JUEGO };
EstadoSistema estado_actual = MENU;

// Matrices del Juego de la Vida
bool cells[ROWS][COLS];
bool next_cells[ROWS][COLS];
int generaciones = 0;

// Cuenta los vecinos alrededor de una celda específica
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

// Cuenta la población total en tiempo real
int contarCelulasVivas() {
    int vivas = 0;
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (cells[row][col]) vivas++;
        }
    }
    return vivas;
}

// Lógica de actualización (Reglas de Conway)
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

// Modos de inicio de la cuadrícula
void inicializarTablero(bool aleatorio) {
    generaciones = 0;
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (aleatorio) {
                cells[row][col] = (std::rand() % 5 == 0); // 20% de probabilidad de vida
            } else {
                cells[row][col] = false; // Lienzo limpio
            }
        }
    }
}

// Dibujado del menú principal
void drawMenu(ALLEGRO_FONT* font) {
    al_clear_to_color(al_map_rgb(20, 20, 25));
    
    al_draw_text(font, al_map_rgb(0, 220, 0), SCREEN_W / 2, 150, ALLEGRO_ALIGN_CENTER, "CONWAY'S GAME OF LIFE");
    
    al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_W / 2, 280, ALLEGRO_ALIGN_CENTER, "Presiona [ V ] para Mapa VACIO");
    al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_W / 2, 340, ALLEGRO_ALIGN_CENTER, "Presiona [ A ] para Mapa ALEATORIO");
    
    al_draw_text(font, al_map_rgb(120, 120, 120), SCREEN_W / 2, 500, ALLEGRO_ALIGN_CENTER, "Presiona ESC para salir");
}

// Dibujado de la cuadrícula, simulación e interfaz inferior
void drawGrid(ALLEGRO_FONT* font, bool pausa) {
    al_clear_to_color(al_map_rgb(0, 0, 0));

    // Dibujar células y líneas divisorias
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            int x1 = col * CELL_SIZE;
            int y1 = row * CELL_SIZE;
            if (cells[row][col]) {
                al_draw_filled_rectangle(x1 + 1, y1 + 1, x1 + CELL_SIZE - 1, y1 + CELL_SIZE - 1, al_map_rgb(0, 200, 0));
            }
            al_draw_rectangle(x1, y1, x1 + CELL_SIZE, y1 + CELL_SIZE, al_map_rgb(45, 45, 45), 1);
        }
    }

    // Dibujar fondo del panel inferior (HUD)
    al_draw_filled_rectangle(0, HUD_H, SCREEN_W, SCREEN_H, al_map_rgb(25, 25, 25));
    al_draw_line(0, HUD_H, SCREEN_W, HUD_H, al_map_rgb(100, 100, 100), 2);

    // Formatear texto de estadísticas de manera segura
    char texto_gen[50];
    char texto_vivas[50];
    std::snprintf(texto_gen, sizeof(texto_gen), "Generacion: %d", generaciones);
    std::snprintf(texto_vivas, sizeof(texto_vivas), "Celulas Vivas: %d", contarCelulasVivas());

    // Imprimir datos en pantalla
    al_draw_text(font, al_map_rgb(255, 255, 255), 20, HUD_H + 20, ALLEGRO_ALIGN_LEFT, texto_gen);
    al_draw_text(font, al_map_rgb(255, 255, 255), 220, HUD_H + 20, ALLEGRO_ALIGN_LEFT, texto_vivas);

    // Indicador dinámico de estado
    if (pausa) {
        al_draw_text(font, al_map_rgb(230, 50, 50), 410, HUD_H + 20, ALLEGRO_ALIGN_LEFT, "[ PAUSA - Espacio/Enter ]");
    } else {
        al_draw_text(font, al_map_rgb(50, 200, 50), 410, HUD_H + 20, ALLEGRO_ALIGN_LEFT, "[ CORRIENDO ]");
    }

    // Botón físico interactivo para Salir
    al_draw_filled_rectangle(660, HUD_H + 15, 780, HUD_H + 45, al_map_rgb(120, 20, 20));
    al_draw_rectangle(660, HUD_H + 15, 780, HUD_H + 45, al_map_rgb(230, 50, 50), 2);
    al_draw_text(font, al_map_rgb(255, 255, 255), 720, HUD_H + 23, ALLEGRO_ALIGN_CENTER, "Salir (ESC)");
}

int main() {
    // Inicialización del motor y periféricos
    if (!al_init()) return -1;
    if (!al_init_primitives_addon()) return -1;
    if (!al_install_mouse()) return -1;
    if (!al_install_keyboard()) return -1;
    
    al_init_font_addon();
    if (!al_init_ttf_addon()) return -1;

    ALLEGRO_DISPLAY* display = al_create_display(SCREEN_W, SCREEN_H);
    if (!display) return -1;

    ALLEGRO_FONT* font = al_create_builtin_font();
    if (!font) {
        al_destroy_display(display);
        return -1;
    }

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 10.0); // 10 Actualizaciones por segundo
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    if (!timer || !queue) {
        if (timer) al_destroy_timer(timer);
        al_destroy_font(font);
        al_destroy_display(display);
        return -1;
    }

    // Vinculación de periféricos a la cola de atención
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_keyboard_event_source());

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    al_start_timer(timer);
    bool running = true;
    bool redraw = true;
    bool pausa = false;

    // Bucle principal de ejecución (Event Loop)
    while (running) {
        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        } 
        
        // Entrada de comandos por teclado
        else if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                running = false;
            }
            
            if (estado_actual == MENU) {
                char tecla = static_cast<char>(event.keyboard.unichar);
                if (tecla == 'v' || tecla == 'V') {
                    inicializarTablero(false);
                    pausa = true; // Inicia pausado para permitir dibujar figuras con calma            
                    estado_actual = JUEGO;
                    redraw = true;
                }
                else if (tecla == 'a' || tecla == 'A') {
                    inicializarTablero(true);
                    pausa = false; // Inicia de inmediato a calcular las generaciones            
                    estado_actual = JUEGO;
                    redraw = true;
                }
            } 
            else if (estado_actual == JUEGO) {
                if (event.keyboard.keycode == ALLEGRO_KEY_SPACE || event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                    pausa = !pausa;
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_C) {
                    estado_actual = MENU; // Detiene la partida actual y regresa a la pantalla de opciones
                    redraw = true;
                }
            }
        }
        
        // Ticks de reloj independientes de la velocidad de refresco del monitor
        else if (event.type == ALLEGRO_EVENT_TIMER) {
            if (estado_actual == JUEGO && !pausa) {
                updateGame();
            }
            redraw = true;
        } 
        
        // Entrada por mouse (Detección de pulsación inicial y botón de salida)
        else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            if (estado_actual == JUEGO) {
                ALLEGRO_MOUSE_STATE mouse_state;
                al_get_mouse_state(&mouse_state);

                if (mouse_state.buttons & 1) { 
                    int mx = mouse_state.x;
                    int my = mouse_state.y;

                    // Evaluar colisión con la hitbox del botón Salir
                    if (mx >= 660 && mx <= 780 && my >= (HUD_H + 15) && my <= (HUD_H + 45)) {
                        running = false; 
                    }
                    else {
                        int col = mx / CELL_SIZE;
                        int row = my / CELL_SIZE;

                        if (col >= 0 && col < COLS && row >= 0 && row < ROWS) {
                            cells[row][col] = true;
                            redraw = true;
                        }
                    }
                }
            }
        }
        
        // Entrada por mouse (Pintado por arrastre continuo)
        else if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
            if (estado_actual == JUEGO) {
                ALLEGRO_MOUSE_STATE mouse_state;
                al_get_mouse_state(&mouse_state);

                if (mouse_state.buttons & 1) { 
                    int col = mouse_state.x / CELL_SIZE;
                    int row = mouse_state.y / CELL_SIZE;

                    if (col >= 0 && col < COLS && row >= 0 && row < ROWS) {
                        cells[row][col] = true;
                        redraw = true;
                    }
                }
            }
        }       

        // Gestión eficiente del renderizado gráfico
        if (redraw && al_is_event_queue_empty(queue)) {
            if (estado_actual == MENU) {
                drawMenu(font);
            } else if (estado_actual == JUEGO) {
                drawGrid(font, pausa);
            }
            al_flip_display();
            redraw = false;
        }
    }

        // Liberación segura de recursos asignados
        al_destroy_font(font);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return 0;
    }

