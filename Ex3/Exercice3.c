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
#define trace(message) printf("Thread %d.%lu: %s\n", getpid(),(unsigned long) pthread_self(), message)
#define resultat(message,res) printf("Thread %d.%lu: %s %d\n", getpid(),(unsigned long) pthread_self(),message, res)
void* Fct1(void* arg);
void Attente(int milli);
pthread_mutex_t mutexParam;
int compteur = 0;
pthread_mutex_t mutexCompteur;
pthread_cond_t condCompteur;
typedef struct
{
	char nom[20];
	int nbSecondes;
}DONNEE;
DONNEE data[] = {
	"MATAGNE", 15,
	"WILVERS", 10,
	"WAGNER", 17,
	"QUETTIER", 8,
	"", 0 
};
pthread_t th[100];
int loop = 1;
int main()
{
	int i;
	trace("Thread principal commence");
	pthread_mutex_init(&mutexCompteur, NULL);
	pthread_cond_init(&condCompteur, NULL);
	for (i = 0; data[i].nbSecondes != 0; i++)
	{
		pthread_mutex_lock(&mutexCompteur);
		compteur++;
		resultat("Creation Thread secondaire: ", i + 1);
		DONNEE *param = (DONNEE*)malloc(sizeof(DONNEE));
		memcpy(param, &data[i], sizeof(DONNEE));
		pthread_create(&th[i], NULL, Fct1, param);
		pthread_mutex_unlock(&mutexCompteur);
	}
	// trace("Attente des threads slaves"); // etape 1
	// for (int j = 0; j < i; j++)
	// {
	// 	pthread_join(th[j], NULL);
	// }
	trace("J'attend que mon compteur passe a 0"); // etape 3
	pthread_mutex_lock(&mutexCompteur);
	char txt[80];
	sprintf(txt, "Compteur actuel: %d", compteur);
	trace(txt);
	while(compteur != 0)
	{
		pthread_cond_wait(&condCompteur, &mutexCompteur);
		sprintf(txt, "Notification reçue (compteur : %d)", compteur);
		trace(txt);
	}
	pthread_mutex_unlock(&mutexCompteur);
	trace("Je me termine (thread principal)");
	pthread_exit(NULL);
}
void* Fct1(void* arg)
{
	pthread_mutex_lock(&mutexParam);
	trace("se lance.");
	DONNEE *d = (DONNEE *) arg;
	trace(d->nom);
	Attente(d->nbSecondes);
	pthread_mutex_unlock(&mutexParam);
	trace("se termine et notifie la VC");
	pthread_mutex_lock(&mutexCompteur);
	compteur--;
	pthread_cond_signal(&condCompteur);
	pthread_mutex_unlock(&mutexCompteur);
	pthread_exit(NULL);
}
void Attente(int milli)
{
	struct timespec ts;
	ts.tv_sec = milli;
	ts.tv_nsec = 0;
	nanosleep(&ts, NULL);
}
