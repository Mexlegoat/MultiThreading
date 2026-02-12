#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#define trace(message) printf("THREAD[%d, %p]: %s\n", getpid(), pthread_self(), message)
#define resultat(res) printf("THREAD[%d, %p]: Nombre d'occurence: %d\n", getpid(), pthread_self(), res)
void* Fct1(void* arg);
void* Fct2(void* arg);
void* Fct3(void* arg);
void* Fct4(void* arg);
void HandlerSIGINT(int sig);
typedef struct
{
	char fich[20];
	char lire[20];
	int tab;
}FICHIER;
pthread_t th1, th2, th3, th4;
int main()
{
	trace("Thread principal commence");
	struct sigaction A;
	A.sa_handler = HandlerSIGINT;
	A.sa_flags = 0;
	sigemptyset(&A.sa_mask);
	sigaction(SIGINT, &A, NULL);
	trace("Armement signal SIGINT");
	trace("Creation Thread secondaire 1");
	pthread_create(&th1, NULL, Fct1, NULL);
	trace("Creation Thread secondaire 2");
	pthread_create(&th2, NULL, Fct1, NULL);
	trace("Creation Thread secondaire 3");
	pthread_create(&th3, NULL, Fct1, NULL);
	trace("Creation Thread secondaire 4");
	pthread_create(&th4, NULL, Fct1, NULL);

	trace("Attente");
	pause();
	trace("Je me termine (thread principal)");
	pthread_exit(NULL);
}
void* Fct1(void* arg)
{
	trace("Thread secondaire commence");
	// sigset_t mask;
	// sigfillset(&mask);
	// pthread_sigmask(SIG_SETMASK, &mask, NULL);
	trace("Attente d'un signal..");
	pause();
	trace("Je me termine");
	pthread_exit(NULL);
}
void HandlerSIGINT(int s)
{
	trace("J'ai recu le signal SIGINT");
}