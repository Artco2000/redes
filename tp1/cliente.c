#include "comum.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSZ 500

void usage(int argc, char **argv)
{
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

int parse(char *buf)
{
	// buf only contains letters, numbers or space
	for (size_t i = 0; i < strlen(buf) - 1; i++)
	{
		if ((buf[i] >= 48 && buf[i] <= 57) || ((buf[i] >= 65 && buf[i] <= 90)) || ((buf[i] >= 97 && buf[i] <= 122)) || (buf[i] == 32))
			continue;
		else
			return -1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		usage(argc, argv);
	}

	struct sockaddr_storage storage;
	if (0 != addrparse(argv[1], argv[2], &storage))
	{
		usage(argc, argv);
	}

	int s;
	s = socket(storage.ss_family, SOCK_STREAM, 0);
	if (s == -1)
	{
		logexit("socket");
	}
	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(s, addr, sizeof(storage)))
	{
		logexit("connect");
	}

	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);

	printf("[log] connected to %s\n", addrstr);

	char buf[BUFSZ];
	int flag = 1; 
	while(flag)
	{
		memset(buf, 0, BUFSZ);

		printf("mensagem> ");
		fgets(buf, BUFSZ - 1, stdin);
		// Substitutes \0 for \n
		buf[strlen(buf) - 1] = '\n';

		if (parse(buf) == -1)
		{
			printf("Please type a valid mensage\n");
			continue;
		}

		size_t count = send(s, buf, strlen(buf), 0);
		printf("[log] sent %ld bytes\n", count);
		if (count != strlen(buf))
		{
			logexit("send");
		}

		// Finaliza cliente assim como servidor
		if (strstr(buf, "kill") != NULL)
		{
			close(s);
			break;
		}

		memset(buf, 0, BUFSZ);
		count = 0;
		size_t iteraction = 0;
		do
		{
			count += recv(s, buf + strlen(buf), BUFSZ - strlen(buf), 0);
			iteraction++;
			if (iteraction > 10000)
			{
				printf("[log] server timout, disconeting...\n");
				flag = 0;
				break;
			}
		} while (buf[strlen(buf) - 1] != '\n');

		printf("[log] received %ld bytes\n", count);
		puts(buf);
	}
	printf("[log] client closing...\n");
	close(s);

	exit(EXIT_SUCCESS);
}