#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>

#ifndef NULL
#define NULL 0
#endif

#ifndef FALSE
#define FALSE  (0)
#endif

#ifndef TRUE
#define TRUE   (!FALSE)
#endif
char* get_in_addr(struct sockaddr *sa);
