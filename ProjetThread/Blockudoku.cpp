#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "GrilleSDL.h"
#include "Ressources.h"

// Dimensions de la grille de jeu
#define NB_LIGNES   12
#define NB_COLONNES 19

// Nombre de cases maximum par piece
#define NB_CASES    4

// Macros utilisees dans le tableau tab
#define VIDE        0
#define BRIQUE      1
#define DIAMANT     2

int tab[NB_LIGNES][NB_COLONNES]
={ {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

typedef struct
{
  int ligne;
  int colonne;
} CASE;

typedef struct
{
  CASE cases[NB_CASES];
  int  nbCases;
  int  couleur;
} PIECE;

PIECE pieces[12] = { 0,0,0,1,1,0,1,1,4,0,       // carre 4
                     0,0,1,0,2,0,2,1,4,0,       // L 4
                     0,1,1,1,2,0,2,1,4,0,       // J 4
                     0,0,0,1,1,1,1,2,4,0,       // Z 4
                     0,1,0,2,1,0,1,1,4,0,       // S 4
                     0,0,0,1,0,2,1,1,4,0,       // T 4
                     0,0,0,1,0,2,0,3,4,0,       // I 4
                     0,0,0,1,0,2,0,0,3,0,       // I 3
                     0,1,1,0,1,1,0,0,3,0,       // J 3
                     0,0,1,0,1,1,0,0,3,0,       // L 3
                     0,0,0,1,0,0,0,0,2,0,       // I 2
                     0,0,0,0,0,0,0,0,1,0 };     // carre 1

void DessinePiece(PIECE piece);
int  CompareCases(CASE case1,CASE case2);
void TriCases(CASE *vecteur,int indiceDebut,int indiceFin);
void* threadDeFileMessage(void * arg);
void* threadPiece(void* arg);
void* threadEvent(void* arg);
void Attente(int milli);
void setMessage(const char* texte, bool signalOn);
void HandlerSIGALRM(int sig);

char* message = NULL;
int tailleMessage;
int indiceCourant;
PIECE pieceEnCours;
void RotationPiece(PIECE* pPiece);
CASE casesInserees[NB_CASES];  // cases insérées par le joueur 
int  nbCasesInserees;  // nombre de cases actuellement insérées par le joueur. 
pthread_mutex_t mutexCasesInserees = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexMessage = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condCasesInserees = PTHREAD_COND_INITIALIZER;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc,char* argv[])
{
  struct sigaction A;
  A.sa_handler = HandlerSIGALRM;
  A.sa_flags = 0;
  sigemptyset(&A.sa_mask);
  sigaction(SIGALRM, &A, NULL);
  sigset_t mask;
  sigfillset(&mask);
  pthread_sigmask(SIG_SETMASK,&mask, NULL);
  EVENT_GRILLE_SDL event;
  pthread_t threadFM, threadP, threadEvnt;
  setMessage(" Bienvenue dans Blockudoku ", true);
  tailleMessage = strlen(message);
  srand((unsigned)time(NULL));
  // Ouverture de la fenetre graphique
  printf("(MAIN %p) Ouverture de la fenetre graphique\n",pthread_self()); fflush(stdout);
  if (OuvertureFenetreGraphique() < 0)
  {
    printf("Erreur de OuvrirGrilleSDL\n");
    fflush(stdout);
    exit(1);
  }
  printf("(MAIN %p) Creation threadDeFileMessage %p\n", pthread_self(), threadFM);
  pthread_create(&threadFM, NULL, threadDeFileMessage, NULL);
  pthread_create(&threadP, NULL, threadPiece, NULL);
  pthread_create(&threadEvnt, NULL, threadEvent, NULL);
  pthread_mutex_init(&mutexMessage, NULL);
  pthread_mutex_init(&mutexCasesInserees, NULL);
  pthread_cond_init(&condCasesInserees, NULL);
  // Exemples d'utilisation du module Ressources --> a supprimer
  // DessineChiffre(1,15,7);
  // char buffer[40];
  // sprintf(buffer,"coucou");
  // for (int i=0 ; i<strlen(buffer) ; i++) DessineLettre(10,2+i,buffer[i]);
  // DessineBrique(7,3,false);
  // DessineBrique(7,5,true);

  pthread_join(threadEvnt, NULL);

  // Fermeture de la fenetre
  printf("(MAIN %p) Fermeture de la fenetre graphique...",pthread_self()); fflush(stdout);
  FermetureFenetreGraphique();
  printf("OK\n");

  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/////// Fonctions fournies ////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void DessinePiece(PIECE piece)
{
  int Lmin,Lmax,Cmin,Cmax;
  int largeur,hauteur,Lref,Cref;

  Lmin = piece.cases[0].ligne;
  Lmax = piece.cases[0].ligne;
  Cmin = piece.cases[0].colonne;
  Cmax = piece.cases[0].colonne;

  for (int i=1 ; i<=(piece.nbCases-1) ; i++)
  {
    if (piece.cases[i].ligne > Lmax) Lmax = piece.cases[i].ligne;
    if (piece.cases[i].ligne < Lmin) Lmin = piece.cases[i].ligne;
    if (piece.cases[i].colonne > Cmax) Cmax = piece.cases[i].colonne;
    if (piece.cases[i].colonne < Cmin) Cmin = piece.cases[i].colonne;
  }

  largeur = Cmax - Cmin + 1;
  hauteur = Lmax - Lmin + 1;

  switch(largeur)
  {
    case 1 : Cref = 15; break;
    case 2 : Cref = 15; break;
    case 3 : Cref = 14; break;
    case 4 : Cref = 14; break;  
  }

  switch(hauteur)
  {
    case 1 : Lref = 4; break;
    case 2 : Lref = 4; break;
    case 3 : Lref = 3; break;
    case 4 : Lref = 3; break;
  }

  for (int L=3 ; L<=6 ; L++) for (int C=14 ; C<=17 ; C++) EffaceCarre(L,C);
  for (int i=0 ; i<piece.nbCases ; i++) DessineDiamant(Lref + piece.cases[i].ligne,Cref + piece.cases[i].colonne,piece.couleur);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int CompareCases(CASE case1,CASE case2)
{
  if (case1.ligne < case2.ligne) return -1;
  if (case1.ligne > case2.ligne) return +1;
  if (case1.colonne < case2.colonne) return -1;
  if (case1.colonne > case2.colonne) return +1;
  return 0;
}

void TriCases(CASE *vecteur,int indiceDebut,int indiceFin)
{ // trie les cases de vecteur entre les indices indiceDebut et indiceFin compris
  // selon le critere impose par la fonction CompareCases()
  // Exemple : pour trier un vecteur v de 4 cases, il faut appeler TriCases(v,0,3); 
  int  i,iMin;
  CASE tmp;

  if (indiceDebut >= indiceFin) return;

  // Recherche du minimum
  iMin = indiceDebut;
  for (i=indiceDebut ; i<=indiceFin ; i++)
    if (CompareCases(vecteur[i],vecteur[iMin]) < 0) iMin = i;

  // On place le minimum a l'indiceDebut par permutation
  tmp = vecteur[indiceDebut];
  vecteur[indiceDebut] = vecteur[iMin];
  vecteur[iMin] = tmp;

  // Tri du reste du vecteur par recursivite
  TriCases(vecteur,indiceDebut+1,indiceFin); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void* threadDeFileMessage(void * arg)
{
  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask, SIGALRM);
  pthread_sigmask(SIG_SETMASK, &mask, NULL);
  while(1)
  {
    for (int i = 0; i < 17; i++)
    {
      int indice = (indiceCourant + i) % tailleMessage;
      DessineLettre(10, i+1, message[indice]);
    }
    indiceCourant++;
    Attente(400);
    if (indiceCourant >= tailleMessage)
    {
      indiceCourant = 0;
    }

  }
}
void* threadPiece(void* arg)
{
  while(1)
  {
    int indice = rand() % 12;
    pieceEnCours = pieces[indice];
    int c = rand() % 4;
    switch(c)
    {
      case 0:
        pieceEnCours.couleur = JAUNE;
        break;
      case 1:
        pieceEnCours.couleur = ROUGE;
        break;
      case 2:
        pieceEnCours.couleur = VERT;
        break;
      case 3:
        pieceEnCours.couleur = VIOLET;
        break;
    }
    int s = rand() % 4;
    for (int i = 0; i < s; i++)
    {
      RotationPiece(&pieceEnCours);
    }
    DessinePiece(pieceEnCours);

    pthread_mutex_lock(&mutexCasesInserees);
    while(nbCasesInserees < pieceEnCours.nbCases)
    {
      pthread_cond_wait(&condCasesInserees, &mutexCasesInserees);
    }
    pthread_mutex_unlock(&mutexCasesInserees);
    TriCases(casesInserees, 0, nbCasesInserees - 1);
    int Lmin = casesInserees[0].ligne, Cmin = casesInserees[0].colonne;
    for (int i = 1; i < nbCasesInserees; i++)
    {
      if (casesInserees[i].ligne < Lmin)
        Lmin = casesInserees[i].ligne;
      if (casesInserees[i].colonne < Cmin)
        Cmin = casesInserees[i].colonne;
    }
    CASE temp[NB_CASES];
    for (int i = 0; i < nbCasesInserees; i++)
    {
      temp[i].ligne = casesInserees[i].ligne;
      temp[i].colonne = casesInserees[i].colonne;
    }
    for (int i = 0; i < nbCasesInserees; i++)
    {
      casesInserees[i].ligne -= Lmin;
      casesInserees[i].colonne -= Cmin;
    }
    bool correct = true;
    for (int i = 0; i < nbCasesInserees; i++)
    {
      if(CompareCases(casesInserees[i], pieceEnCours.cases[i]) != 0)
      {
        correct = false;
        break;
      }
    }
    if (correct)
    {
      for (int i = 0; i < nbCasesInserees; i++)
      {
        int L = temp[i].ligne;
        int C = temp[i].colonne;

        tab[L][C] = BRIQUE;
        DessineBrique(L, C, false);
      }
    }
    else
    {
     for (int i = 0; i < nbCasesInserees; i++)
      {
        int L = temp[i].ligne;
        int C = temp[i].colonne;

        tab[L][C] = VIDE;
        EffaceCarre(L, C);
      } 
    }
    nbCasesInserees = 0;
  }
}
void* threadEvent(void* arg)
{
  EVENT_GRILLE_SDL event;
  bool ok = false;
  int ligne, colonne;
  while(!ok)
  {
    event = ReadEvent();
    if (event.type == CROIX) ok = true;
    if (event.type == CLIC_GAUCHE)
    {
      pthread_mutex_lock(&mutexCasesInserees);
      if (event.ligne >= 0 && event.ligne < 9 && event.colonne >= 0 && event.colonne < 9)
      {
        if (tab[event.ligne][event.colonne] == VIDE)
        {
          DessineDiamant(event.ligne,event.colonne,pieceEnCours.couleur);
          tab[event.ligne][event.colonne] = DIAMANT;
          casesInserees[nbCasesInserees].ligne = event.ligne;
          casesInserees[nbCasesInserees].colonne = event.colonne;
          nbCasesInserees++;
          pthread_cond_signal(&condCasesInserees);
        }
      }
      pthread_mutex_unlock(&mutexCasesInserees);
    }
    if (event.type == CLIC_DROIT)
    {
      for (int i = 0; i < nbCasesInserees; i++)
      {
        EffaceCarre(casesInserees[i].ligne, casesInserees[i].colonne);
        ligne = casesInserees[i].ligne;
        colonne = casesInserees[i].colonne;
        tab[ligne][colonne] = VIDE;
      }
      nbCasesInserees = 0;

    }
  }

  pthread_exit(NULL);
}
void Attente(int milli)
{
  struct timespec ts;
  ts.tv_sec = milli / 1000;
  ts.tv_nsec = (milli % 1000) * 1000000;
  nanosleep(&ts, NULL);
}
void setMessage(const char* texte, bool signalOn)
{
  pthread_mutex_lock(&mutexMessage);
  alarm(0);
  if (message != NULL)
    free(message);
  message = (char*) malloc(strlen(texte) + 1);
  strcpy(message, texte);
  tailleMessage = strlen(message);
  indiceCourant = 0;
  if(signalOn)
  {
    alarm(10);
  }
  pthread_mutex_unlock(&mutexMessage);

}
void HandlerSIGALRM(int sig)
{
  setMessage(" Jeu en cours ", false);
}
void RotationPiece(PIECE* pPiece)
{
  int Lmin, Cmin;
  for(int i = 0; i < pPiece->nbCases; i++)
  {
    int l = pPiece->cases[i].ligne;
    pPiece->cases[i].ligne = -pPiece->cases[i].colonne;
    pPiece->cases[i].colonne = l;
  }
  Lmin = pPiece->cases[0].ligne;
  Cmin = pPiece->cases[0].colonne;
  for (int i = 0; i < pPiece->nbCases; i++)
  {
    if(pPiece->cases[i].ligne < Lmin)
      Lmin = pPiece->cases[i].ligne;
    if(pPiece->cases[i].colonne < Cmin)
      Cmin = pPiece->cases[i].colonne;
  }

  for (int i = 0; i < pPiece->nbCases; i++)
  {
    pPiece->cases[i].ligne -= Lmin;
    pPiece->cases[i].colonne -= Cmin;
  }

  TriCases(pPiece->cases,0,pPiece->nbCases - 1);
}