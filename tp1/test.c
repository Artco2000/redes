//stdout=sp.DEVNULL,

#include "comum.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>

// suficiente para receber ate 5 mensagens de tamanho maximo
#define BUFSZ 2500
// Limite da fila de espera de conexoes
#define QUEUESZ 10
// Tamanho menor mensagem + '\n' : (kill\n)
#define MINSZ 5

struct Coord
{
    int x;
    int y;
};

struct Msg
{
    char comand[10];
    struct Coord coord;
};

bool format(char *buf, struct Msg *frmtdMsg)
{
    printf("format\n");
    // indice vetor principal
    unsigned i = 0;
    // indice dentro da palavra
    unsigned j = 0;
    // numero da palavra
    unsigned npal = 0;
    // numero da mensagem
    unsigned nmsg = 0;

    for (int i = 0; i < BUFSZ; i++)
    {
        printf("format char = %c i = %d  j = %d  npal = %d  nmsg = %d\n",buf[i], i, j, npal, nmsg);
        if (buf[i] == '*')
        {
            // fim dos dados
            return 0;
        }
        if (buf[i] == '\\' && buf[i+1] == 'n')
        {
            // nova mensagem
            nmsg++;
            j = npal = 0;
            i++;
            continue;
        }
        else if (buf[i] == ' ')
        {
            // nova palavra
            npal++;
            j = 0;
            continue;
        }
        else
        {
            switch (npal)
            {
            case 0:
                // palavra de comando
                frmtdMsg[nmsg].comand[j] = buf[i];
                j++;
                // sempre assume se tratar do ultimo caracter da palavra
                frmtdMsg[nmsg].comand[j] = '\0';
                break;

            case 1:
                // numero X
                // consersao para inteiro, multiplicacao devido a casas decimais
                frmtdMsg[nmsg].coord.x = frmtdMsg[nmsg].coord.x * 10 + buf[i] - '0';
                j++;
                break;

            case 2:
                // numero Y
                // consersao para inteiro
                frmtdMsg[nmsg].coord.y = frmtdMsg[nmsg].coord.x * 10 + buf[i] - '0';
                j++;
                break;

            default:
                return -1;
                break;
            }
        }
    }
    return -1;
}

int main(int argc, char **argv)
{
    struct Coord data[50];
    printf("%ld\n",sizeof(*data)/sizeof(struct Coord));
    char buf[BUFSZ];
    memset(buf, '*', BUFSZ);
    fgets(buf, BUFSZ, stdin);
    printf("%s",buf);
    struct Msg frmtdMsg[BUFSZ / MINSZ];
    memset(frmtdMsg, 0, sizeof(frmtdMsg[0]) * floor(BUFSZ / MINSZ));
    if (format(buf, frmtdMsg) != 0)
    {
        printf("deu pau\n");
    }
    else
    {
        perror("error sinistro!");
        exit(EXIT_SUCCESS);
        printf("deu bom \n");
        int i = 0;
        while (frmtdMsg[i].comand[0] != 0)
        {
            printf("%s %d %d\n", frmtdMsg[i].comand, frmtdMsg[i].coord.x, frmtdMsg[i].coord.x);
            i++;
        }
        printf("Comparacao = %d\n",strcmp("test",frmtdMsg[0].comand));
    }

    return 0;
}