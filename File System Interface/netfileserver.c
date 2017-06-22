#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "libnetfiles.h"

int threadMain(struct core *);
void* netopenex(void *);
void* netreadex(void *);
void* netwriteex(void *);
void* netcloseex(void *);
int nortell(struct core * ,void * const, long unsigned int);
int delivery(struct core * ,void * const, long unsigned int);
void connectcon(struct core *);
void incorrect(char*, int, char *);
pthread_mutex_t allofit;

static int allofit[100];

struct core
{
	  	int dirr;
};

int nortell(struct core * mas, void *const buff, long unsigned int buffsize)
{
	int FBytes;
	size_t real = buffsize;
	void * tempbuff = buff;
	do
	{
 		FBytes = recv(mas->dirr, tempbuff, buffsize, 0);
		buffsize -= FBytes;
		tempbuff += FBytes;
	}
	while(buffsize > 0 && FBytes > 0);

	if (FBytes < 0)
	{
		return -1;
	}
	else if (FBytes == 0)
	{
		return 0;
	}
	else
	{
		return real - buffsize;
	}
}


int delivery(struct core * mas, void *const buff, long unsigned int buffsize)
{
	int SBytes;
	size_t real = buffsize;
	void * tempbuff = buff;
	do
	{
		SBytes = send(mas->dirr, tempbuff, buffsize, 0);
		buffsize -= SBytes;
		tempbuff += SBytes;
	}
	while(buffsize > 0 && SBytes > 0);

	if (SBytes < 0)
	{
		return -1;
	}
	else if (SBytes == 0)
	{
		return 0;
	}
	else
	{
		return real - buffsize;
	}
}

void incorrect(char* name, int section, char *message)
{
  printf("%s,%d, errno: %s \n", name, section, strerror(errno));
  exit(1);
}

void connectcon(struct core *mas)
{
	unsigned long int FBytes;
	char buff[24] = "";
	sprintf(buff, "Client Server #%d is connected!\n",1);

	if (0 > delivery(mas, buff, sizeof(buff)))
	{
		incorrect(__FILE__,__LINE__,"Problem during sending.");
	}

	FBytes = nortell(mas, buff, sizeof(buff));

	if(0 > FBytes)
	{
		incorrect(__FILE__,__LINE__,"Problems with receive.");
	}
	else if (0 == FBytes)
	{
		printf("Exiting.\n");
		shutdown(mas->dirr, 2);
		pthread_exit(NULL);
	}
	printf("%s\n",buff);
}

void * netopenex(void * buff)
{
	int catagory[4];
	struct steak intdata = {0};
	unsigned int flags;
	memcpy(&intdata, buff, sizeof(intdata));
	catagory[0] = intdata.catagory[0];
	catagory[1] = intdata.catagory[1];
	catagory[2] = intdata.catagory[2];

	size_t datacookie = catagory[1] - catagory[0];
	char * filedes = calloc(datacookie, sizeof(char));
	memcpy(filedes, buff + intdata.catagory[0], datacookie);
	filedes[datacookie - 1] = '\0';
	memcpy(&flags, buff + catagory[1], sizeof(int));
	free(buff);

	printf("Calling netopen for flags: %d\n", flags);
	printf("Calling netopen for filedes: %s\n", filedes);
	errno = 0;
	int xyz = open(filedes, flags);
	memcpy(&intdata.fixing, &errno, sizeof(int));
	printf("%d\n",intdata.fixing);
	printf("Calling netopen for xyz/fd: %d\n", xyz);

	memset(intdata.attributes, 0, sizeof(intdata.attributes));
	intdata.catagory[0] = sizeof(intdata);
	intdata.catagory[1] = intdata.catagory[0] + sizeof(int);
	buff = calloc(intdata.catagory[1], sizeof(char));
	memcpy(buff, &intdata, sizeof(intdata));
	memcpy(buff + intdata.catagory[0], &xyz, sizeof(int));

	free(filedes);
	return buff;
}

