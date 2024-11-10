#include <stdio.h>
#include <unistd.h> // Para usleep, getcwd
#include <time.h>
#include <stdlib.h> // malloc, rand, srand, abs, exit
#include <string.h> // Para manipulação de strings
#include <limits.h> // Para PATH_MAX
#include <mach-o/dyld.h>  // Para _NSGetExecutablePath
#include <libgen.h>       // Para dirname

#include "keyboard.h"
#include "screen.h"
#include "listaEncadeada.h"

#define FRAME_TIME 16667 // 16.67ms para 60 FPS

#define MAP_WIDTH (MAXX - MINX + 1)
#define MAP_HEIGHT (MAXY - MINY + 1)

// Sistema de pontuação
int pontuacao = 0;

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

// Função para obter o diretório do executável
void obterDiretorioExecutavel(char* diretorio, size_t tamanho) {
    char path[PATH_MAX];
    uint32_t size = sizeof(path);

    if (_NSGetExecutablePath(path, &size) != 0) {
        fprintf(stderr, "Buffer muito pequeno. Necessário tamanho: %u\n", size);
        exit(EXIT_FAILURE);
    }

    // Remove o nome do executável para obter apenas o diretório
    char *dir = dirname(path);
    strncpy(diretorio, dir, tamanho - 1);
    diretorio[tamanho - 1] = '\0'; // Garantir terminação nula
}

Objeto obj = {10, 5};                   // Posição inicial do jogador
Machado machado = {0, 0, 0, ' ', 0, 0}; // Inicializa o machado como inativo
Node* inimigos = NULL;                  // Lista de inimigos
Node* spawnPositions = NULL;
char lastDir = 'd';   // Direção padrão inicial (direita)
char nomeJogador[50]; // Armazena o nome do jogador

