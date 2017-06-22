#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "libnetfiles.h"

int NewSocket(struct sockaddr *);
int nortell(int, void * const, long unsigned int);
int delivery(int, void * const, long unsigned int);
int incorrect(char *, int, char *);

static int InitialSock;
int xyzarr[100];
static int xyzmarker = 0;
struct sockaddr *NewServe;
static char * clienter;


//Incorrect: aids in displaying errors in the functions.
int incorrect(char* name, int section, char *message)
{
  printf("%s,%d, errno: %s \n", name, section, strerror(errno));
  return -1;

}

int nortell(int InitialSock, void *const buff, long unsigned int buffsize)
{
	int FBytes;
	size_t real = buffsize;
	void * tempbuff = buff;
	do
	{
		FBytes = recv(InitialSock, tempbuff, buffsize, 0);
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

int delivery(int InitialSock, void *const buff, long unsigned int buffsize)
{
	int DBytes;
	size_t real = buffsize;
	void * tempbuff = buff;
	do
	{
		DBytes = send(InitialSock, tempbuff, buffsize, 0);
		buffsize -= DBytes;
		tempbuff += DBytes;
	}
	while(buffsize > 0 && DBytes > 0);

	if (DBytes < 0)
	{
		return -1;
	}
	else if (DBytes == 0)
	{
		return 0;
	}
	else
	{
		return real - buffsize;
	}
}

//NetOpen: sends intdata from client to the server
int netopen(const char *pathname, int flags)
{
	struct steak intdata = {0};
	long unsigned int FBytes;
	int xyz; //represented fd, but xyz sounds cooler.

	intdata.fixing = 0;
	memcpy(&intdata.attributes, "op", sizeof("op"));
	intdata.catagory[0] = sizeof(intdata);
	intdata.catagory[1] = intdata.catagory[0] + sizeof(char)*(strlen(pathname) + 1);
	intdata.catagory[2] = intdata.catagory[1] + sizeof(int);

	size_t buffsize = intdata.catagory[2];
	char * buff = calloc(buffsize, sizeof(char));
	memcpy(buff, &intdata, sizeof(intdata));
	memcpy(buff + intdata.catagory[0], pathname, sizeof(char)*(strlen(pathname) + 1));
	memcpy(buff + intdata.catagory[1], &flags, sizeof(int));

	if (0 > delivery(InitialSock, buff, intdata.catagory[2]))
	{
		free(buff);
		return incorrect(__FILE__,__LINE__,"Problem with sending process.");
	}
	if (0 > delivery(InitialSock, &intdata, sizeof(intdata)))
	{
		free(buff);
		return incorrect(__FILE__,__LINE__,"Problem with sending process.");
	}

	free(buff);
	buffsize = sizeof(intdata) + sizeof(int);
	buff = calloc(buffsize, sizeof(char));
	FBytes = nortell(InitialSock, buff, buffsize);

	if(0 > FBytes)
	{
		free(buff);
		return incorrect(__FILE__,__LINE__,"Problem during receive.");
	}
	else if (0 == FBytes)
	{
		printf("Server Closed\n");
		free(buff);
		shutdown(InitialSock, 2);
		exit(0);
	}

	memcpy(&intdata, buff, sizeof(intdata));
	printf("%d\n", intdata.fixing);

	if (intdata.fixing)
	{
		errno = intdata.fixing;
		free(buff);
		return incorrect(__FILE__,__LINE__,"Problem during receive.");
	}

	memcpy(&xyz, buff + intdata.catagory[0], sizeof(int));
	xyzarr[xyzmarker] = xyz;
	xyzmarker++;
	free(buff);
    return xyz;
}

//NetRead: This function connects with the host, clears the buffer and
//sends the read functionality to the server.
ssize_t netread(int fd, void *buf, size_t nbytes)
{
	long unsigned int FBytes;
 	struct steak intdata = {0};
	intdata.fixing = 0;
	ssize_t RBytes;
	memcpy(&intdata.attributes, "rd", sizeof("rd"));
	intdata.catagory[0] = sizeof(intdata);
	intdata.catagory[1] = intdata.catagory[0] + sizeof(int);
	intdata.catagory[2] = intdata.catagory[1] + sizeof(size_t);

 	if (0 > delivery(InitialSock, &intdata, sizeof(intdata)))
 	{
		return incorrect(__FILE__,__LINE__,"Problem during sending process.");
	}

	char * buff = calloc(intdata.catagory[2], sizeof(char));
	memcpy(buff, &intdata, sizeof(intdata));
	memcpy(buff + intdata.catagory[0], &fd, sizeof(int));
	memcpy(buff + intdata.catagory[1], &nbytes, sizeof(size_t));

	if (0 > delivery(InitialSock, buff, intdata.catagory[2]))
	{
		free(buff);
		return incorrect(__FILE__,__LINE__,"Prlbem during sending process.");
	}
	free(buff);
	memset(&intdata, 0, sizeof(intdata));
	FBytes = nortell(InitialSock, &intdata, sizeof(intdata));

	if(0 > FBytes)
	{
		return incorrect(__FILE__,__LINE__,"Problem during receive.");
	}
	else if (0 == FBytes)
	{
		printf("Server Closed\n");
		shutdown(InitialSock, 2);
		exit(0);
	}
	if (intdata.fixing)
	{
		errno = intdata.fixing;
		return incorrect(__FILE__,__LINE__,"Problem during receive.");
	}
	buff = calloc(intdata.catagory[2], sizeof(char));
	FBytes = nortell(InitialSock, buff, intdata.catagory[2]);
	if(0 > FBytes)
	{
		free(buff);
		return incorrect(__FILE__,__LINE__,"Problem during receive.");
	}
	else if (0 == FBytes)
	{
		printf("Server Closed\n");
		free(buff);
		shutdown(InitialSock, 2);
		exit(0);
	}

	memcpy(&RBytes, buff + intdata.catagory[0], sizeof(ssize_t));
	memcpy(buf, buff + intdata.catagory[1], RBytes);
	free(buff);
    return RBytes;
}

//NetWrite: Connects to host and sends "write" functionality to the server.
ssize_t netwrite(int fd, const void *buf, size_t nbytes)
{
	struct steak intdata = {0};
	long unsigned int FBytes;
	intdata.fixing = 0;
	ssize_t WBytes;
	memcpy(&intdata.attributes, "wr", sizeof("wr"));
	intdata.catagory[0] = sizeof(intdata);
	intdata.catagory[1] = intdata.catagory[0] + sizeof(int);
	intdata.catagory[2] = intdata.catagory[1] + sizeof(size_t);
	intdata.catagory[3] = intdata.catagory[2] + nbytes;

 	if (0>delivery(InitialSock, &intdata, sizeof(intdata)))
 	{
		return incorrect(__FILE__,__LINE__,"Problem during sending process.");
	}
	char * buff = calloc(intdata.catagory[3], sizeof(char));
	memcpy(buff, &intdata, sizeof(intdata));
	memcpy(buff + intdata.catagory[0], &fd, sizeof(int));
	memcpy(buff + intdata.catagory[1], &nbytes, sizeof(size_t));
	memcpy(buff + intdata.catagory[2], buf, nbytes);

	if (0 > delivery(InitialSock, buff,intdata.catagory[3]))
	{
		free(buff);
		return incorrect(__FILE__,__LINE__,"Problem during sending process.");
	}
	free(buff);
	int buffsize = sizeof(intdata) + sizeof(ssize_t);
	buff = calloc(buffsize, sizeof(char));
	FBytes = nortell(InitialSock, buff, buffsize);

	if(0 > FBytes)
	{
		free(buff);
		return incorrect(__FILE__,__LINE__,"Problem during receive.");
	}
	else if (0 == FBytes)
	{
		printf("Server Closed\n");
		free(buff);
		shutdown(InitialSock, 2);
		exit(0);
	}

	memcpy(&intdata, buff, sizeof(intdata));
	if (intdata.fixing)
	{
		errno = intdata.fixing;
		free(buff);
		return incorrect(__FILE__,__LINE__,"Problem during receive.");
	}

	memcpy(&WBytes, buff + sizeof(intdata), sizeof(ssize_t));
	free(buff);
    return WBytes;
}

//NetClose: sends the message of "close[fd]" to the server.
int netclose(int fd)
{

	long unsigned int FBytes;
 	struct steak intdata = {0};
 	intdata.fixing = 0;
 	int fin;
	memcpy(&intdata.attributes, "cl", sizeof("cl"));
	intdata.catagory[0] = sizeof(intdata);
	intdata.catagory[1] = intdata.catagory[0] + sizeof(int);
	size_t buffsize = intdata.catagory[1];
	char * buff = calloc(buffsize, sizeof(char));
	memcpy(buff, &intdata, sizeof(intdata));
	memcpy(buff + intdata.catagory[0], &fd, sizeof(int));

	if (0 > delivery(InitialSock, buff, intdata.catagory[1]))
	{
		free(buff);
		return incorrect(__FILE__,__LINE__,"Problem during sending process.");
	}

 	if (0 > delivery(InitialSock, &intdata, sizeof(intdata)))
 	{
 		free(buff);
		return incorrect(__FILE__,__LINE__,"Problem during sending process.");
	}

	free(buff);
	buffsize = sizeof(intdata) + sizeof(int);
	buff = calloc(buffsize, sizeof(char));
	FBytes = nortell(InitialSock, buff, buffsize);

	if(0 > FBytes)
	{
		free(buff);
		return incorrect(__FILE__,__LINE__,"Problem during receive.");
	}
	else if (0 == FBytes)
	{
		printf("Server Closed\n");
		free(buff);
		shutdown(InitialSock, 2);
		exit(0);
	}

	memcpy(&intdata, buff, sizeof(intdata));

	if (intdata.fixing)
	{
		errno = intdata.fixing;
		free(buff);
		return incorrect(__FILE__,__LINE__,"Problem during receive.");
	}

	memcpy(&fin, buff + intdata.catagory[0], sizeof(int));
	xyzarr[xyzmarker] = 0;
	xyzmarker--;
	free(buff);
    return fin;
}

// NetServerInit: Makes sure the host exists for usage.
// Also NOTE: make sure that library is connected to file connection mode
// to each net file command sent.
int netserverinit(char* hostname)
{
	int messager;
	char *user = NULL;
	char *trans = NULL;
	struct addrinfo w;
	struct addrinfo *x;
	struct addrinfo *y;
	clienter = hostname;

	memset(&w, 0, sizeof(w));
	w.ai_family = AF_INET;
	w.ai_socktype = SOCK_STREAM;
	messager = getaddrinfo(clienter, PORTS, &w, &x);
	if (messager)
	{
		printf("%s\n",gai_strerror(messager));
		freeaddrinfo(x);
		return -1;
	}

	if (x == NULL)
	{
		return -1;
	}

	for(y = x; y != NULL; y = y->ai_next)
	{
		getnameinfo(y->ai_addr, y->ai_addrlen, user,sizeof(user), trans, sizeof(trans), NI_NUMERICHOST);
		NewServe = y->ai_addr;
	}
	InitialSock = NewSocket(NewServe);
	return 0;
}

//NewSocket: Establishing connection with a new socket.
//NOTE: remember to end it with an open connection AND the socket has returned
int NewSocket(struct sockaddr * servering)
{
	char buff[24];
	InitialSock = socket(AF_INET, SOCK_STREAM, 0);
	long unsigned int FBytes = nortell(InitialSock, buff, sizeof(buff));

	if (InitialSock < 0)
	{
		return incorrect(__FILE__,__LINE__,"Problem opening the socket connection.");
	}
	if (0 > connect(InitialSock, (struct sockaddr *) servering,sizeof(*servering)))
	{
  		return incorrect(__FILE__,__LINE__,"Problem connecting to the socket.");
	}
  	if(0 > FBytes)
	{
		return incorrect(__FILE__,__LINE__,"Problem during receive");
	}
	else if (0 == FBytes)
	{
		printf("Server is currently Closed.\n");
		shutdown(InitialSock, 2);
		exit(0);
	}
	printf("%s\n", buff);
	if (0 > delivery(InitialSock, buff,sizeof(buff)))
	{
		return incorrect(__FILE__,__LINE__,"Problem in the sending process.");
	}
	return InitialSock;
}
