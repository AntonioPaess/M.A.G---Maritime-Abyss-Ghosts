#include "menu.h"
#include "util.h"
#include "screen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "menu.h"
#include "inimigo.h"    
#include "keyboard.h"    
#include "listaEncadeada.h"       
#include "globals.h"
#include "util.h"

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

        // Ordena os registros
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

        // Exibe os registros
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

void mostrarTelaGameOver(double tempoDecorrido, int pontuacao)
{
    screenClear();
    y = 1; // Reset da variável global y

    // Não precisamos obter o tamanho do terminal; usaremos MAP_WIDTH e MAP_HEIGHT

    // Ler e exibir o conteúdo de "GameOver.txt" centralizado
    char diretorio[PATH_MAX];
    obterDiretorioExecutavel(diretorio, sizeof(diretorio));
    char caminhoMenu[PATH_MAX];
    snprintf(caminhoMenu, sizeof(caminhoMenu), "%s/assets/GameOver.txt", diretorio);

    FILE *arquivo = fopen(caminhoMenu, "r");
    if (arquivo != NULL)
    {
        char linha[256];
        int colorIndex = 0;
        const char *colors[] = {
            "\033[34m", // Azul
            "\033[94m", // Azul claro
            "\033[36m", // Ciano
            "\033[96m", // Ciano claro
        };
        int numColors = sizeof(colors) / sizeof(colors[0]);

        while (fgets(linha, sizeof(linha), arquivo))
        {
            // Remove o caractere de nova linha se existir
            size_t len = strlen(linha);
            if (len > 0 && linha[len - 1] == '\n')
            {
                linha[len - 1] = '\0';
                len--;
            }

            // Calcula o padding necessário para centralizar usando MAP_WIDTH
            int padding = (MAP_WIDTH - len) / 2;
            if (padding < 0)
                padding = 0;

            // Alterna a cor do texto
            printf("%s", colors[colorIndex]);
            screenGotoxy(padding, y++);
            printf("%s\n", linha);
            colorIndex = (colorIndex + 1) % numColors;
        }
        fclose(arquivo);

        // Resetar cor
        printf("\033[0m");
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

    int startY = y; // Define startY com o valor de y atual

    // Exibe estatísticas centralizadas
    for (int i = 0; i < 3; i++)
    {
        screenGotoxy((MAP_WIDTH - strlen(stats[i])) / 2, startY + 2 + i);
        printf("%s", stats[i]);
    }

    // Linha divisória
    screenGotoxy(0, startY + 6);
    for (int i = 0; i < MAP_WIDTH - 3; i++)
        printf("═");

    // Hall da Fama
    screenGotoxy((MAP_WIDTH - 24) / 2, startY + 8);
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
        int yPos = startY + 10;
        int rank = 1;

        // Cabeçalho da tabela
        screenGotoxy((MAP_WIDTH - 50) / 2, yPos++);
        printf("%-5s %-15s %-15s %-15s", "Rank", "Nome", "Tempo", "Pontuação");
        yPos++;

        for (int i = 0; i < count && rank <= 2; i++)
        {
            screenGotoxy((MAP_WIDTH - 50) / 2, yPos++);
            printf("%-5d %-15s %-15.1f %-15d", rank++, registros[i].nome, registros[i].tempo, registros[i].pontuacao);
        }
    }

    const char *opcoes[] = {
        "1. Salvar e voltar ao menu",
        "2. Sair"};
    int numOpcoes = sizeof(opcoes) / sizeof(opcoes[0]);
    int opcaoSelecionada = 0;

    while (1)
    {
        for (int i = 0; i < numOpcoes; i++)
        {
            if (i == opcaoSelecionada)
            {
                printf("\033[7m"); // Inverte as cores para destacar a opção selecionada
            }
            screenGotoxy((MAP_WIDTH - strlen(opcoes[i])) / 2, MAP_HEIGHT - 4 + i);
            printf("%s", opcoes[i]);
            printf("\033[0m"); // Reseta as cores
        }

        int ch = getchar();
        if (ch == '\033')
        {
            getchar(); // Ignora o '['
            switch (getchar())
            {
            case 'A':
                opcaoSelecionada = (opcaoSelecionada - 1 + numOpcoes) % numOpcoes;
                break;
            case 'B':
                opcaoSelecionada = (opcaoSelecionada + 1) % numOpcoes;
                break;
            }
        }
        else if (ch == '\n')
        {
            break;
        }
    }

    switch (opcaoSelecionada)
    {
    case 0: // Salvar e voltar ao menu
        salvarPontuacao(nomeJogador, tempoDecorrido, pontuacao);
        reiniciarJogo();
        gameOver = 1; // Usa a flag existente para sair do loop do jogo
        break;
    case 1:
        liberarInimigos(&inimigos);
        keyboardDestroy();
        screenDestroy();
        freeSpawnPositions();
        exit(0);
        break;
    default:
        printf("\nOpção inválida. Tente novamente.\n");
        while (getchar() != '\n')
            ; // Limpa o buffer de entrada
        break;
    }
    gameOver = 1;
}