void initSpawnPositions() {
    spawnPositions = criarLista();
    for (int x = MINX; x <= MAXX; x++) {
        for (int y = MINY; y <= MAXY; y++) {
            // Evita spawnar na posição do jogador ou do machado
            if ((x == obj.x && y == obj.y) || (x == machado.x && y == machado.y)) {
                continue;
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
    // Calcula a diferença em x e y
    int deltaX = obj->x - inimigo->x;
    int deltaY = obj->y - inimigo->y;

    if (abs(deltaX) > abs(deltaY)) {
        // Move no eixo x
        if (deltaX > 0) {
            inimigo->x += 1;
        } else if (deltaX < 0) {
            inimigo->x -= 1;
        }
    } else {
        // Move no eixo y
        if (deltaY > 0) {
            inimigo->y += 1;
        } else if (deltaY < 0) {
            inimigo->y -= 1;
        }
    }
}

void atualizarTela(Objeto *obj, Machado *machado, double tempoDecorrido) {
    screenClear(); // Limpa a tela
    // Desenha o jogador
    screenGotoxy(obj->x, obj->y);
    printf("O");
    // Desenha os inimigos
    Node* temp = inimigos;
    while (temp != NULL) {
        Inimigo* inimigo = (Inimigo*)temp->data;
        if (inimigo->ativo && inimigo->vida > 0) {
            screenGotoxy(inimigo->x, inimigo->y);
            printf("X");
        }
        temp = temp->next;
    }
    // Desenha o machado se estiver ativo
    if (machado->ativo) {
        screenGotoxy(machado->x, machado->y);
        printf("M");
    }
    // Exibe a pontuação
    screenGotoxy(1, MAP_HEIGHT + 2);
    printf("Pontuação: %d", pontuacao);
    // Exibe o cronômetro
    screenGotoxy(1, MAP_HEIGHT + 3);
    printf("Tempo: %.1f segundos", tempoDecorrido);
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
    machado.distancia = 8; // Aumenta a distância para 8 blocos
    machado.moveCounter = 0; // Inicializa o contador
}

Inimigo* criarInimigo() {
    Inimigo* novoInimigo = (Inimigo*)malloc(sizeof(Inimigo));
    getRandomSpawnPosition(&novoInimigo->x, &novoInimigo->y); // Define posição aleatória
    novoInimigo->vida = 100;
    novoInimigo->ativo = 1;
    return novoInimigo;
}

void adicionarInimigo(Node** lista, Inimigo* inimigo) {
    inserirFim(lista, inimigo);
}

int contarInimigos(Node* lista) {
    int count = 0;
    Node* temp = lista;
    while (temp != NULL) {
        count++;
        temp = temp->next;
    }
    return count;
}

void duplicarInimigos() {
    int numInimigos = contarInimigos(inimigos);
    printf("Dobrando inimigos! Número atual: %d, Novo número: %d\n", numInimigos, numInimigos * 2); // Debug
    for (int i = 0; i < numInimigos; i++) {
        Inimigo* novoInimigo = criarInimigo();
        adicionarInimigo(&inimigos, novoInimigo);
    }
}

void moverMachadoEAtacar() {
    if (machado.ativo) {
        moverMachado();
        // Verifica se o machado atingiu algum inimigo
        Node* temp = inimigos;
        while (temp != NULL) {
            Inimigo* inimigo = (Inimigo*)temp->data;
            if (machado.ativo && inimigo->ativo && machado.x == inimigo->x && machado.y == inimigo->y) {
                machado.ativo = 0; // Desativa o machado
                inimigo->vida -= 100; // Machado causa 100 de dano
                if (inimigo->vida <= 0) {
                    inimigo->ativo = 0; // Inimigo morto
                    pontuacao += 100;   // Incrementa a pontuação em 100
                    printf("Inimigo derrotado! Pontuação atual: %d\n", pontuacao); // Debug
                }
            }
            temp = temp->next;
        }
    }
}

void liberarInimigos(Node** lista) {
    Node* temp = *lista;
    while (temp != NULL) {
        Inimigo* inimigo = (Inimigo*)temp->data;
        free(inimigo); // Libera o inimigo
        temp = temp->next;
    }
    liberarLista(lista); // Libera a lista
}

void salvarPontuacao(char* nome, double tempo, int pontuacao) {
    printf("Tentando salvar a pontuação...\n"); // Debug

    // Obter o diretório do executável
    char diretorio[PATH_MAX];
    obterDiretorioExecutavel(diretorio, sizeof(diretorio));
    printf("Diretório do executável: %s\n", diretorio); // Debug

    // Construir o caminho completo para scores.txt
    char caminhoScores[PATH_MAX];
    snprintf(caminhoScores, sizeof(caminhoScores), "%s/scores.txt", diretorio);

    // Abrir o arquivo no modo de adição
    FILE* arquivo = fopen(caminhoScores, "a");
    if (arquivo != NULL) {
        fprintf(arquivo, "Nome: %s | Tempo: %.1f segundos | Pontuação: %d\n", nome, tempo, pontuacao);
        fclose(arquivo);
        printf("Pontuação salva com sucesso em %s.\n", caminhoScores); // Debug
    } else {
        perror("Erro ao abrir o arquivo para salvar a pontuação");
    }
}

int main() {
    srand(time(NULL)); // Inicializa o gerador de números aleatórios
    initSpawnPositions(); // Inicializa as posições de spawn

    // Solicita o nome do jogador
    printf("Digite seu nome: ");
    fgets(nomeJogador, sizeof(nomeJogador), stdin);
    // Remove o caractere de nova linha caso exista
    nomeJogador[strcspn(nomeJogador, "\n")] = '\0';
    printf("Nome do jogador: %s\n", nomeJogador); // Debug

    char input;
    int frameCount = 0;
    double tempoDecorrido = 0.0;
    double nextEnemyIncreaseTime = 5.5;
    int gameOver = 0; // Flag de game over
    keyboardInit(); // Inicializa o teclado
    screenInit(1);   // Inicializa a tela
    printf("Use WASD para mover o objeto. Pressione 'q' para sair. Pressione 'm' para lançar o machado.\n");

    // Inicializa a lista de inimigos com um inimigo
    Inimigo* inimigoInicial = criarInimigo();
    adicionarInimigo(&inimigos, inimigoInicial);

    while (1) {
        // Verifica entrada do usuário sem bloquear
        if (keyhit()) {
            input = getchar();
            if (input == 'q') {
                salvarPontuacao(nomeJogador, tempoDecorrido, pontuacao); // Salva a pontuação
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
        moverMachadoEAtacar();
        // Incrementa o contador de frames
        frameCount++;
        // Atualiza o tempo decorrido
        tempoDecorrido += FRAME_TIME / 1000000.0; // Convertendo microsegundos para segundos

        // Dobra o número de inimigos a cada 5.5 segundos
        if (tempoDecorrido >= nextEnemyIncreaseTime) {
            nextEnemyIncreaseTime += 5.5;
            duplicarInimigos();
        }

        // Move os inimigos a cada 10 frames para torná-los mais lentos
        if (frameCount % 10 == 0) {
            Node* temp = inimigos;
            while (temp != NULL) {
                Inimigo* inimigo = (Inimigo*)temp->data;
                if (inimigo->ativo && inimigo->vida > 0) {
                    moverInimigo(inimigo, &obj);
                }
                temp = temp->next;
            }
        }

        // Verifica colisão entre os inimigos e o jogador
        Node* temp = inimigos;
        while (temp != NULL) {
            Inimigo* inimigo = (Inimigo*)temp->data;
            if (inimigo->ativo && inimigo->x == obj.x && inimigo->y == obj.y) {
                gameOver = 1;
                break;
            }
            temp = temp->next;
        }

        if (gameOver) {
            screenClear();
            printf("Game Over! Você foi pego pelo inimigo.\n");
            printf("Pontuação final: %d\n", pontuacao);
            printf("Tempo sobrevivido: %.1f segundos\n", tempoDecorrido);
            printf("Pressione 'q' para sair e salvar a pontuação.\n");

            // Aguarda o jogador pressionar 'q' para sair
            while (1) {
                if (keyhit()) {
                    input = getchar();
                    if (input == 'q') {
                        salvarPontuacao(nomeJogador, tempoDecorrido, pontuacao); // Salva a pontuação
                        break;
                    }
                }
                usleep(FRAME_TIME); // Pausa para evitar alto uso de CPU
            }
            break;
        }

        // Atualiza a tela
        atualizarTela(&obj, &machado, tempoDecorrido);
        // Controla a taxa de quadros
        usleep(FRAME_TIME); // Pausa para manter a taxa de quadros
    }

    keyboardDestroy(); // Restaura o terminal
    screenDestroy();   // Restaura a tela
    freeSpawnPositions(); // Libera a lista de posições de spawn
    liberarInimigos(&inimigos); // Libera a lista de inimigos
    return 0;
}