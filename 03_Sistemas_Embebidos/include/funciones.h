#include <ulfius.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <errno.h>

#define HORA_TAM 24
#define SERVICE 64
#define LOG 128
#define LOG_USERS 1
#define LOG_GOES 2
#define TAM 1024

void print_log(int, char*);
void get_time(char*);

