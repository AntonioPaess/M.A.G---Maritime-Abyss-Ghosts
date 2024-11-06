#include <stdio.h>
#include <unistd.h> // Para usleep
#include "keyboard.h" // Inclui a biblioteca keyboard
#include "screen.h"   // Inclui a biblioteca screen
#include "timer.h"    // Inclui a biblioteca timer

#define MAP_WIDTH  20
#define MAP_HEIGHT 10
#define FRAME_TIME 16000 // Tempo de frame em microsegundos para ~60 FPS

typedef struct {
    int x;
    int y;
} Objeto;

typedef struct {
    int x;
    int y;
    int ativo;
    char direcao;
    int distancia;
} Machado;

typedef struct {
    int x;
    int y;
    int vida;
} Inimigo;

Objeto obj = {SCRSTARTX, SCRSTARTY};
Inimigo inimigo = {SCRENDX, SCRENDY, 100}; // Inicializa o inimigo com 100 de vida
Machado machado = {0, 0, 0, ' ', 0}; // Inicializa o machado como inativo

void moverObjeto(Objeto *obj, char direcao) {
    switch (direcao) {
        case 'w':
            obj->y -= 1;
            break;
        case 'a':
            obj->x -= 1;
            break;
        case 's':
            obj->y += 1;
            break;
        case 'd':
            obj->x += 1;
            break;
    }
}

void moverInimigo(Inimigo *inimigo, Objeto *obj) {
    if (inimigo->x < obj->x) {
        inimigo->x += 1;
    } else if (inimigo->x > obj->x) {
        inimigo->x -= 1;
    }

    if (inimigo->y < obj->y) {
        inimigo->y += 1;
    } else if (inimigo->y > obj->y) {
        inimigo->y -= 1;
    }
}

void moverMachado() {
    if (machado.ativo) {
        machado.x += 1; // Move o machado na direção +1 do eixo x
        machado.distancia -= 1;
        if (machado.distancia <= 0) {
            machado.ativo = 0; // Desativa o machado quando a distância é zero
            timerDestroy(); // Para o temporizador
        }
    }
}

int atacar(Inimigo *inimigo, Machado *machado) {
    // Verifica se o machado atingiu o inimigo
    if (machado->ativo && machado->x == inimigo->x && machado->y == inimigo->y) {
        machado->ativo = 0; // Desativa o machado
        inimigo->vida -= 100; // Machado causa 100 de dano
        if (inimigo->vida <= 0) {
            return 1; // Inimigo morto
        }
    }
    return 0; // Inimigo não morto
}

void atualizarTela(Objeto *obj, Inimigo *inimigo, Machado *machado) {
    screenClear(); // Limpa a tela
    screenGotoxy(obj->x, obj->y); // Move o cursor para a nova posição do objeto
    printf("O"); // Desenha o objeto na nova posição
    if (inimigo->vida > 0) {
        screenGotoxy(inimigo->x, inimigo->y); // Move o cursor para a nova posição do inimigo
        printf("X"); // Desenha o inimigo na nova posição
    }
    if (machado->ativo) {
        screenGotoxy(machado->x, machado->y); // Move o cursor para a nova posição do machado
        printf("M"); // Desenha o machado na nova posição
    }
    fflush(stdout); // Atualiza a tela
}

void iniciarMovimentoMachado() {
    machado.ativo = 1;
    machado.direcao = 'm';
    machado.distancia = MAP_WIDTH - machado.x; // Distância máxima do machado
    timerInit(500); // Inicia o temporizador para mover o machado por 0.5 segundos
}

int main() {
    char input;

    keyboardInit(); // Inicializa o teclado
    screenInit(1);   // Inicializa a tela

    printf("Use WASD para mover o objeto. Use Q, E, Z, C para mover na diagonal. Pressione 'q' para sair. Pressione 'M' para lançar o machado.\n");

    while (1) {
        if (keyhit()) {
            input = getchar();
            if (input == 'q') {
                break;
            }
            if (input == 'm') {
                if (!machado.ativo) {
                    machado.x = obj.x + 1; // Machado começa na posição +1 do eixo x
                    machado.y = obj.y;
                    iniciarMovimentoMachado();
                }
            } else {
                moverObjeto(&obj, input);
            }
        }

        if (machado.ativo && timerTimeOver()) {
            moverMachado();
            timerUpdateTimer(500); // Reinicia o temporizador para continuar movendo o machado
        }

        if (inimigo.vida > 0) {
            moverInimigo(&inimigo, &obj);
        }
        if (atacar(&inimigo, &machado)) {
            inimigo.vida = 0; // Inimigo foi morto
        }
        atualizarTela(&obj, &inimigo, &machado);
        usleep(FRAME_TIME); // Pausa para manter a taxa de quadros
    }

    keyboardDestroy(); // Destrói o teclado
    screenDestroy();   // Destrói a tela
    timerDestroy();    // Destroi o temporizador

    return 0;
}