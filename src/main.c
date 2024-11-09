#include <stdio.h>
#include <unistd.h> // Para usleep
#include <time.h>
#include <stdlib.h> // Para malloc, rand, srand

#include "keyboard.h"
#include "screen.h"
#include "listaEncadeada.h"

#define FRAME_TIME 16667 // 16.67ms para 60 FPS

#define MAP_WIDTH (MAXX - MINX + 1)
#define MAP_HEIGHT (MAXY - MINY + 1)

typedef struct {
    int x;
    int y;
} Objeto;

typedef struct {
    int x;
    int y;
    int vida;
    int ativo; // Indica se o inimigo está ativo
} Inimigo;

typedef struct {
    int x;
    int y;
    int ativo;
    char direcao;
    int distancia;
    int moveCounter; // Contador para controlar a frequência de movimento
} Machado;

Objeto obj = {10, 5}; // Posição inicial do jogador
Inimigo inimigo = {0, 0, 100, 0}; // Inimigo inativo inicialmente
Machado machado = {0, 0, 0, ' ', 0}; // Inicializa o machado como inativo
Node* spawnPositions = NULL;
char lastDir = 'd'; // Direção padrão inicial (direita)

void initSpawnPositions() {
    spawnPositions = criarLista();
    for (int x = MINX; x <= MAXX; x++) {
        for (int y = MINY; y <= MAXY; y++) {
            if ((x == obj.x && y == obj.y) || (x == machado.x && y == machado.y)) {
                continue; // Evita spawnar na posição do jogador ou do machado
            }
            int* pos = (int*)malloc(2 * sizeof(int));
            pos[0] = x;
            pos[1] = y;
            inserirFim(&spawnPositions, pos);
        }
    }
}

void freeSpawnPositions() {
    liberarLista(&spawnPositions);
}

void getRandomSpawnPosition(int* x, int* y) {
    int count = 0;
    Node* temp = spawnPositions;
    while (temp != NULL) {
        count++;
        temp = temp->next;
    }
    if (count == 0) return; // Evita divisão por zero
    int randomIndex = rand() % count;
    temp = spawnPositions;
    for (int i = 0; i < randomIndex; i++) {
        temp = temp->next;
    }
    int* pos = (int*)temp->data;
    *x = pos[0];
    *y = pos[1];
}

void moverMachado() {
    if (machado.ativo) {
        machado.moveCounter++;
        if (machado.moveCounter % 3 == 0) { // Mover a cada 3 frames (mais lento)
            switch (machado.direcao) {
                case 'w': machado.y--; break; // Move para cima
                case 's': machado.y++; break; // Move para baixo
                case 'a': machado.x--; break; // Move para a esquerda
                case 'd': machado.x++; break; // Move para a direita
            }
            machado.distancia--;
            if (machado.distancia <= 0 || machado.x < MINX || machado.x > MAXX || machado.y < MINY || machado.y > MAXY) {
                machado.ativo = 0; // Desativa o machado
            }
        }
    }
}

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
    lastDir = direcao; // Atualiza a última direção
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
    // Desenha o inimigo se estiver ativo
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
    char oppositeDir;
    // Determina a direção oposta
    switch(lastDir) {
        case 'w': oppositeDir = 's'; break;
        case 's': oppositeDir = 'w'; break;
        case 'a': oppositeDir = 'd'; break;
        case 'd': oppositeDir = 'a'; break;
        default: oppositeDir = 'd'; // Direção padrão
    }
    machado.ativo = 1;
    machado.direcao = oppositeDir;
    machado.distancia = 5; // Aumenta a distância de 2 para 5 blocos
    machado.moveCounter = 0; // Inicializa o contador
}

int main() {
    srand(time(NULL)); // Inicializa o gerador de números aleatórios
    initSpawnPositions(); // Inicializa as posições de spawn
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
                    machado.x = obj.x;
                    machado.y = obj.y;
                    iniciarMovimentoMachado(); // Inicia o movimento do machado
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
                getRandomSpawnPosition(&inimigo.x, &inimigo.y); // Define posição aleatória para o inimigo
                inimigo.ativo = 1; // Ativa o inimigo
            }
            // Move o inimigo a cada 4 frames para torná-lo mais lento
            if (inimigo.ativo && inimigo.vida > 0 && frameCount % 4 == 0) {
                moverInimigo(&inimigo, &obj);
            }
            if (atacar(&inimigo, &machado)) {
                // Inimigo foi morto
            }
            // Verifica colisão entre o inimigo e o jogador
            if (inimigo.ativo && inimigo.x == obj.x && inimigo.y == obj.y) {
                screenClear();
                printf("Game Over! Você foi pego pelo inimigo.\n");
                printf("Pressione Enter para sair.\n");
                keyboardDestroy(); // Restaura o terminal
                screenDestroy();   // Restaura a tela
                freeSpawnPositions(); // Libera a lista de posições de spawn
                return 0;
            }
        }
        // Atualiza a tela
        atualizarTela(&obj, &inimigo, &machado, tempoRestante);
        // Controla a taxa de quadros
        usleep(FRAME_TIME); // Pausa para manter a taxa de quadros
    }
    keyboardDestroy(); // Destrói o teclado
    screenDestroy();   // Destrói a tela
    freeSpawnPositions(); // Libera a lista de posições de spawn
    return 0;
}