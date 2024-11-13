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
#define MAX_INIMIGOS 42
#define TEMPO_2_INIMIGOS 5.5
#define TEMPO_4_INIMIGOS 11.0
#define TEMPO_8_INIMIGOS 16.5
#define TEMPO_16_INIMIGOS 22.0

// Sistema de pontuação
int pontuacao = 0;
int y = 1;

typedef struct {
    int x;
    int y;
    int vidas; // Número de vidas do jogador
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

Objeto obj = {10, 5, 18};          // Posição inicial do jogador com 3 vidas
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
    printf("🐟");
    // Desenha os inimigos
    Node* temp = inimigos;
    while (temp != NULL) {
        Inimigo* inimigo = (Inimigo*)temp->data;
        if (inimigo->ativo && inimigo->vida > 0) {
            screenGotoxy(inimigo->x, inimigo->y);
            printf("🦈");
        }
        temp = temp->next;
    }
    // Desenha o machado se estiver ativo
    if (machado->ativo) {
        screenGotoxy(machado->x, machado->y);
        printf("💦");
    }
    // Exibe a pontuação
    screenGotoxy(1, MAXY - 2);
    printf("Pontuação: %d     ", pontuacao);
    // Exibe o cronômetro
    screenGotoxy(1, MAXY - 1);
    printf("Tempo: %.1f segundos     ", tempoDecorrido);
    // Exibe o número de vidas
    screenGotoxy(1, MAXY);
    printf("Vidas: %d     ", obj->vidas);
    fflush(stdout); // Atualiza a tela
}

void iniciarMovimentoMachado() {
    char dir;
    // Determina a direção
    switch(lastDir) {
        case 'w': dir = 'w'; break;
        case 's': dir = 's'; break;
        case 'a': dir = 'a'; break;
        case 'd': dir = 'd'; break;
        default: dir = 'd'; // Direção padrão
    }
    machado.ativo = 1;
    machado.direcao = dir;
    machado.distancia = 8; // Aumenta a distância para 8 blocos
    machado.moveCounter = 0; // Inicializa o contador
    machado.x = obj.x;
    machado.y = obj.y;
}

