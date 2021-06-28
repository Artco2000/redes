#include "comum.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <float.h>

#include <sys/socket.h>
#include <sys/types.h>

// Tamanho Buffer recebimento
#define BUFSZ 500
// Limite da fila de espera de conexoes
#define QUEUESZ 10
// Tamanho menor mensagem + '\n' : (kill\n)
#define MINSZ 5
// Quantidade coordenadas armazenadas
#define DATASIZE 50

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

void usage(int argc, char **argv)
{
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int format(char *buf, int size, struct Msg *frmtdMsg)
{

    // indice vetor principal
    unsigned i = 0;
    // indice dentro da palavra
    unsigned j = 0;
    // numero da palavra
    unsigned npal = 0;
    // numero da mensagem
    unsigned nmsg = 0;

    for (; i < BUFSZ; i++)
    {
        if (buf[i] == 0)
        {
            // fim dos dados
            if (i > 0)
            {
                if (buf[i - 1] == '\n')
                    return 0;
                else
                    return -1;
            }
            return 0;
        }
        if (buf[i] == '\n')
        {
            // nova mensagem
            nmsg++;
            j = npal = 0;
        }
        else if (buf[i] == ' ')
        {
            // nova palavra
            npal++;
            j = 0;
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
                frmtdMsg[nmsg].coord.y = frmtdMsg[nmsg].coord.y * 10 + buf[i] - '0';
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

void add(struct Coord *coord, struct Coord *data, unsigned size, char *toSend)
{
    //Check if coodinates are valid
    if ((coord->x < 0 || coord->x > 9999) || (coord->y < 0 || coord->y > 9999))
    {
        sprintf(toSend, "invalid coordinates\n");
        return;
    }

    //Search for repeated coordinates
    for (size_t i = 0; i < size; i++)
    {
        if (data[i].x == coord->x && data[i].y == coord->y)
        {
            sprintf(toSend, "%d %d already exists\n", coord->x, coord->y);
            return;
        }
    }

    //Search for first free space
    for (size_t i = 0; i < size; i++)
    {
        if (data[i].x == -1)
        {
            data[i].x = coord->x;
            data[i].y = coord->y;
            sprintf(toSend, "%d %d added\n", coord->x, coord->y);
            return;
        }
    }
    sprintf(toSend, "limit exceeded\n");
    return;
}

void rm(struct Coord *coord, struct Coord *data, unsigned size, char *toSend)
{
    //Check if coodinates are valid
    if ((coord->x < 0 || coord->x > 9999) || (coord->y < 0 || coord->y > 9999))
    {
        sprintf(toSend, "invalid coordinates\n");
        return;
    }

    //Search for match
    for (size_t i = 0; i < size; i++)
    {
        if (data[i].x == coord->x && data[i].y == coord->y)
        {
            printf("Oi, eu achei um match!\n");
            // Shift left 1 position all coordinates after removed one
            for (size_t j = i; j < size; j++)
            {
                if (j == 49)
                {
                    data[j].x = -1;
                    data[j].y = -1;
                }
                else
                {
                    data[j].x = data[j + 1].x;
                    data[j].y = data[j + 1].y;
                }
            }
            sprintf(toSend, "%d %d removed\n", coord->x, coord->y);
            return;
        }
    }

    // did not find a match
    sprintf(toSend, "%d %d does not exist\n", coord->x, coord->y);
    return;
}

void list(struct Coord *data, unsigned size, char *toSend)
{
    char aux[10] = {0};
    for (size_t i = 0; i < size; i++)
    {
        if (data[i].x != -1)
        {
            sprintf(aux, "%d %d ", data[i].x, data[i].y);
            strcat(toSend, aux);
        }
    }
    if (strlen(toSend) == 0)
    {
        sprintf(toSend, "none\n");
        return;
    }
    //removes last space and adds ending character
    toSend[strlen(toSend) - 1] = '\n';
    return;
}

double dist(struct Coord *coord1, struct Coord *coord2)
{
    double aux = 0;
    aux = sqrt(pow((coord1->x - coord2->x), 2) + pow((coord1->y - coord2->y), 2));
    return aux;
}

void query(struct Coord *coord, struct Coord *data, unsigned size, char *toSend)
{
    //Check if coodinates are valid
    if ((coord->x < 0 || coord->x > 9999) || (coord->y < 0 || coord->y > 9999))
    {
        sprintf(toSend, "invalid coordinates\n");
        return;
    }

    struct Coord closest;
    closest.x = -1;
    closest.y = -1;
    double minDist = DBL_MAX;

    for (size_t i = 0; i < size; i++)
    {
        if (data[i].x != -1)
        {
            if (dist(coord, &data[i]) < minDist)
            {
                minDist = dist(coord, &data[i]);
                closest.x = data[i].x;
                closest.y = data[i].y;
            }
        }
    }

    if (minDist != DBL_MAX)
    {
        sprintf(toSend, "%d %d\n", closest.x, closest.y);
        return;
    }

    sprintf(toSend, "none\n");
    return;
}

int main(int argc, char **argv)
{
    /*
    =============
    Inicializacao 
    =============
    */

    struct Coord data[DATASIZE];
    for (size_t i = 0; i < 50; i++)
    {
        data[i].x = data[i].y = -1;
    }

    if (argc < 3)
    {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    // Funcao personalizada contida em comum.h
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage))
    {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1)
    {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)))
    {
        logexit("setsockopt");
    }

    // Associacao de porta ao socket
    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage)))
    {
        logexit("bind");
    }

    if (0 != listen(s, QUEUESZ))
    {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("[log] bound to %s, waiting connections\n", addrstr);

    /* 
    =========================
    Processamento de clientes
    =========================
    */

    while (1)
    {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        // Fluxo do programa pausa ate receber uma conexao
        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1)
        {
            logexit("accept");
        }

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        printf("[log] connection from %s\n", caddrstr);

        char buf[BUFSZ];

        // Recebe mensagem mesmo que em pacotes diferentes
        char bufTest[1];
        // Itera enquanto cliente conectado
        while (recv(csock, bufTest, 1, MSG_PEEK) > 0)
        {
            memset(buf, 0, BUFSZ);
            size_t count = 0;
            size_t iteraction = 0;
            do
            {
                count += recv(csock, buf + strlen(buf), BUFSZ - strlen(buf), 0);
                iteraction++;
                if (iteraction > 10000)
                {
                    printf("[log] client timout, disconeting...\n");
                    close(csock);
                    break;
                }
            } while (buf[strlen(buf) - 1] != '\n');

            printf("[log] %s, %ld bytes: %s\n", caddrstr, count, buf);

            // Maximo de mensagens recebidas de uma vez = total buffer / menor mensagem
            struct Msg frmtdMsg[BUFSZ / MINSZ] = {0};
            if (format(buf, BUFSZ, frmtdMsg) == -1)
            {
                //Disconects client with faulty msg
                printf("[log] format error\n");
                close(csock);
                break;
            }

            // Itera por todas as mensagens recebidas de um cliente
            // ate terminar ou encontrar um erro
            unsigned i = 0;
            while (strlen(frmtdMsg[i].comand) != 0)
            {

                char toSend[BUFSZ];
                memset(toSend, 0, BUFSZ);

                if (strcmp("add", frmtdMsg[i].comand) == 0)
                {
                    add(&frmtdMsg[i].coord, data, DATASIZE, toSend);
                }
                else if (strcmp("rm", frmtdMsg[i].comand) == 0)
                {
                    rm(&frmtdMsg[i].coord, data, DATASIZE, toSend);
                }
                else if (strcmp("list", frmtdMsg[i].comand) == 0)
                {
                    list(data, DATASIZE, toSend);
                }
                else if (strcmp("query", frmtdMsg[i].comand) == 0)
                {
                    query(&frmtdMsg[i].coord, data, DATASIZE, toSend);
                }
                else if (strcmp("kill", frmtdMsg[i].comand) == 0)
                {
                    printf("[log] shutting down...\n");
                    close(csock);
                    exit(EXIT_SUCCESS);
                }
                else
                {
                    printf("[log] invalid client comand\n[log] Disconecting...\n");
                }

                count = send(csock, toSend, strlen(toSend), 0);
                printf("[log] sent %ld bytes\n", count);
                printf("[log] sent:\'%s\'\n", toSend);
                if (count != strlen(toSend))
                {
                    logexit("send:");
                }
                i++;
            }
        }
        close(csock);
    }
    exit(EXIT_SUCCESS);
}
