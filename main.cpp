#include <allegro5/allegro.h> //funciones de la libreria allegro
#include <allegro5/allegro_primitives.h> //libreria para dibijar figuras geometricas
#include <allegro5/allegro_font.h> //libreria para textos y tipografias
#include <iostream>
#include <cstdlib> //funciones internas para codigo
#include <ctime> //funciones de relog
#include "game.h" //inclucion de las funciones declradas 

using namespace std; //nombres de espacios

//esta funcion se llama al hacer click o al arrastrar, pinta la celda correspondiente
void pintar_celda(int x, int y) {
    
    // convertir coordenadas de pantalla a indices del tablero
    int c = x / tablero.tamCel;
    int f = y / tablero.tamCel;

    //checamos que no nos pasemos de los limites
    if (f >= 0 && f < tablero.filas && c >= 0 && c < tablero.cols) {
        
        tablero.celulas[pos(f, c)] = !tablero.celulas[pos(f, c)];
    }
}

int main() {
    //montamos todo el entorno de allegro, si algo falla cerramos
    if (!al_init() || !al_init_primitives_addon() || !al_install_mouse() || !al_install_keyboard()){ 
        
        std::cerr << "Error al iniciar allegro" << endl;
        return -1;
    }
   

    al_init_font_addon();

    //creamos ventana, fuente, reloj y cola de eventos
    ALLEGRO_DISPLAY* ventana = al_create_display(ANCHO, ALTO);
    ALLEGRO_FONT* fuente = al_create_builtin_font();
    ALLEGRO_TIMER* reloj = al_create_timer(1.0 / 10.0);
    ALLEGRO_EVENT_QUEUE* cola = al_create_event_queue();

    //si algo no se pudo crear, salimos con error
    if (!ventana || !fuente || !reloj || !cola || !inicializarTablero(tablero.tamCel)) {
       
        std::cerr << "No se pudo ejecutar una funcion en la cola de eventos" << endl;
        return -1;
    }

    //registramos las fuentes de eventos para cada cosa
    al_register_event_source(cola, al_get_display_event_source(ventana));
    al_register_event_source(cola, al_get_timer_event_source(reloj));
    al_register_event_source(cola, al_get_mouse_event_source());
    al_register_event_source(cola, al_get_keyboard_event_source());

    srand((unsigned int) time(NULL));

    //cargamos la lista de mapas que haya guardados
    cargarListaGuardados();

    bool activo = true;
    bool redibujar= true;
    bool pausa = false;

    al_start_timer(reloj);

    //loop principal, se queda hasta que cierren
    while (activo) {
       
        ALLEGRO_EVENT ev;
        al_wait_for_event(cola, &ev);

        //cerrar con la X de la ventana
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
           
            activo = false;
        }
        //teclas
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            
            int tecla = ev.keyboard.keycode;

            //escape para salir en cualquier momento
            if (tecla == ALLEGRO_KEY_ESCAPE) {
               
                activo = false;
            }
            //controles del menu principal
            else if (estado == MENU) {
               
                if (tecla == ALLEGRO_KEY_V) {
                   
                    prepararTablero(false);
                    pausa = true;
                    estado = JUEGO;
                }
                else if (tecla == ALLEGRO_KEY_A) {
                    
                    prepararTablero(true);
                    pausa = false;
                    estado = JUEGO;
                }
                else if (tecla == ALLEGRO_KEY_L) {
                   
                    cargarListaGuardados();
                    estado = CARGAR;
                }
                //agrandar el tamano de las celdas
                else if ((tecla == ALLEGRO_KEY_UP ||
                          
                    tecla == ALLEGRO_KEY_EQUALS ||
                          tecla == ALLEGRO_KEY_PAD_PLUS) &&
                         tablero.tamCel < CEL_MAX) {
                    inicializarTablero(tablero.tamCel + 1);
                }
                //achicar el tamano de las celdas
                else if ((tecla == ALLEGRO_KEY_DOWN || tecla == ALLEGRO_KEY_MINUS || tecla == ALLEGRO_KEY_PAD_MINUS) && tablero.tamCel > CEL_MIN) {
                    
                    inicializarTablero(tablero.tamCel - 1);
                }
            
                redibujar = true;
            }
           
            //controles durante el juego
            else if (estado == JUEGO) {
               
                if (tecla == ALLEGRO_KEY_SPACE || tecla == ALLEGRO_KEY_ENTER) {
               
                    pausa = !pausa;
                }
                else if (tecla == ALLEGRO_KEY_S && pausa) {
               
                    guardarMapaActual();
                }
                else if (tecla == ALLEGRO_KEY_D) {
               
                    vaciarTablero();
                }
                else if (tecla == ALLEGRO_KEY_C) {
               
                    estado = MENU;
                }
                redibujar = true;
            }
            //controles del menu de carga
            else if (estado == CARGAR) {
              
                if (tecla == ALLEGRO_KEY_B) {
              
                    estado = MENU;
                }
                else if (tecla >= ALLEGRO_KEY_1 && tecla <= ALLEGRO_KEY_9) {
              
                    int i = tecla - ALLEGRO_KEY_1;
                    if (i < cantGuardados && cargarMapa(listaGuardados[i])) {
              
                        pausa = true;
                        estado = JUEGO;
                    }
                }
                redibujar = true;
            }
        }
        //ticks del reloj, si no esta pausado el juego avanza
        else if (ev.type == ALLEGRO_EVENT_TIMER) {
            
            if (estado == JUEGO && !pausa) {
            
                actualizar();
            }
            redibujar = true;
        }
        //click del mouse
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        
            if (estado == JUEGO && ev.mouse.button == 1) {
        
                int x = ev.mouse.x;
                int y = ev.mouse.y;

                //estos son los botones de la barra inferior
                if (x >= 520 && x <= 640 && y >= BARRA_Y + 15 && y <= BARRA_Y + 45) {
        
                    estado = MENU;
                }
                else if (x >= 660 && x <= 780 && y >= BARRA_Y + 15 && y <= BARRA_Y + 45) {
        
                    activo = false;
                }
                else {
        
                    pintar_celda(x, y);
                }
                redibujar = true;
            }
        }
        //para poder pintar celdas arrastrando el mouse
        else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
        
            if (estado == JUEGO) {
        
                ALLEGRO_MOUSE_STATE mouse;
                al_get_mouse_state(&mouse);
                if (mouse.buttons & 1) {
        
                    pintar_celda(mouse.x, mouse.y);
                    redibujar = true;
                }
            }
        }

        //solo redibuja si hace falta y no hay eventos pendientes
        if (redibujar && al_is_event_queue_empty(cola)) {
        
            if (estado == MENU) {
        
                dibujarMenu(fuente);
            }
            else if (estado == CARGAR) {
        
                dibujarCarga(fuente);
            }
            else if (estado == JUEGO) {
        
                dibujarTablero(fuente, pausa);
            }
            al_flip_display();
            redibujar = false;
        }
    }

    //liberamos la memoria del tablero y de allegro
    liberarTablero();
    al_destroy_font(fuente);
    al_destroy_timer(reloj);
    al_destroy_event_queue(cola);
    al_destroy_display(ventana);

    return 0;
}