void * netreadex(void * buff)
{
	int xyz;
	struct steak intdata = {0};
	size_t ByteNum;

	memcpy(&intdata, buff, sizeof(intdata));
	memcpy(&xyz, buff + intdata.catagory[0], sizeof(int));
	memcpy(&ByteNum, buff + intdata.catagory[1], sizeof(size_t));
	free(buff);
	char *RetributionBuff = calloc(ByteNum, sizeof(char));
	printf("Calling netread on ByteNum: %zd\n", ByteNum);
	printf("Calling netread on xyz/fd: %d\n", xyz);

	errno = 0;
	ssize_t RBytes = read(xyz, RetributionBuff, ByteNum);
	intdata.fixing = errno;
	memset(intdata.attributes, 0, sizeof(intdata.attributes));
	intdata.catagory[0] = sizeof(intdata);
	intdata.catagory[1] = intdata.catagory[0] + sizeof(ssize_t);

	if(intdata.fixing)
	{
		intdata.catagory[2] = intdata.catagory[1];
		free(RetributionBuff);
		buff = calloc(intdata.catagory[2], sizeof(char));
		memcpy(buff, &intdata, sizeof(intdata));
		memcpy(buff + intdata.catagory[0], &RBytes, sizeof(ssize_t));
		return buff;
	}
	else
	{
		intdata.catagory[2] = intdata.catagory[1] + RBytes;
	}
	buff = calloc(intdata.catagory[2], sizeof(char));
	memcpy(buff, &intdata, sizeof(intdata));
	memcpy(buff + intdata.catagory[0], &RBytes, sizeof(ssize_t));
	memcpy(buff + intdata.catagory[1], RetributionBuff, (size_t)RBytes);
	free(RetributionBuff);
	return buff;
}

void * netwriteex(void * buff)
{
	int xyz;
	struct steak intdata = {0};
	memcpy(&intdata, buff, sizeof(intdata));
	memcpy(&xyz, buff + intdata.catagory[0], sizeof(int));
	size_t ByteNum;
	memcpy(&ByteNum, buff + intdata.catagory[1], sizeof(size_t));
	char * tempbuff = calloc(ByteNum, sizeof(char));
	memcpy(tempbuff, buff + intdata.catagory[2], sizeof(char)*ByteNum);
	free(buff);
	printf("Calling netwrite on ByteNum: %zd\n", ByteNum);
	printf("Calling netwrite on xyz/fd: %d\n", xyz);

	errno = 0;
	ssize_t WBytes = write(xyz, tempbuff, ByteNum);

	free(tempbuff);
	intdata.fixing = errno;
	memset(intdata.attributes, 0, sizeof(intdata.attributes));
	intdata.catagory[0] = sizeof(intdata);
	intdata.catagory[1] = intdata.catagory[0] + sizeof(ssize_t);
	buff = calloc(intdata.catagory[1], sizeof(char));
	memcpy(buff, &intdata, sizeof(intdata));
	memcpy(buff + intdata.catagory[0], &WBytes, sizeof(ssize_t));
	return buff;
}

void * netcloseex(void * buff)
{
	int xyz;
	struct steak intdata = {0};
	memcpy(&intdata, buff, sizeof(intdata));
	memcpy(&xyz, buff + intdata.catagory[0], sizeof(int));

	printf("Calling netclose on xyz/fd: %d\n", xyz);

	errno = 0;
	int Retribution = close(xyz);

	intdata.fixing = errno;
	memset(intdata.attributes, 0, sizeof(intdata.attributes));
	intdata.catagory[0]= sizeof(intdata);
	intdata.catagory[1] = intdata.catagory[0] + sizeof(int);
	free(buff);
	buff = calloc(intdata.catagory[1], sizeof(char));
	memcpy(buff, &intdata, sizeof(intdata));
	memcpy(buff + intdata.catagory[0], &Retribution, sizeof(int));
	return buff;
}

