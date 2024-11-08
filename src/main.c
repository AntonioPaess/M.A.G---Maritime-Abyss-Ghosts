#include <stdio.h>
#include <unistd.h> // Para usleep
#include <time.h>   // Para medir o tempo
#include "keyboard.h" // Certifique-se de que keyhit() é não bloqueante
#include "screen.h"   // Inclui a biblioteca screen

#define MAP_WIDTH  20
#define MAP_HEIGHT 10
#define FRAME_TIME 16667 // Tempo de frame em microsegundos para ~60 FPS

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
    int ativo; // Indica se o inimigo está ativo
} Inimigo;

Objeto obj = {10, 5}; // Posição inicial do jogador
Inimigo inimigo = {0, 0, 100, 0}; // Inimigo inativo inicialmente
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

void moverInimigo(Inimigo *inimigo, Objeto *obj, int frameCount) {
    // Alterna o eixo de movimento a cada frame
    if (frameCount % 2 == 0) {
        // Move no eixo X
        if (inimigo->x < obj->x) {
            inimigo->x += 1;
        } else if (inimigo->x > obj->x) {
            inimigo->x -= 1;
        }
    } else {
        // Move no eixo Y
        if (inimigo->y < obj->y) {
            inimigo->y += 1;
        } else if (inimigo->y > obj->y) {
            inimigo->y -= 1;
        }
    }
}

void moverMachado() {
    if (machado.ativo) {
        machado.x += 1; // Move o machado na direção +1 do eixo x
        machado.distancia -= 1;
        if (machado.distancia <= 0) {
            machado.ativo = 0; // Desativa o machado quando a distância é zero
        }
    }
}

int atacar(Inimigo *inimigo, Machado *machado) {
    // Verifica se o machado atingiu o inimigo
    if (machado->ativo && inimigo->ativo && machado->x == inimigo->x && machado->y == inimigo->y) {
        machado->ativo = 0; // Desativa o machado
        inimigo->vida -= 100; // Machado causa 100 de dano
        if (inimigo->vida <= 0) {
            inimigo->ativo = 0; // Inimigo morto
            return 1;
        }
    }
    return 0; // Inimigo não morto
}

void atualizarTela(Objeto *obj, Inimigo *inimigo, Machado *machado, int tempoRestante) {
    screenClear(); // Limpa a tela

    // Desenha o jogador
    screenGotoxy(obj->x, obj->y);
    printf("O");

    // Desenha o inimigo se ele estiver ativo
    if (inimigo->ativo && inimigo->vida > 0) {
        screenGotoxy(inimigo->x, inimigo->y);
        printf("X");
    }

    // Desenha o machado se estiver ativo
    if (machado->ativo) {
        screenGotoxy(machado->x, machado->y);
        printf("M");
    }

    // Exibe aviso antes do inimigo aparecer
    if (!inimigo->ativo && tempoRestante > 0) {
        screenGotoxy(1, MAP_HEIGHT + 2);
        printf("O inimigo aparecerá em %d segundos!", tempoRestante);
    }

    fflush(stdout); // Atualiza a tela
}

void iniciarMovimentoMachado() {
    machado.ativo = 5;
    machado.direcao = 'm';
    machado.distancia = MAP_WIDTH - machado.x; // Distância máxima do machado
}

int main() {
    char input;
    int frameCount = 0;
    time_t tempoInicial = time(NULL);
    keyboardInit(); // Inicializa o teclado
    screenInit(1);   // Inicializa a tela
    printf("Use WASD para mover o objeto. Pressione 'q' para sair. Pressione 'm' para lançar o machado.\n");

    while (1) {
        // Calcula o tempo decorrido
        time_t tempoAtual = time(NULL);
        int tempoDecorrido = (int)difftime(tempoAtual, tempoInicial);
        int tempoRestante = 5 - tempoDecorrido;
        if (tempoRestante < 0) tempoRestante = 0;

        // Verifica entrada do usuário sem bloquear
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

        // Atualiza lógica do jogo
        if (machado.ativo) {
            moverMachado();
        }

        // Incrementa o contador de frames
        frameCount++;

        // Controla o aparecimento do inimigo após 5 segundos
        if (tempoDecorrido >= 5) {
            if (!inimigo.ativo) {
                inimigo.ativo = 1; // Ativa o inimigo
            }
            // Move o inimigo a cada 5 frames para diminuir a velocidade
            if (inimigo.ativo && inimigo.vida > 0 && frameCount % 5 == 0) {
                moverInimigo(&inimigo, &obj, frameCount);
            }
            if (atacar(&inimigo, &machado)) {
                // Inimigo foi morto
            }

            // Verifica colisão entre o inimigo e o jogador
            if (inimigo.ativo && inimigo.x == obj.x && inimigo.y == obj.y) {
                screenClear();
                printf("Game Over! Você foi pego pelo inimigo.\n");
                printf("Pressione enter para sair.\n");
                keyboardDestroy(); // Restaura o terminal
                getchar(); // Aguarda o jogador pressionar uma tecla
                break; // Sai do loop principal
            }
        }

        // Atualiza a tela
        atualizarTela(&obj, &inimigo, &machado, tempoRestante);

        // Controla a taxa de quadros
        usleep(FRAME_TIME); // Pausa para manter a taxa de quadros
    }

    keyboardDestroy(); // Destrói o teclado
    screenDestroy();   // Destrói a tela
    return 0;
}