Inimigo* criarInimigo() {
    Inimigo* novoInimigo = (Inimigo*)malloc(sizeof(Inimigo));
    getRandomSpawnPosition(&novoInimigo->x, &novoInimigo->y); // Define posição aleatória
    novoInimigo->vida = 100;
    novoInimigo->ativo = 1;
    return novoInimigo;
}
int contarInimigosAtivos(Node* lista) {
    int count = 0;
    Node* temp = lista;
    while (temp != NULL) {
        Inimigo* inimigo = (Inimigo*)temp->data;
        if (inimigo->ativo && inimigo->vida > 0) {
            count++;
        }
        temp = temp->next;
    }
    return count;
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

// Substitua a função duplicarInimigos por esta nova versão
void duplicarInimigos(double tempoAtual) {
    int numInimigosDesejados;
    
    // Determina quantos inimigos devem existir com base no tempo
    if (tempoAtual >= TEMPO_16_INIMIGOS) {
        numInimigosDesejados = 16;
    } else if (tempoAtual >= TEMPO_8_INIMIGOS) {
        numInimigosDesejados = 8;
    } else if (tempoAtual >= TEMPO_4_INIMIGOS) {
        numInimigosDesejados = 4;
    } else if (tempoAtual >= TEMPO_2_INIMIGOS) {
        numInimigosDesejados = 2;
    } else {
        numInimigosDesejados = 1;
    }

    int numInimigosAtivos = contarInimigosAtivos(inimigos);
    
    // Se precisamos adicionar mais inimigos
    if (numInimigosAtivos < numInimigosDesejados) {
        int inimigosParaAdicionar = numInimigosDesejados - numInimigosAtivos;
        printf("Adicionando inimigos! De %d para %d\n", numInimigosAtivos, numInimigosDesejados);
        
        for (int i = 0; i < inimigosParaAdicionar; i++) {
            Inimigo* novoInimigo = criarInimigo();
            adicionarInimigo(&inimigos, novoInimigo);
        }
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
        // Salva em formato CSV: nome,tempo,pontuacao
        fprintf(arquivo, "%s,%.1f,%d\n", nome, tempo, pontuacao);
        fclose(arquivo);
        printf("Pontuação salva com sucesso em %s.\n", caminhoScores); // Debug
    } else {
        perror("Erro ao abrir o arquivo para salvar a pontuação");
    }
}

void mostrarHallDaFama() {
    screenClear();
    screenGotoxy(1, 1);
    printf("Hall da Fama:\n\n");

    char diretorio[PATH_MAX];
    obterDiretorioExecutavel(diretorio, sizeof(diretorio));

    char caminhoScores[PATH_MAX];
    snprintf(caminhoScores, sizeof(caminhoScores), "%s/scores.txt", diretorio);

    FILE* arquivo = fopen(caminhoScores, "r");
    if (arquivo != NULL) {
        typedef struct {
            char nome[50];
            double tempo;
            int pontuacao;
        } Registro;

        Registro registros[100];
        int count = 0;

        char linha[256];
        while (fgets(linha, sizeof(linha), arquivo)) {
            if (sscanf(linha, "%49[^,],%lf,%d", registros[count].nome, &registros[count].tempo, &registros[count].pontuacao) == 3) {
                count++;
            }
        }
        fclose(arquivo);

        // Ordena os registros em ordem decrescente de pontuação
        for (int i = 0; i < count - 1; i++) {
            for (int j = i + 1; j < count; j++) {
                if (registros[j].pontuacao > registros[i].pontuacao) {
                    Registro temp = registros[i];
                    registros[i] = registros[j];
                    registros[j] = temp;
                }
            }
        }

        // Exibe os registros ordenados
        for (int i = 0; i < count; i++) {
            printf("%d. Nome: %s | Tempo: %.1f s | Pontuação: %d\n", i + 1, registros[i].nome, registros[i].tempo, registros[i].pontuacao);
        }
    } else {
        printf("Nenhuma pontuação disponível.\n");
    }
    printf("\nPressione Enter para voltar.\n");
    getchar();
}

void mostrarTelaInicial() {
    screenClear();
    screenGotoxy(1,1);
    // Ler e exibir o conteúdo de "Menu.txt"
    char diretorio[PATH_MAX];
    obterDiretorioExecutavel(diretorio, sizeof(diretorio));
    char caminhoMenu[PATH_MAX];
    snprintf(caminhoMenu, sizeof(caminhoMenu), "%s/assets/Menu.txt", diretorio);
    FILE* arquivo = fopen(caminhoMenu, "r");
    if (arquivo != NULL) {
        char linha[256];
        while (fgets(linha, sizeof(linha), arquivo)) {
            screenGotoxy(1, y++);
            printf("%s", linha);
        }
        fclose(arquivo);
    } else {
        printf("Não foi possível carregar o menu.\n");
    }
    // Exibir as opções abaixo da arte
    // ...código existente...
    screenGotoxy(1, y + 1);
    printf("1. Iniciar Jogo\n");
    printf("2. Hall da Fama\n");
    printf("Selecione uma opção: ");
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
    double tempoDecorrido = 2.0;
    double nextEnemyIncreaseTime = 5.5;
    int gameOver = 0; // Flag de game over
    int inimigosCongelados = 0;
    double tempoCongelamentoInicio = 0.0;

    keyboardInit(); // Inicializa o teclado
    screenInit(1);   // Inicializa a tela
    printf("Use WASD para mover o objeto. Pressione as setas para lançar o machado. Pressione 'q' para sair\n");

    // Inicializa a lista de inimigos com um inimigo
    Inimigo* inimigoInicial = criarInimigo();
    adicionarInimigo(&inimigos, inimigoInicial);

    char escolha;
    while (1) {
        mostrarTelaInicial();
        escolha = getchar();
        getchar(); // Limpa o '\n' do buffer
        if (escolha == '1') {
            break; // Inicia o jogo
        } else if (escolha == '2') {
            mostrarHallDaFama();
        } else {
            printf("Opção inválida. Tente novamente.\n");
            while ((getchar()) != '\n'); // Limpa o buffer de entrada
        }
    }

    // Declaração da variável temp antes do loop principal
    Node* temp;

while (1) {
    // Verifica entrada do usuário sem bloquear
    if (keyhit()) {
        input = getchar();
        if (input == 'q') {
            salvarPontuacao(nomeJogador, tempoDecorrido, pontuacao); // Salva a pontuação
            break;
        }

        if (input == '\033') { // Tecla especial (setas)
            getchar(); // Ignora o caractere '['
            input = getchar();
            switch(input) {
                case 'A': // Seta para cima
                    if (!machado.ativo) {
                        lastDir = 'w';
                        iniciarMovimentoMachado();
                    }
                    break;
                case 'B': // Seta para baixo
                    if (!machado.ativo) {
                        lastDir = 's';
                        iniciarMovimentoMachado();
                    }
                    break;
                case 'C': // Seta para a direita
                    if (!machado.ativo) {
                        lastDir = 'd';
                        iniciarMovimentoMachado();
                    }
                    break;
                case 'D': // Seta para a esquerda
                    if (!machado.ativo) {
                        lastDir = 'a';
                        iniciarMovimentoMachado();
                    }
                    break;
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
    duplicarInimigos(tempoDecorrido);
}

    // Garante que sempre haja no máximo 16 inimigos ativos


    // Verifica se deve descongelar os inimigos
    if (inimigosCongelados && (tempoDecorrido - tempoCongelamentoInicio) >= 2.0) {
        inimigosCongelados = 0;
    }

    // Move os inimigos apenas se não estiverem congelados
    if (!inimigosCongelados && frameCount % 10 == 0) {
        temp = inimigos;
        while (temp != NULL) {
            Inimigo* inimigo = (Inimigo*)temp->data;
            if (inimigo->ativo && inimigo->vida > 0) {
                moverInimigo(inimigo, &obj);
            }
            temp = temp->next;
        }
    }

    // Verifica colisão entre os inimigos e o jogador
    temp = inimigos;
    while (temp != NULL) {
        Inimigo* inimigo = (Inimigo*)temp->data;
        if (inimigo->ativo && inimigo->x == obj.x && inimigo->y == obj.y) {
            obj.vidas--;
            if (obj.vidas <= 0) {
                gameOver = 1;
            } else {
                // Congela os inimigos por 2 segundos
                inimigosCongelados = 1;
                tempoCongelamentoInicio = tempoDecorrido;
            }
            break;
        }
        temp = temp->next;
    }

    if (gameOver) {
        screenClear();
        printf("Game Over! Você perdeu todas as vidas.\n");
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