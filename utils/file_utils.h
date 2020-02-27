#ifndef _FILE_UTILS_
#define _FILE_UTILS_

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

int createSim(char* decor, int sim_d, unsigned char* buff, unsigned char* titre);
int openFile(char* path);
int openFileSim(char* simFile, unsigned char *buff, unsigned char* titre);
void readMap(int fd, unsigned char* buff, unsigned char* titre);
char* getFileExt(const char* path);
char* getFileBase (const char* path);
void insertElement(int fd, int x, int y, unsigned char element);
void constructeurFile(int fd);
void writeTitle(int fd, unsigned char c);
void writePosVoiture(int fd, int n, unsigned char y, unsigned char x);
unsigned int readPosVoitureX(int fd, int n);
unsigned int readPosVoitureY(int fd, int n);
#endif