void mostrarTelaYouWin(double tempoDecorrido, int pontuacao)
{
    screenClear();
    y = 1; // Reset da variável global y

    // Não precisamos obter o tamanho do terminal; usaremos MAP_WIDTH e MAP_HEIGHT

    // Ler e exibir o conteúdo de "GameOver.txt" centralizado
    char diretorio[PATH_MAX];
    obterDiretorioExecutavel(diretorio, sizeof(diretorio));
    char caminhoMenu[PATH_MAX];
    snprintf(caminhoMenu, sizeof(caminhoMenu), "%s/assets/youwin.txt", diretorio);

    FILE *arquivo = fopen(caminhoMenu, "r");
    if (arquivo != NULL)
    {
        char linha[256];
        int colorIndex = 0;
        const char *colors[] = {
            "\033[34m", // Azul
            "\033[94m", // Azul claro
            "\033[36m", // Ciano
            "\033[96m", // Ciano claro
        };
        int numColors = sizeof(colors) / sizeof(colors[0]);

        while (fgets(linha, sizeof(linha), arquivo))
        {
            // Remove o caractere de nova linha se existir
            size_t len = strlen(linha);
            if (len > 0 && linha[len - 1] == '\n')
            {
                linha[len - 1] = '\0';
                len--;
            }

            // Calcula o padding necessário para centralizar usando MAP_WIDTH
            int padding = (MAP_WIDTH - len) / 2;
            if (padding < 0)
                padding = 0;

            // Alterna a cor do texto
            printf("%s", colors[colorIndex]);
            screenGotoxy(padding, y++);
            printf("%s\n", linha);
            colorIndex = (colorIndex + 1) % numColors;
        }
        fclose(arquivo);

        // Resetar cor
        printf("\033[0m");
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

    int startY = y; // Define startY com o valor de y atual

    // Exibe estatísticas centralizadas
    for (int i = 0; i < 3; i++)
    {
        screenGotoxy((MAP_WIDTH - strlen(stats[i])) / 2, startY + 2 + i);
        printf("%s", stats[i]);
    }

    // Linha divisória
    screenGotoxy(0, startY + 6);
    for (int i = 0; i < MAP_WIDTH - 3; i++)
        printf("═");

    // Hall da Fama
    screenGotoxy((MAP_WIDTH - 24) / 2, startY + 8);
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
        int yPos = startY + 10;
        int rank = 1;

        // Cabeçalho da tabela
        screenGotoxy((MAP_WIDTH - 50) / 2, yPos++);
        printf("%-5s %-15s %-15s %-15s", "Rank", "Nome", "Tempo", "Pontuação");
        yPos++;

        for (int i = 0; i < count && rank <= 2; i++)
        {
            screenGotoxy((MAP_WIDTH - 50) / 2, yPos++);
            printf("%-5d %-15s %-15.1f %-15d", rank++, registros[i].nome, registros[i].tempo, registros[i].pontuacao);
        }
    }

    const char *opcoes[] = {
        "1. Salvar e voltar ao menu",
        "2. Sair"};
    int numOpcoes = sizeof(opcoes) / sizeof(opcoes[0]);
    int opcaoSelecionada = 0;

    while (1)
    {
        for (int i = 0; i < numOpcoes; i++)
        {
            if (i == opcaoSelecionada)
            {
                printf("\033[7m"); // Inverte as cores para destacar a opção selecionada
            }
            screenGotoxy((MAP_WIDTH - strlen(opcoes[i])) / 2, MAP_HEIGHT - 4 + i);
            printf("%s", opcoes[i]);
            printf("\033[0m"); // Reseta as cores
        }

        int ch = getchar();
        if (ch == '\033')
        {
            getchar(); // Ignora o '['
            switch (getchar())
            {
            case 'A':
                opcaoSelecionada = (opcaoSelecionada - 1 + numOpcoes) % numOpcoes;
                break;
            case 'B':
                opcaoSelecionada = (opcaoSelecionada + 1) % numOpcoes;
                break;
            }
        }
        else if (ch == '\n')
        {
            break;
        }
    }

    switch (opcaoSelecionada)
    {
    case 0: // Salvar e voltar ao menu
        salvarPontuacao(nomeJogador, tempoDecorrido, pontuacao);
        reiniciarJogo();
        youWin = 0; // Usa a flag existente para sair do loop do jogo
        break;
    case 1:
        liberarInimigos(&inimigos);
        keyboardDestroy();
        screenDestroy();
        freeSpawnPositions();
        exit(0);
        break;
    default:
        printf("\nOpção inválida. Tente novamente.\n");
        while (getchar() != '\n')
            ; // Limpa o buffer de entrada
        break;
    }
    gameOver = 1;
}

