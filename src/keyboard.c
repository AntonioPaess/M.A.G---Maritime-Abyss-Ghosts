/**
 * keyboard.h
 * Created on Aug, 23th 2023
 * Author: Tiago Barros
 * Based on "From C to C++ course - 2002"
*/

#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#include "keyboard.h"

static struct termios initialSettings, newSettings;
static int peekCharacter;


void keyboardInit()
{
    tcgetattr(0,&initialSettings);
    newSettings = initialSettings;
    newSettings.c_lflag &= ~ICANON;
    newSettings.c_lflag &= ~ECHO;
    newSettings.c_lflag &= ~ISIG;
    newSettings.c_cc[VMIN] = 1;
    newSettings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &newSettings);
}

void keyboardDestroy()
{
    tcsetattr(0, TCSANOW, &initialSettings);
}

int keyhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Desabilita entrada canonica e eco
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0); // Salva flags antigas
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK); // Modo não bloqueante

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restaura terminal
    fcntl(STDIN_FILENO, F_SETFL, oldf); // Restaura flags

    if (ch != EOF) {
        ungetc(ch, stdin); // Devolve caractere
        return 1;
    }

    return 0;
}

int readch()
{
    char ch;

    if(peekCharacter != -1)
    {
        ch = peekCharacter;
        peekCharacter = -1;
        return ch;
    }
    read(0,&ch,1);
    return ch;
}
