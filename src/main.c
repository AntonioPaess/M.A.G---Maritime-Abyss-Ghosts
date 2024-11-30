#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include "keyboard.h"
#include "screen.h"
#include "listaEncadeada.h"
#include "inimigo.h"
#include "player.h"
#include "menu.h"
#include "boss.h"
#include "projeteis.h"
#include "util.h"
#include "globals.h"
#include "drops.h"

// Sistema de pontuação

int pontuacao = 0;
int gameOver = 0;

double tempoDecorrido = 0.0;
char nomeJogador[50];
int y = 1;

int main()
{
    screenClear();
    srand(time(NULL));
    initSpawnPositions();
    inicializarDrops();
    keyboardInit();
    screenInit(0);

    while (1)
    {
        int opcao = mostrarTelaInicial();

        switch (opcao)
        {
        case 0: // Iniciar Jogo
            screenClear();

            // Configuração inicial do terminal
            struct winsize w;
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
            int terminalWidth = w.ws_col;
            int terminalHeight = w.ws_row;

            const char *titulo = "Digite seu nome:";
            const char *instrucoes[] = {
                "             [W]             [↑]         ",
                "              [A][S][D]       [←][↓][→]      ",
                "                                      ",
                "       Movimento      Jato D'água   "};

            const char *lore[] = {
                "Após naufragar no oceano, Gronkarr desperta como um peixe (🐟) que cospe água.",
                "Ele deve enfrentar criaturas das profundezas marinhas.",
                "Seu maior desafio será derrotar Leviatã, o dragão aquático."};

            // Calcula a largura da janela do terminal
            int paddingVertical = terminalHeight / 2 - 8; // Ajusta a posição vertical

            // Exibe a lore centralizada
            for (int i = 0; i < 3; i++)
            {
                int padding = (terminalWidth - strlen(lore[i])) / 2;
                screenGotoxy(padding, paddingVertical + i);
                printf("\033[93m%s\033[0m", lore[i]);
            }

            // Exibe as instruções centralizadas
            paddingVertical += 5; // Ajusta a posição vertical para as instruções
            for (int i = 0; i < 4; i++)
            {
                int padding = (terminalWidth - strlen(instrucoes[i])) / 2;
                screenGotoxy(padding, paddingVertical + i);
                printf("\033[96m%s\033[0m", instrucoes[i]);
            }

            // Exibe o título centralizado
            screenGotoxy((terminalWidth - strlen(titulo)) / 2, terminalHeight / 2 + 6);
            printf("\033[94m%s\033[0m", titulo);

            // Exibe o prompt centralizado
            screenGotoxy((terminalWidth - 20) / 2, terminalHeight / 2 + 8);
            printf("\033[92m> \033[0m");

            // Captura do nome
            size_t index = 0;
            char ch;
            while ((ch = getchar()) != '\n' && index < sizeof(nomeJogador) - 1)
            {
                if (ch == 127 || ch == '\b')
                {
                    if (index > 0)
                    {
                        index--;
                        nomeJogador[index] = '\0';
                    }
                }
                else
                {
                    nomeJogador[index++] = ch;
                    nomeJogador[index] = '\0';
                }
                screenGotoxy((terminalWidth - 20) / 2 + 2, terminalHeight / 2 + 8); // Ajuste a posição vertical aqui
                printf("\033[92m%-20s\033[0m", nomeJogador);
                fflush(stdout);
            }
            nomeJogador[index] = '\0';

            // Inicialização das variáveis do jogo
            reiniciarJogo();
            char input;
            int frameCount = 0;
            tempoDecorrido = 0.0;
            double nextEnemyIncreaseTime = 5.5;
            int gameOver = 0;
            int inimigosCongelados = 0;
            double tempoCongelamentoInicio = 0.0;

            // Inicializa o primeiro inimigo
            Inimigo *inimigoInicial = criarInimigo();
            adicionarInimigo(&inimigos, inimigoInicial);

            // Loop principal do jogo
            Node *temp;
            while (1)
            {
                if (keyhit())
                {
                    input = getchar();
                    if (input == 'q')
                    {
                        salvarPontuacao(nomeJogador, tempoDecorrido, pontuacao);
                        break;
                    }

                    if (input == '\033')
                    {
                        getchar();
                        input = getchar();
                        switch (input)
                        {
                        case 'A':
                            if (!machado.ativo)
                            {
                                lastDir = 'w';
                                iniciarMovimentoMachado();
                            }
                            break;
                        case 'B':
                            if (!machado.ativo)
                            {
                                lastDir = 's';
                                iniciarMovimentoMachado();
                            }
                            break;
                        case 'C':
                            if (!machado.ativo)
                            {
                                lastDir = 'd';
                                iniciarMovimentoMachado();
                            }
                            break;
                        case 'D':
                            if (!machado.ativo)
                            {
                                lastDir = 'a';
                                iniciarMovimentoMachado();
                            }
                            break;
                        }
                    }
                    else
                    {
                        moverObjeto(&obj, input);
                    }
                }

                // Calcular deltaTime (tempo entre frames)
                static clock_t ultimoTempo = 0;
                clock_t tempoAtual = clock();
                float deltaTime = (float)(tempoAtual - ultimoTempo) / CLOCKS_PER_SEC;
                ultimoTempo = tempoAtual;

                // Atualizar drops
                atualizarDrops(deltaTime);
                verificarColetaDrops(&obj);
                atualizarTela(&obj, &machado, tempoDecorrido);
                desenharDrops();

                // Lógica do jogo
                moverMachadoEAtacar();
                frameCount++;
                tempoDecorrido += FRAME_TIME / 1000000.0;

                // Verifica se o boss deve aparecer
                verificarSpawnBoss(tempoDecorrido, pontuacao);

                // Atualiza o boss se estiver ativo
                if (boss.ativo)
                {
                    atacarBoss(tempoDecorrido);
                    moverProjeteisBoss();
                    moverBossQuadrado();
                }

                if (tempoDecorrido >= nextEnemyIncreaseTime)
                {
                    nextEnemyIncreaseTime += 5.5;
                    duplicarInimigos(tempoDecorrido);
                }

                // Lógica de congelamento
                if (inimigosCongelados && (tempoDecorrido - tempoCongelamentoInicio) >= 2.0)
                {
                    inimigosCongelados = 0;
                }

                // Movimento dos inimigos
                if (!inimigosCongelados && frameCount % 10 == 0)
                {
                    temp = inimigos;
                    while (temp != NULL)
                    {
                        Inimigo *inimigo = (Inimigo *)temp->data;
                        if (inimigo->ativo && inimigo->vida > 0)
                        {
                            moverInimigo(inimigo, &obj);
                        }
                        temp = temp->next;
                    }
                }

                // Verificação de colisão
                temp = inimigos;
                while (temp != NULL)
                {
                    Inimigo *inimigo = (Inimigo *)temp->data;

                    // Verifica colisão com inimigo
                    if (inimigo->ativo && inimigo->x == obj.x && inimigo->y == obj.y)
                    {
                        if (playerShield.ativo) {
                            reduzirShield();
                            // Ao invés de return, usamos continue para pular o dano
                            inimigosCongelados = 1;
                            tempoCongelamentoInicio = tempoDecorrido;
                        }
                        else {
                            // Aplica dano normal se não tiver shield
                            aplicarDano(&obj, 1);
                            if ((obj.vidas - obj.dano / DANO_POR_VIDA) <= 0)
                            {
                                gameOver = 1;
                            }
                            else
                            {
                                inimigosCongelados = 1;
                                tempoCongelamentoInicio = tempoDecorrido;
                            }
                        }
                        break; // Sai do loop após tratar a colisão
                    }
                    temp = temp->next;
                }

                // Dentro do loop que verifica colisões ou ataques
                temp = inimigos;
                while (temp != NULL) {
                    Inimigo *inimigo = (Inimigo *)temp->data;

                    // Verificar se o inimigo foi atingido pelo machado
                    if (inimigo->ativo && inimigo->x == machado.x && inimigo->y == machado.y) {
                        inimigo->vida -= 100; // Substitua por quanto dano o machado causa

                        if (inimigo->vida <= 0) {
                            inimigo->ativo = 0;
                            pontuacao += 10; // Incrementa a pontuação

                            // Chama a função para tentar criar um drop
                            tentarCriarDrop(inimigo->x, inimigo->y);
                        }
                    }

                    temp = temp->next;
                }

                if (gameOver)
                {
                    mostrarTelaGameOver(tempoDecorrido, pontuacao);
                    break;
                }
                if (youWin)
                {
                    mostrarTelaYouWin(tempoDecorrido, pontuacao);
                    break;
                }

                atualizarTela(&obj, &machado, tempoDecorrido);
                usleep(FRAME_TIME);
            }

            // Limpa os recursos do jogo atual
            liberarInimigos(&inimigos);
            break;

        case 1: // Hall da Fama
            mostrarHallDaFama();
            break;

        case 2: // Sair
            keyboardDestroy();
            screenDestroy();
            freeSpawnPositions();
            exit(0);
            break;

        default:
            printf("\nOpção inválida. Tente novamente.\n");
            while (getchar() != '\n')
                ;
            break;
        }
    }

    keyboardDestroy();
    screenDestroy();
    freeSpawnPositions();
    return 0;
}