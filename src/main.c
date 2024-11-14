#include <stdio.h>
#include <unistd.h> // Para usleep, getcwd
#include <time.h>
#include <stdlib.h>      // malloc, rand, srand, abs, exit
#include <string.h>      // Para manipulação de strings
#include <limits.h>      // Para PATH_MAX
#include <mach-o/dyld.h> // Para _NSGetExecutablePath
#include <libgen.h>      // Para dirname
#include <sys/ioctl.h>   // Para obter o tamanho do terminal

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

typedef struct
{
    int x;
    int y;
    int vidas; // Número de vidas do jogador
} Objeto;

typedef struct
{
    int x;
    int y;
    int vida;
    int ativo; // Indica se o inimigo está ativo
} Inimigo;

typedef struct
{
    int x;
    int y;
    int ativo;
    char direcao;
    int distancia;
    int moveCounter; // Contador para controlar a frequência de movimento
} Machado;

Objeto obj = {MAP_WIDTH / 2, MAP_HEIGHT / 2, 18};                // Posição inicial do jogador com 3 vidas
Machado machado = {0, 0, 0, ' ', 0, 0}; // Inicializa o machado como inativo
Node *inimigos = NULL;                  // Lista de inimigos
Node *spawnPositions = NULL;
char lastDir = 'd';   // Direção padrão inicial (direita)
char nomeJogador[50]; // Armazena o nome do jogador

void obterDiretorioExecutavel(char *diretorio, size_t tamanho)
{
    char path[PATH_MAX];
    uint32_t size = sizeof(path);

    if (_NSGetExecutablePath(path, &size) != 0)
    {
        fprintf(stderr, "Buffer muito pequeno. Necessário tamanho: %u\n", size);
        exit(EXIT_FAILURE);
    }

    // Remove o nome do executável para obter apenas o diretório
    char *dir = dirname(path);
    strncpy(diretorio, dir, tamanho - 1);
    diretorio[tamanho - 1] = '\0'; // Garante que a string está terminada em null
}

void initSpawnPositions()
{
    spawnPositions = criarLista();
    for (int x = MINX; x <= MAXX; x++)
    {
        for (int y = MINY; y <= MAXY; y++)
        {
            // Evita spawnar na posição do jogador ou do machado
            if ((x == obj.x && y == obj.y) || (x == machado.x && y == machado.y))
            {
                continue;
            }
            int *pos = (int *)malloc(2 * sizeof(int));
            pos[0] = x;
            pos[1] = y;
            inserirFim(&spawnPositions, pos);
        }
    }
}

void freeSpawnPositions()
{
    liberarLista(&spawnPositions);
}

void getRandomSpawnPosition(int *x, int *y)
{
    int count = 0;
    Node *temp = spawnPositions;
    while (temp != NULL)
    {
        count++;
        temp = temp->next;
    }
    if (count == 0)
        return; // Evita divisão por zero
    int randomIndex = rand() % count;
    temp = spawnPositions;
    for (int i = 0; i < randomIndex; i++)
    {
        temp = temp->next;
    }
    int *pos = (int *)temp->data;
    *x = pos[0];
    *y = pos[1];
}

void moverMachado()
{
    if (machado.ativo)
    {
        machado.moveCounter++;
        if (machado.moveCounter % 3 == 0)
        { // Mover a cada 3 frames (mais lento)
            switch (machado.direcao)
            {
            case 'w':
                machado.y--;
                break; // Move para cima
            case 's':
                machado.y++;
                break; // Move para baixo
            case 'a':
                machado.x--;
                break; // Move para a esquerda
            case 'd':
                machado.x++;
                break; // Move para a direita
            }
            machado.distancia--;
            if (machado.distancia <= 0 || machado.x < MINX || machado.x > MAXX || machado.y < MINY || machado.y > MAXY)
            {
                machado.ativo = 0; // Desativa o machado
            }
        }
    }
}

