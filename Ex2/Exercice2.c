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
#define resultat(message,res) printf("THREAD[%d, %p]: %s %d\n", getpid(), pthread_self(),message, res)
void* Fct1(void* arg);
void* Fct2(void* arg);
void Fct2Fin(void* arg);
void HandlerSIGINT(int sig);
void HandlerSIGUSR1(int sig);
typedef struct
{
	char fich[20];
	char lire[20];
	int tab;
}FICHIER;
pthread_t th1, th2, th3, th4, master;
int loop = 1;
int main()
{
	trace("Thread principal commence");
	struct sigaction A;
	A.sa_handler = HandlerSIGINT;
	A.sa_flags = 0;
	sigemptyset(&A.sa_mask);
	sigaction(SIGINT, &A, NULL);
	A.sa_handler = HandlerSIGUSR1;
	sigaction(SIGUSR1, &A, NULL);
	trace("Armement signal SIGINT");
	trace("Creation du thread master");
	pthread_create(&master, NULL, Fct2, NULL);
	trace("Creation Thread secondaire 1");
	pthread_create(&th1, NULL, Fct1, NULL);
	trace("Creation Thread secondaire 2");
	pthread_create(&th2, NULL, Fct1, NULL);
	trace("Creation Thread secondaire 3");
	pthread_create(&th3, NULL, Fct1, NULL);
	trace("Creation Thread secondaire 4");
	pthread_create(&th4, NULL, Fct1, NULL);

	trace("Attente des threads slaves");
	sigset_t mask;
	sigfillset(&mask);
	pthread_sigmask(SIG_SETMASK, &mask, NULL);
	// pause // etape 1
	pthread_join(th1, NULL);
	pthread_join(th2, NULL);
	pthread_join(th3, NULL);
	pthread_join(th4, NULL);
	trace("Demande d'annulation au master");
	pthread_cancel(master);
	trace("Je me termine (thread principal)");
	pthread_exit(NULL);
}
void* Fct1(void* arg)
{
	int i = 1000;
	if (pthread_self() == th1)
		i = 1;
	else if (pthread_self() == th2)
		i = 2;
	else if (pthread_self() == th3)
		i = 3;
	else 
		i = 4;
	resultat("Thread secondaire", i);
	sigset_t mask;
	sigfillset(&mask);
	sigdelset(&mask, SIGUSR1);
	pthread_sigmask(SIG_SETMASK, &mask, NULL);
	trace("Je masque tout sauf SIGUSR1"); // etape 3
	trace("Attente d'un signal..");
	pause();
	resultat("Je me termine (Thread secondaire):", i);
	pthread_exit(NULL);
}
void* Fct2(void* s) // etape 2
{
	trace("Thread master commence");
	trace("Attente d'un signal..");
	pthread_cleanup_push(Fct2Fin, (void*) "Thread Master"); // etape 4
	sigset_t mask;
	sigfillset(&mask);
	sigdelset(&mask, SIGINT);
	pthread_sigmask(SIG_SETMASK, &mask, NULL);
	trace("Je masque tout sauf SIGINT"); // etape 3
	while(1)
	{
		pause();
		trace("Thread Master recoit le signal SIGINT");
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); // etape 4
		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	}
	pthread_cleanup_pop(1);
}
void HandlerSIGINT(int s) // etape 1
{
	loop = 0;
	trace("J'ai recu le signal SIGINT");
	kill(getpid(), SIGUSR1);
}
void HandlerSIGUSR1(int s) // etape 3
{
	trace("J'ai recu le signal SIGUSR1");
}
void Fct2Fin(void* s) // etape 4
{
	const char* p = (const char*) s;
	char txt[80];
	sprintf(txt, "%s 	: Je passe par ma fonction de terminaison\n", p);
	trace(txt);
}