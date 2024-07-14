#ifndef a1_h
#define a1_h

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include <sys/stat.h> //no lseek 

#define BUFFER_SIZE 1024 // Buffer size for reading from files
#define MAX_NUM_FILES 100 // Max number of files 
#define NUM_ALPHABETS 26 // Alphabets 

// Global variables
int communicationPipes[MAX_NUM_FILES][2]; // Pipes for communication between parent and child processes
int childPIDs[MAX_NUM_FILES]; // Array to store child process IDs
int numChildProcesses = 0; // Number of child processes created
int numTerminatedChildProcesses = 0; // Number of child processes terminated

//Function Prototypes 
int *histogramCalculator(char *fileName, int fileSize);
void sigchld_handler(int sig);
void sigint_handler(int sig);


#endif 