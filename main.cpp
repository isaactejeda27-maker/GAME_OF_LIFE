#include <allegro5/allegro.h> 
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <cstdlib> 
#include <ctime>
#include "game.h" //funciones definidas

int main() {
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

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 10.0);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    if (!timer || !queue) {
        if (timer) al_destroy_timer(timer);
        al_destroy_font(font);
        al_destroy_display(display);
        return -1;
    }

    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_keyboard_event_source());

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    cargarListaGuardados();

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
        else if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                running = false;
            }

            if (estado_actual == MENU) {
                char tecla = static_cast<char>(event.keyboard.unichar);
                if (tecla == 'v' || tecla == 'V') {
                    inicializarTablero(false);
                    pausa = true;
                    estado_actual = JUEGO;
                    redraw = true;
                }
                else if (tecla == 'a' || tecla == 'A') {
                    inicializarTablero(true);
                    pausa = false;
                    estado_actual = JUEGO;
                    redraw = true;
                }
                else if (tecla == 'l' || tecla == 'L') {
                    cargarListaGuardados();
                    estado_actual = CARGAR;
                    redraw = true;
                }
            }
            else if (estado_actual == JUEGO) {
                if (event.keyboard.keycode == ALLEGRO_KEY_SPACE || event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                    pausa = !pausa;
                }
                else if ((event.keyboard.keycode == ALLEGRO_KEY_S) && pausa) {
                    guardarPatronActual();
                    redraw = true;
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_D) {
                    limpiarTablero();
                    redraw = true;
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_C) {
                    estado_actual = MENU;
                    redraw = true;
                }
            }
            else if (estado_actual == CARGAR) {
                char tecla = static_cast<char>(event.keyboard.unichar);
                if (tecla == 'b' || tecla == 'B') {
                    estado_actual = MENU;
                    redraw = true;
                } else if (tecla >= '1' && tecla <= '9') {
                    int index = tecla - '1';
                    if (index < saveFileCount) {
                        cargarPatron(saveFiles[index]);
                        pausa = true;
                        estado_actual = JUEGO;
                        redraw = true;
                    }
                }
            }
        }
        else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (estado_actual == MENU) {
                if ((event.keyboard.keycode == ALLEGRO_KEY_PAD_PLUS || event.keyboard.keycode == ALLEGRO_KEY_EQUALS)
                    && cellSize < MAX_CELL_SIZE) {
                    cellSize++;
                    updateGridDimensions();
                    redraw = true;
                } else if ((event.keyboard.keycode == ALLEGRO_KEY_MINUS || event.keyboard.keycode == ALLEGRO_KEY_PAD_MINUS)
                    && cellSize > MIN_CELL_SIZE) {
                    cellSize--;
                    updateGridDimensions();
                    redraw = true;
                } else if (event.keyboard.keycode == ALLEGRO_KEY_UP && cellSize < MAX_CELL_SIZE) {
                    cellSize++;
                    updateGridDimensions();
                    redraw = true;
                } else if (event.keyboard.keycode == ALLEGRO_KEY_DOWN && cellSize > MIN_CELL_SIZE) {
                    cellSize--;
                    updateGridDimensions();
                    redraw = true;
                }
            }
        }
        else if (event.type == ALLEGRO_EVENT_TIMER) {
            if (estado_actual == JUEGO && !pausa) {
                updateGame();
            }
            redraw = true;
        }
        else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            if (estado_actual == JUEGO) {
                ALLEGRO_MOUSE_STATE mouse_state;
                al_get_mouse_state(&mouse_state);

                if (mouse_state.buttons & 1) {
                    int mx = mouse_state.x;
                    int my = mouse_state.y;

                    if (mx >= 520 && mx <= 640 && my >= (HUD_H + 15) && my <= (HUD_H + 45)) {
                        estado_actual = MENU;
                        redraw = true;
                    }
                    else if (mx >= 660 && mx <= 780 && my >= (HUD_H + 15) && my <= (HUD_H + 45)) {
                        running = false;
                    }
                    else {
                        int col = mx / cellSize;
                        int row = my / cellSize;
                        if (col >= 0 && col < COLS && row >= 0 && row < ROWS) {
                            cells[row][col] = true;
                            redraw = true;
                        }
                    }
                }
            }
        }
        else if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
            if (estado_actual == JUEGO) {
                ALLEGRO_MOUSE_STATE mouse_state;
                al_get_mouse_state(&mouse_state);
                if (mouse_state.buttons & 1) {
                    int col = mouse_state.x / cellSize;
                    int row = mouse_state.y / cellSize;
                    if (col >= 0 && col < COLS && row >= 0 && row < ROWS) {
                        cells[row][col] = true;
                        redraw = true;
                    }
                }
            }
        }

        if (redraw && al_is_event_queue_empty(queue)) {
            if (estado_actual == MENU) {
                drawMenu(font);
            } else if (estado_actual == CARGAR) {
                drawLoadMenu(font);
            } else if (estado_actual == JUEGO) {
                drawGrid(font, pausa);
            }
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

