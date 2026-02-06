#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>
#define trace(message) printf("THREAD[%d, %p]: %s\n", getpid(), pthread_self(), message)
#define resultat(res) printf("THREAD[%d, %p]: Nombre d'occurence: %d\n", getpid(), pthread_self(), res)
void* Fct1(void* arg);
void* Fct2(void* arg);
void* Fct3(void* arg);
void* Fct4(void* arg);
typedef struct
{
	char fich[20];
	char lire[20];
	int tab;
}FICHIER;
int main()
{
	FICHIER fs;
	strcpy(fs.fich, "file1.txt");
	strcpy(fs.lire, "printf");
	fs.tab = 2;
	FICHIER fs2;
	strcpy(fs2.fich, "file3.txt");
	strcpy(fs2.lire, "cout");
	fs2.tab = 3;
	FICHIER fs3;
	strcpy(fs3.fich, "file2.txt");
	strcpy(fs3.lire, "Bonjour");
	fs3.tab = 1;
	FICHIER fs4;
	strcpy(fs4.fich, "file4.txt");
	strcpy(fs4.lire, "cout");
	fs4.tab = 4;
	pthread_t th1,th2,th3,th4;
	trace("Thread pricipal commence");
	pthread_create(&th1, NULL, Fct1, &fs);
	pthread_create(&th2, NULL, Fct1, &fs2);
	pthread_create(&th3, NULL, Fct1, &fs3);
	pthread_create(&th4, NULL, Fct1, &fs4);
	// pthread_create(&th1, NULL, Fct1, NULL);
	// pthread_create(&th2, NULL, Fct2, NULL);
	// pthread_create(&th3, NULL, Fct3, NULL);
	// pthread_create(&th4, NULL, Fct4, NULL);
	int* res1,*res2,*res3,*res4;
	pthread_join(th1, (void**)&res1);
	pthread_join(th2, (void**)&res2);
	pthread_join(th3, (void**)&res3);
	pthread_join(th4, (void**)&res4);
	resultat(*res1);
	resultat(*res2);
	resultat(*res3);
	resultat(*res4);
	free(res1);
	free(res2);
	free(res3);
	free(res4);
	pthread_exit(NULL);
}
void* Fct1(void* arg)
{
	FICHIER* f = (FICHIER *)arg;
	trace("Thread secondaire 1 commence");
	int file;
	const char* fn = "file1.txt";
	char exemple[20] = "printf";
	char lect[21];
	int len = strlen(f->lire);
	int i = 0;
	int count = 0;
	int n;
	while(1)
	{
		file = open(f->fich, O_RDONLY);
		if (file == -1)
		{
	    perror("open");
	    pthread_exit(NULL);
		}
		for(int k = 0; k <= f->tab; k++)
			printf("\t");
		printf("*\n");
		fflush(stdout);
		if(lseek(file, i, SEEK_SET) == -1)
		{
			close(file);
			break;
		}
		n = read(file, lect, len);
		close(file);

		if(n < len)
			break;
		lect[len] = '\0';
		if(strcmp(lect, f->lire) == 0)
			count++;
		i++;
		usleep(100000);

	}
	int* res = (int*)malloc(sizeof(int));
	*res = count;
	trace("Thread secondaire 1 fini");
	pthread_exit(res);
}
void* Fct2(void* arg)
{
	trace("Thread secondaire 2 commence");
	int file;
	const char* fn = "file2.txt";
	char exemple[20] = "printf";
	char lect[21];
	int len = strlen(exemple);
	int i = 0;
	int count = 0;
	int n;
	while(1)
	{
		file = open(fn, O_RDONLY);
		if (file == -1)
		{
	    perror("open");
	    pthread_exit(NULL);
		}
		printf("\t*\n");
		fflush(stdout);
		if(lseek(file, i, SEEK_SET) == -1)
		{
			close(file);
			break;
		}
		n = read(file, lect, len);
		close(file);

		if(n < len)
			break;
		lect[len] = '\0';
		if(strcmp(lect, exemple) == 0)
			count++;
		i++;
		usleep(100000);

	}
	int* res = (int*)malloc(sizeof(int));
	*res = count;
	trace("Thread secondaire 2 fini");
	pthread_exit(res);
}
void* Fct3(void* arg)
{
	trace("Thread secondaire 3 commence");
	int file;
	const char* fn = "file3.txt";
	char exemple[20] = "printf";
	char lect[21];
	int len = strlen(exemple);
	int i = 0;
	int count = 0;
	int n;
	while(1)
	{
		file = open(fn, O_RDONLY);
		if (file == -1)
		{
	    perror("open");
	    pthread_exit(NULL);
		}
		printf("\t\t*\n");
		fflush(stdout);
		if(lseek(file, i, SEEK_SET) == -1)
		{
			close(file);
			break;
		}
		n = read(file, lect, len);
		close(file);

		if(n < len)
			break;
		lect[len] = '\0';
		if(strcmp(lect, exemple) == 0)
			count++;
		i++;
		usleep(100000);

	}
	int* res = (int*)malloc(sizeof(int));
	*res = count;
	trace("Thread secondaire 3 fini");
	pthread_exit(res);
}
void* Fct4(void* arg)
{
	trace("Thread secondaire 4 commence");
	int file;
	const char* fn = "file4.txt";
	char exemple[20] = "printf";
	char lect[21];
	int len = strlen(exemple);
	int i = 0;
	int count = 0;
	int n;
	while(1)
	{
		file = open(fn, O_RDONLY);
		if (file == -1)
		{
	    perror("open");
	    pthread_exit(NULL);
		}
		printf("\t\t\t*\n");
		fflush(stdout);
		if(lseek(file, i, SEEK_SET) == -1)
		{
			close(file);
			break;
		}
		n = read(file, lect, len);
		close(file);

		if(n < len)
			break;
		lect[len] = '\0';
		if(strcmp(lect, exemple) == 0)
			count++;
		i++;
		usleep(100000);

	}
	int* res = (int*)malloc(sizeof(int));
	*res = count;
	trace("Thread secondaire 4 fini");
	pthread_exit(res);
}