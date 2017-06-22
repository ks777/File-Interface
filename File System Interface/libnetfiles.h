#ifndef LIBNETFILES_H
#define LIBNETFILES_H

int netopen(const char *, int);
ssize_t netread(int, void *, size_t);
ssize_t netwrite(int, const void *, size_t);
int netclose(int);
int netserverinit(char* );

const char *PORTS = "20000";

struct steak
{
	int fixing;
	unsigned int catagory[4];
	char attributes[4];
};

#endif