int mostrarTelaInicial()
{
    screenClear();
    y = 1; // Reset da variável global y

    // Obter o tamanho do terminal
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int terminalWidth = w.ws_col;
    //int terminalHeight = w.ws_row;

    // Ler e exibir o conteúdo de "Menu.txt" centralizado
    char diretorio[PATH_MAX];
    obterDiretorioExecutavel(diretorio, sizeof(diretorio));
    char caminhoMenu[PATH_MAX];
    snprintf(caminhoMenu, sizeof(caminhoMenu), "%s/assets/Menu.txt", diretorio);

    FILE *arquivo = fopen(caminhoMenu, "r");
    if (arquivo != NULL)
    {
        char linha[256];
        int colorIndex = 0;
        const char *colors[] = {
            "\033[34m", // Blue
            "\033[94m", // Light Blue
            "\033[36m", // Cyan
            "\033[96m", // Light Cyan
        };
        int numColors = sizeof(colors) / sizeof(colors[0]);

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

            // Alterna a cor do texto
            printf("%s", colors[colorIndex]);
            screenGotoxy(padding, y++);
            printf("%s\n", linha);
            colorIndex = (colorIndex + 1) % numColors;
        }
        fclose(arquivo);

        // Reset color
        printf("\033[0m");
    }
    else
    {
        printf("Não foi possível carregar o menu.\n");
    }

    // Centraliza as opções do menu usando a largura do terminal
    const char *opcoes[] = {
        "1. Iniciar Jogo",
        "2. Hall da Fama",
        "3. Sair"};
    int numOpcoes = sizeof(opcoes) / sizeof(opcoes[0]);
    int opcaoSelecionada = 0;

    while (1)
    {
        for (int i = 0; i < numOpcoes; i++)
        {
            if (i == opcaoSelecionada)
            {
                printf("\033[7m"); // Inverte as cores para destacar a opção selecionada
            }
            screenGotoxy((terminalWidth - strlen(opcoes[i])) / 2, y + 2 + i);
            printf("%s", opcoes[i]);
            printf("\033[0m"); // Reseta as cores
        }

        int ch = getchar();
        if (ch == '\033')
        {
            getchar(); // Skip the [
            switch (getchar())
            {
            case 'A':
                opcaoSelecionada = (opcaoSelecionada - 1 + numOpcoes) % numOpcoes;
                break;
            case 'B':
                opcaoSelecionada = (opcaoSelecionada + 1) % numOpcoes;
                break;
            }
        }
        else if (ch == '\n')
        {
            break;
        }
    }

    return opcaoSelecionada;
}