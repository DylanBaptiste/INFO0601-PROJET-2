#ifndef _FILE_UTILS_
#define _FILE_UTILS_

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

int createSim(char* decor, int sim_d, unsigned char* buff);
void writeMap(unsigned char* buffer, int fd);
int openFile(char* path);
int openFileSim(char* simFile, unsigned char *buff);
void readMap(int fd, unsigned char* buff);
char* getFileExt(const char* path);
char* getFileBase (const char* path);
void insertElement(int fd, int x, int y, unsigned char element);
void constructeurFile(int fd);
void writeTitle(int fd, unsigned char c);
#endif