void moverObjeto(Objeto *obj, char direcao)
{
    switch (direcao)
    {
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

void moverInimigo(Inimigo *inimigo, Objeto *obj)
{
    // Calcula a diferença em x e y
    int deltaX = obj->x - inimigo->x;
    int deltaY = obj->y - inimigo->y;

    if (abs(deltaX) > abs(deltaY))
    {
        // Move no eixo x
        if (deltaX > 0)
        {
            inimigo->x += 1;
        }
        else if (deltaX < 0)
        {
            inimigo->x -= 1;
        }
    }
    else
    {
        // Move no eixo y
        if (deltaY > 0)
        {
            inimigo->y += 1;
        }
        else if (deltaY < 0)
        {
            inimigo->y -= 1;
        }
    }
}

void atualizarTela(Objeto *obj, Machado *machado, double tempoDecorrido)
{
    screenClear(); // Limpa a tela
    // Desenha o jogador
    screenGotoxy(obj->x, obj->y);
    printf("🐟");
    // Desenha os inimigos
    Node *temp = inimigos;
    while (temp != NULL)
    {
        Inimigo *inimigo = (Inimigo *)temp->data;
        if (inimigo->ativo && inimigo->vida > 0)
        {
            screenGotoxy(inimigo->x, inimigo->y);
            printf("🦈");
        }
        temp = temp->next;
    }
    // Desenha o machado se estiver ativo
    if (machado->ativo)
    {
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

void iniciarMovimentoMachado()
{
    char dir;
    // Determina a direção
    switch (lastDir)
    {
    case 'w':
        dir = 'w';
        break;
    case 's':
        dir = 's';
        break;
    case 'a':
        dir = 'a';
        break;
    case 'd':
        dir = 'd';
        break;
    default:
        dir = 'd'; // Direção padrão
    }
    machado.ativo = 1;
    machado.direcao = dir;
    machado.distancia = 8;   // Aumenta a distância para 8 blocos
    machado.moveCounter = 0; // Inicializa o contador
    machado.x = obj.x;
    machado.y = obj.y;
}

Inimigo *criarInimigo()
{
    Inimigo *novoInimigo = (Inimigo *)malloc(sizeof(Inimigo));
    getRandomSpawnPosition(&novoInimigo->x, &novoInimigo->y); // Define posição aleatória
    novoInimigo->vida = 100;
    novoInimigo->ativo = 1;
    return novoInimigo;
}
int contarInimigosAtivos(Node *lista)
{
    int count = 0;
    Node *temp = lista;
    while (temp != NULL)
    {
        Inimigo *inimigo = (Inimigo *)temp->data;
        if (inimigo->ativo && inimigo->vida > 0)
        {
            count++;
        }
        temp = temp->next;
    }
    return count;
}

void adicionarInimigo(Node **lista, Inimigo *inimigo)
{
    inserirFim(lista, inimigo);
}

int contarInimigos(Node *lista)
{
    int count = 0;
    Node *temp = lista;
    while (temp != NULL)
    {
        count++;
        temp = temp->next;
    }
    return count;
}

// Substitua a função duplicarInimigos por esta nova versão
void duplicarInimigos(double tempoAtual)
{
    int numInimigosDesejados;

    // Determina quantos inimigos devem existir com base no tempo
    if (tempoAtual >= TEMPO_16_INIMIGOS)
    {
        numInimigosDesejados = 16;
    }
    else if (tempoAtual >= TEMPO_8_INIMIGOS)
    {
        numInimigosDesejados = 8;
    }
    else if (tempoAtual >= TEMPO_4_INIMIGOS)
    {
        numInimigosDesejados = 4;
    }
    else if (tempoAtual >= TEMPO_2_INIMIGOS)
    {
        numInimigosDesejados = 2;
    }
    else
    {
        numInimigosDesejados = 1;
    }

    int numInimigosAtivos = contarInimigosAtivos(inimigos);

    // Se precisamos adicionar mais inimigos
    if (numInimigosAtivos < numInimigosDesejados)
    {
        int inimigosParaAdicionar = numInimigosDesejados - numInimigosAtivos;
        printf("Adicionando inimigos! De %d para %d\n", numInimigosAtivos, numInimigosDesejados);

        for (int i = 0; i < inimigosParaAdicionar; i++)
        {
            Inimigo *novoInimigo = criarInimigo();
            adicionarInimigo(&inimigos, novoInimigo);
        }
    }
}

void moverMachadoEAtacar()
{
    if (machado.ativo)
    {
        moverMachado();
        // Verifica se o machado atingiu algum inimigo
        Node *temp = inimigos;
        while (temp != NULL)
        {
            Inimigo *inimigo = (Inimigo *)temp->data;
            if (machado.ativo && inimigo->ativo && machado.x == inimigo->x && machado.y == inimigo->y)
            {
                machado.ativo = 0;    // Desativa o machado
                inimigo->vida -= 100; // Machado causa 100 de dano
                if (inimigo->vida <= 0)
                {
                    inimigo->ativo = 0;                                            // Inimigo morto
                    pontuacao += 100;                                              // Incrementa a pontuação em 100
                    printf("Inimigo derrotado! Pontuação atual: %d\n", pontuacao); // Debug
                }
            }
            temp = temp->next;
        }
    }
}

void liberarInimigos(Node **lista)
{
    Node *temp = *lista;
    while (temp != NULL)
    {
        Inimigo *inimigo = (Inimigo *)temp->data;
        free(inimigo); // Libera o inimigo
        temp = temp->next;
    }
    liberarLista(lista); // Libera a lista
}

void salvarPontuacao(char *nome, double tempo, int pontuacao)
{
    printf("Tentando salvar a pontuação...\n"); // Debug

    // Obter o diretório do executável
    char diretorio[PATH_MAX];
    obterDiretorioExecutavel(diretorio, sizeof(diretorio));
    printf("Diretório do executável: %s\n", diretorio); // Debug

    // Construir o caminho completo para scores.txt
    char caminhoScores[PATH_MAX];
    snprintf(caminhoScores, sizeof(caminhoScores), "%s/scores.txt", diretorio);

    // Abrir o arquivo no modo de adição
    FILE *arquivo = fopen(caminhoScores, "a");
    if (arquivo != NULL)
    {
        // Salva em formato CSV: nome,tempo,pontuacao
        fprintf(arquivo, "%s,%.1f,%d\n", nome, tempo, pontuacao);
        fclose(arquivo);
        printf("Pontuação salva com sucesso em %s.\n", caminhoScores); // Debug
    }
    else
    {
        perror("Erro ao abrir o arquivo para salvar a pontuação");
    }
}

void mostrarHallDaFama()
{
    screenClear();
    screenGotoxy(1, 1);
    printf("Hall da Fama:\n\n");

    char diretorio[PATH_MAX];
    obterDiretorioExecutavel(diretorio, sizeof(diretorio));

    char caminhoScores[PATH_MAX];
    snprintf(caminhoScores, sizeof(caminhoScores), "%s/scores.txt", diretorio);

    FILE *arquivo = fopen(caminhoScores, "r");
    if (arquivo != NULL)
    {
        typedef struct
        {
            char nome[50];
            double tempo;
            int pontuacao;
        } Registro;

        Registro registros[100];
        int count = 0;

        char linha[256];
        while (fgets(linha, sizeof(linha), arquivo))
        {
            if (sscanf(linha, "%49[^,],%lf,%d", registros[count].nome, &registros[count].tempo, &registros[count].pontuacao) == 3)
            {
                count++;
            }
        }
        fclose(arquivo);

        // Ordena os registros em ordem decrescente de pontuação
        for (int i = 0; i < count - 1; i++)
        {
            for (int j = i + 1; j < count; j++)
            {
                if (registros[j].pontuacao > registros[i].pontuacao)
                {
                    Registro temp = registros[i];
                    registros[i] = registros[j];
                    registros[j] = temp;
                }
            }
        }

        // Exibe os registros ordenados
        for (int i = 0; i < count; i++)
        {
            printf("%d. Nome: %s | Tempo: %.1f s | Pontuação: %d\n", i + 1, registros[i].nome, registros[i].tempo, registros[i].pontuacao);
        }
    }
    else
    {
        printf("Nenhuma pontuação disponível.\n");
    }
    printf("\nPressione Enter para voltar.\n");
    getchar();
}

void mostrarTelaInicial()
{
    screenClear();
    y = 1; // Reset da variável global y

    // Obter o tamanho do terminal
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int terminalWidth = w.ws_col;
    int terminalHeight = w.ws_row;

    // Ler e exibir o conteúdo de "Menu.txt" centralizado
    char diretorio[PATH_MAX];
    obterDiretorioExecutavel(diretorio, sizeof(diretorio));
    char caminhoMenu[PATH_MAX];
    snprintf(caminhoMenu, sizeof(caminhoMenu), "%s/assets/Menu.txt", diretorio);

    FILE *arquivo = fopen(caminhoMenu, "r");
    if (arquivo != NULL)
    {
        char linha[256];
        while (fgets(linha, sizeof(linha), arquivo))
        {
            // Remove o caractere de nova linha se existir
            size_t len = strlen(linha);
            if (len > 0 && linha[len - 1] == '\n')
            {
                linha[len - 1] = '\0';
                len--;
            }

            // Calcula o padding necessário para centralizar usando a largura do terminal
            int padding = (terminalWidth - len) / 2;
            if (padding < 0)
                padding = 0;

            screenGotoxy(padding, y++);
            printf("%s", linha);
        }
        fclose(arquivo);
    }
    else
    {
        printf("Não foi possível carregar o menu.\n");
    }

    // Centraliza as opções do menu usando a largura do terminal
    const char *opcao1 = "1. Iniciar Jogo";
    const char *opcao2 = "2. Hall da Fama";
    const char *sair = "3. Sair";

    screenGotoxy((terminalWidth - strlen(opcao1)) / 2, y + 2);
    printf("%s", opcao1);

    screenGotoxy((terminalWidth - strlen(opcao2)) / 2, y + 3);
    printf("%s", opcao2);

    screenGotoxy((terminalWidth - strlen(sair)) / 2, y + 5);
    printf("%s", sair);
}

void mostrarTelaGameOver(double tempoDecorrido, int pontuacao)
{
    screenClear();
    y = 1; // Reset da variável global y

    // Obter o tamanho do terminal
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int terminalWidth = w.ws_col;
    int terminalHeight = w.ws_row;

    // Ler e exibir o conteúdo de "GameOver.txt" centralizado
    char diretorio[PATH_MAX];
    obterDiretorioExecutavel(diretorio, sizeof(diretorio));
    char caminhoMenu[PATH_MAX];
    snprintf(caminhoMenu, sizeof(caminhoMenu), "%s/assets/GameOver.txt", diretorio);

    FILE *arquivo = fopen(caminhoMenu, "r");
    if (arquivo != NULL)
    {
        char linha[256];
        while (fgets(linha, sizeof(linha), arquivo))
        {
            // Remove o caractere de nova linha se existir
            size_t len = strlen(linha);
            if (len > 0 && linha[len - 1] == '\n')
            {
                linha[len - 1] = '\0';
                len--;
            }

            // Calcula o padding necessário para centralizar usando a largura do terminal
            int padding = (terminalWidth - len) / 2;
            if (padding < 0)
                padding = 0;

            screenGotoxy(padding, y++);
            printf("%s", linha);
        }
        fclose(arquivo);
    }
    else
    {
        printf("Não foi possível carregar o menu.\n");
    }

    // Estatísticas do jogador
    char stats[3][100];
    sprintf(stats[0], "Jogador: %s", nomeJogador);
    sprintf(stats[1], "Pontuação final: %d", pontuacao);
    sprintf(stats[2], "Tempo sobrevivido: %.1f segundos", tempoDecorrido);

    int startY = y; // Define startY com o valor de y

    // Mostra estatísticas centralizadas
    for (int i = 0; i < 3; i++)
    {
        screenGotoxy((MAP_WIDTH - strlen(stats[i])) / 2, startY + 7 + i);
        printf("%s", stats[i]);
    }

    // Linha divisória
    screenGotoxy(0, startY + 11);
    for (int i = 0; i < MAP_WIDTH - 4; i++)
        printf("═");

    // Hall da Fama
    screenGotoxy((MAP_WIDTH - 24) / 2, startY + 13);
    printf("╔═══ HALL DA FAMA ═══╗");

    char caminhoScores[PATH_MAX];
    snprintf(caminhoScores, sizeof(caminhoScores), "%s/scores.txt", diretorio);

    FILE *arquivoScores = fopen(caminhoScores, "r");
    if (arquivoScores != NULL)
    {
        typedef struct
        {
            char nome[50];
            double tempo;
            int pontuacao;
        } Registro;

        Registro registros[100];
        int count = 0;

        char linha[256];
        while (fgets(linha, sizeof(linha), arquivoScores))
        {
            if (sscanf(linha, "%49[^,],%lf,%d", registros[count].nome, &registros[count].tempo, &registros[count].pontuacao) == 3)
            {
                count++;
            }
        }
        fclose(arquivoScores);

        // Ordena os registros em ordem decrescente de pontuação
        for (int i = 0; i < count - 1; i++)
        {
            for (int j = i + 1; j < count; j++)
            {
                if (registros[j].pontuacao > registros[i].pontuacao)
                {
                    Registro temp = registros[i];
                    registros[i] = registros[j];
                    registros[j] = temp;
                }
            }
        }

        // Exibe os registros ordenados
        int y = startY + 15;
        int rank = 1;

        // Cabeçalho da tabela
        screenGotoxy((MAP_WIDTH - 50) / 2, y++);
        printf("%-5s %-15s %-15s %-15s", "Rank", "Nome", "Tempo", "Pontuação");
        y++;

        for (int i = 0; i < count && rank <= 5; i++)
        {
            screenGotoxy((MAP_WIDTH - 50) / 2, y++);
            printf("%-5d %-15s %-15.1f %-15d", rank++, registros[i].nome, registros[i].tempo, registros[i].pontuacao);
        }
    }

    // Instruções
    screenGotoxy((MAP_WIDTH - 35) / 2, MAP_HEIGHT - 3);
    printf("Pressione '1' para salvar e voltar ao menu...\n");
    screenGotoxy((MAP_WIDTH - strlen("2.Sair")) / 2, MAP_HEIGHT - 2);
    printf("2.Sair");
}

void reiniciarJogo() {
    obj.x = MAP_WIDTH/2;
    obj.y = MAP_HEIGHT/2;
    obj.vidas = 3;
    pontuacao = 0;
    liberarInimigos(&inimigos);
    inimigos = NULL;
    machado.ativo = 0;
}

int main() {

    screenClear();
    srand(time(NULL));
    initSpawnPositions();
    keyboardInit();
    screenInit(0);

    char escolha;
    while (1) {
        mostrarTelaInicial();
        escolha = getchar();
        getchar(); // Limpa o '\n' do buffer

        if (escolha == '1') {
            screenClear();
            // Solicita o nome do jogador após escolher iniciar o jogo
            printf("Digite seu nome: ");
            fgets(nomeJogador, sizeof(nomeJogador), stdin);
            nomeJogador[strcspn(nomeJogador, "\n")] = '\0';

            // Inicialização das variáveis do jogo
            char input;
            int frameCount = 0;
            double tempoDecorrido = 2.0;
            double nextEnemyIncreaseTime = 5.5;
            int gameOver = 0;
            int inimigosCongelados = 0;
            double tempoCongelamentoInicio = 0.0;

            // Inicializa o primeiro inimigo
            Inimigo *inimigoInicial = criarInimigo();
            adicionarInimigo(&inimigos, inimigoInicial);

            printf("Use WASD para mover o objeto. Pressione as setas para lançar o machado. Pressione 'q' para sair\n");

            // Loop principal do jogo
            Node *temp;
            while (1) {
                if (keyhit()) {
                    input = getchar();
                    if (input == 'q') {
                        salvarPontuacao(nomeJogador, tempoDecorrido, pontuacao);
                        break;
                    }

                    if (input == '\033') { // Teclas especiais (setas)
                        getchar();
                        input = getchar();
                        switch (input) {
                            case 'A':
                                if (!machado.ativo) {
                                    lastDir = 'w';
                                    iniciarMovimentoMachado();
                                }
                                break;
                            case 'B':
                                if (!machado.ativo) {
                                    lastDir = 's';
                                    iniciarMovimentoMachado();
                                }
                                break;
                            case 'C':
                                if (!machado.ativo) {
                                    lastDir = 'd';
                                    iniciarMovimentoMachado();
                                }
                                break;
                            case 'D':
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

                // Lógica do jogo
                moverMachadoEAtacar();
                frameCount++;
                tempoDecorrido += FRAME_TIME / 1000000.0;

                if (tempoDecorrido >= nextEnemyIncreaseTime) {
                    nextEnemyIncreaseTime += 5.5;
                    duplicarInimigos(tempoDecorrido);
                }

                // Lógica de congelamento
                if (inimigosCongelados && (tempoDecorrido - tempoCongelamentoInicio) >= 2.0) {
                    inimigosCongelados = 0;
                }

                // Movimento dos inimigos
                if (!inimigosCongelados && frameCount % 10 == 0) {
                    temp = inimigos;
                    while (temp != NULL) {
                        Inimigo *inimigo = (Inimigo *)temp->data;
                        if (inimigo->ativo && inimigo->vida > 0) {
                            moverInimigo(inimigo, &obj);
                        }
                        temp = temp->next;
                    }
                }

                // Verificação de colisão
                temp = inimigos;
                while (temp != NULL) {
                    Inimigo *inimigo = (Inimigo *)temp->data;
                    if (inimigo->ativo && inimigo->x == obj.x && inimigo->y == obj.y) {
                        obj.vidas--;
                        if (obj.vidas <= 0) {
                            gameOver = 1;
                        } else {
                            inimigosCongelados = 1;
                            tempoCongelamentoInicio = tempoDecorrido;
                        }
                        break;
                    }
                    temp = temp->next;
                }

                if (gameOver) {
                    mostrarTelaGameOver(tempoDecorrido, pontuacao);
                    bool sair = false;
                    while (!sair) {
                        if (keyhit()) {
                            input = getchar();
                            switch(input) {
                                case '1':
                                    salvarPontuacao(nomeJogador, tempoDecorrido, pontuacao);
                                    reiniciarJogo();
                                    sair = true;
                                    break;
                                case '2':
                                    liberarInimigos(&inimigos);
                                    keyboardDestroy();
                                    screenDestroy();
                                    freeSpawnPositions();
                                    exit(0);
                                    break;
                                default:
                                    printf("\nOpção inválida. Tente novamente.\n");
                            }
                        }
                        usleep(FRAME_TIME);   
                    }
                    break; // Sai do loop do jogo para voltar ao menu inicial
                }

                atualizarTela(&obj, &machado, tempoDecorrido);
                usleep(FRAME_TIME);
            }

            // Limpa os recursos do jogo atual
            liberarInimigos(&inimigos);
        } else if (escolha == '2') {
            mostrarHallDaFama();
        } else if (escolha == '3') {
            break;
        } else {
            printf("\nOpção inválida. Tente novamente.\n");
            while (getchar() != '\n'); // Limpa o buffer de entrada
        }
    }

    // Limpeza final
    keyboardDestroy();
    screenDestroy();
    freeSpawnPositions();
    return 0;
}