//ThreadMain: attempts to send confirmation back to the client server, as well
//as waiting for its input.
int threadMain(struct core * mas)
{
	char * buff;
	unsigned long int FBytes;
	connectcon(mas);
	struct steak intdata;
	while(1)
	{
		memset(&intdata, 0, sizeof(intdata));
		FBytes = nortell(mas, &intdata, sizeof(intdata));

		if(0 > FBytes)
		{
			incorrect(__FILE__,__LINE__,"Problem with receive.");
		}
		else if (0 == FBytes)
		{
			printf("Exiting.\n");
			shutdown(mas->dirr, 2);
			pthread_exit(NULL);
		}

		if(0 == strncmp(intdata.attributes, "op", 2))
		{
			buff = calloc(intdata.catagory[2], sizeof(char));
			FBytes = nortell(mas, buff, intdata.catagory[2]);

			if(0 > FBytes)
			{
				free(buff);
				incorrect(__FILE__,__LINE__,"Problem with receive.");
			}
			else if (0 == FBytes)
			{
				printf("Exiting.\n");
				free(buff);
				shutdown(mas->dirr, 2);
				pthread_exit(NULL);
			}

			buff = netopenex(buff);
			memcpy(&intdata, buff, sizeof(intdata));

			if (0 > delivery(mas, buff, intdata.catagory[1]))
			{
				free(buff);
				incorrect(__FILE__,__LINE__,"Problem with sending.");
			}
			free(buff);
		}
		else if (0 == strncmp(intdata.attributes, "rd", 2))
		{
			buff = calloc(intdata.catagory[2], sizeof(char));
			FBytes =nortell(mas, buff, intdata.catagory[2]);

			if(0 > FBytes)
			{
				free(buff);
				incorrect(__FILE__,__LINE__,"Problem with receive.");
			}
			else if (0 == FBytes)
			{
				printf("Exiting.\n");
				free(buff);
				shutdown(mas->dirr, 2);
				pthread_exit(NULL);
			}

			buff = netreadex(buff);
			memcpy(&intdata, buff, sizeof(intdata));
			//Checking for errors.
			if (0 > delivery(mas, &intdata, sizeof(intdata)))
			{
				free(buff);
				incorrect(__FILE__,__LINE__,"Problem with sending.");
			}

			//BUFFER
			if(intdata.fixing)
			{
				free(buff);
			}
			else
			{
				if (0 > delivery(mas, buff, intdata.catagory[2]))
				{
					free(buff);
					incorrect(__FILE__,__LINE__,"Problem with sending.");
				}
				free(buff);
			}
		}
		else if (0 == strncmp(intdata.attributes, "wr", 2))
		{

			buff = calloc(intdata.catagory[3], sizeof(char));
			FBytes = nortell(mas, buff, intdata.catagory[3]);

			if(0 > FBytes)
			{
				free(buff);
				incorrect(__FILE__,__LINE__,"Problem with receive.");
			}
			else if (0 == FBytes)
			{
				printf("Exiting.\n");
				free(buff);
				shutdown(mas->dirr, 2);
				pthread_exit(NULL);
			}

			buff = netwriteex(buff);
			memcpy(&intdata, buff, sizeof(intdata));

			if (0 > delivery(mas, buff, intdata.catagory[1]))
			{
				free(buff);
				incorrect(__FILE__,__LINE__,"Problem with sending.");
			}
				free(buff);
		}
		else if (0 == strncmp(intdata.attributes, "cl", 2))
		{
			buff = calloc(intdata.catagory[1], sizeof(char));
			FBytes = nortell(mas, buff, intdata.catagory[1]);

			if(0 > FBytes)
			{
				free(buff);
				incorrect(__FILE__,__LINE__,"Problem with receive.");
			}
			else if (0 == FBytes)
			{
				printf("Exiting.\n");
				free(buff);
				shutdown(mas->dirr, 2);
				pthread_exit(NULL);
			}
			buff = netcloseex(buff);
			memcpy(&intdata, buff, sizeof(intdata));
			if (0 > delivery(mas, buff, intdata.catagory[1]))
			{
				free(buff);
				incorrect(__FILE__,__LINE__,"Problem with sending.");
			}
			free(buff);
		}
		else
		{
			printf("You provided a packet with no attributes!\n");
		}
	}
	shutdown(mas->dirr, 2);
	pthread_exit(NULL);
	return 0;
}

int main(int argc,char **argv)
{
	//Time to open up a connection!
	int x = 0;
  	pthread_t tib;
	int hearconn;
	int dirr;
	struct sockaddr_in serverside;
	struct sockaddr_in clientside;
	socklen_t cal;

	//listening for a connection
	hearconn = socket(AF_INET, SOCK_STREAM, 0);
	if (hearconn < 0)
	{
		incorrect(__FILE__,__LINE__,"Problem opening a socket connection.");
	}

	bzero((char *) &serverside, sizeof(serverside));

	//Internet connections time!!!
	serverside.sin_family = AF_INET;
	serverside.sin_port = htons(atoi(PORTS));
	serverside.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(hearconn, (struct sockaddr *) &serverside, sizeof(serverside)) < 0)
	{
  		incorrect(__FILE__,__LINE__,"Problem during binding procedure.");
	}

  	listen(hearconn, 200000);
  	cal = sizeof(clientside);

  	while (1)
  	{
  		++x;
  		struct core center;
  		dirr = accept(hearconn, (struct sockaddr *) &clientside, &cal);

		if (dirr < 0)
		{
	  		incorrect(__FILE__,__LINE__,"Problem on waiting for a connection.");
		}

	  	center.dirr = dirr;

  		if (pthread_create(&tib, NULL,(void*) &threadMain, &center))
  		{
  			fprintf(stderr, "error");
  			incorrect(__FILE__,__LINE__,"Problem on running pthreads.");
  		}
  	}

	shutdown(hearconn, 2);
	printf("Server commencing shutdown.\n");
	return 0;
}
