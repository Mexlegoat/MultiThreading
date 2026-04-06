#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "GrilleSDL.h"
#include "Ressources.h"
// tracings:
#define trace(message ,thread) printf("(MAIN %p) Creation du %s %p\n", pthread_self(), message ,thread)
#define tracer(message, thread) printf("(MAIN %p) %s *p\n", pthread_self(), message ,thread)
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
void* threadScore(void* arg);
void* threadCases(void* arg);
void* threadNettoyeur(void* arg);

void FonctionTerminaison(void* arg);

void Attente(int milli);
void setMessage(const char* texte, bool signalOn);
void LiberationCle(void* p);

void HandlerSIGALRM(int sig);
void HandlerSIGUSR1(int sig);

void RotationPiece(PIECE* pPiece);

char* message = NULL;
int tailleMessage;

int indiceCourant;
PIECE pieceEnCours;

CASE casesInserees[NB_CASES];  // cases insérées par le joueur 
int  nbCasesInserees;  // nombre de cases actuellement insérées par le joueur. 

bool MAJScore = false;
bool MAJCombos = false;
int score = 0;
int combos = 0;

int lignesCompletes[NB_CASES];       
int nbLignesCompletes = 0;    
int colonnesCompletes[NB_CASES];    
int nbColonnesCompletes = 0;  
int carresComplets[NB_CASES];
int nbCarresComplets;
int nbAnalyses = 0;

bool traitementEnCours = false;

pthread_t threadFM, threadP, threadEvnt, threadScre, threadNtyr;
pthread_t tabThreadCase[9][9];

pthread_mutex_t mutexMessage = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexCasesInserees = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexScore = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexAnalyse = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexTraitement = PTHREAD_MUTEX_INITIALIZER;


pthread_cond_t condCasesInserees = PTHREAD_COND_INITIALIZER;
pthread_cond_t condScore = PTHREAD_COND_INITIALIZER;
pthread_cond_t condAnalyse = PTHREAD_COND_INITIALIZER;
pthread_cond_t condTraitement = PTHREAD_COND_INITIALIZER;

pthread_key_t cletab;


EVENT_GRILLE_SDL event;
///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc,char* argv[])
{
  struct sigaction A;
  A.sa_handler = HandlerSIGALRM;
  A.sa_flags = 0;
  sigemptyset(&A.sa_mask);
  sigaction(SIGALRM, &A, NULL);
  A.sa_handler = HandlerSIGUSR1;
  sigaction(SIGUSR1, &A, NULL);
  sigset_t mask;
  sigfillset(&mask);
  pthread_sigmask(SIG_SETMASK,&mask, NULL);
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
  
  trace("threadDeFileMessage", threadFM);
  pthread_create(&threadFM, NULL, threadDeFileMessage, NULL);
  trace("threadPiece", threadP);
  pthread_create(&threadP, NULL, threadPiece, NULL);
  trace("threadEvent", threadEvnt);
  pthread_create(&threadEvnt, NULL, threadEvent, NULL);
  trace("threadScore", threadScre);
  pthread_create(&threadScre, NULL, threadScore, NULL);
  trace("threadNettoyeur", threadNtyr);
  pthread_create(&threadNtyr, NULL, threadNettoyeur, NULL);

  pthread_key_create(&cletab, LiberationCle);
  for(int i = 0; i < 9; i++)
  {
    for(int j = 0; j < 9; j++)
    {
      trace("threadCases", tabThreadCase[i][j]);
      CASE* temp = (CASE*) malloc(sizeof(CASE));
      temp->ligne = i;
      temp->colonne = j;
      pthread_create(&tabThreadCase[i][j], NULL, threadCases, temp);
    }
  }
  pthread_join(threadP, NULL);
  tracer("Demande d'annulation du thread Event", threadEvnt);
  pthread_cancel(threadEvnt);
  tracer("Annulation acceptee", threadEvnt);
  for(int i = 0; i < 9; i++)
  {
    for(int j = 0; j < 9; j++)
    {
      tracer("Demande d'annulation de la case: ", tabThreadCase[i][j]);
      pthread_cancel(tabThreadCase[i][j]);
      pthread_join(tabThreadCase[i][j], NULL);
      tracer("Annulation acceptee", tabThreadCase[i][j]);
    }
  }
  bool ok = false;
  while(!ok)
  {
    if (event.type == CROIX) ok = true;
  }
  // Fermeture de la fenetre
  printf("(MAIN %p) Fermeture de la fenetre graphique...",pthread_self()); fflush(stdout);
  FermetureFenetreGraphique();
  printf("OK\n");
  tracer("Demande d'annulation du thread de la file de message", threadFM);
  pthread_cancel(threadFM);
  pthread_key_delete(cletab);

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
  pthread_cleanup_push(FonctionTerminaison, (void *) message);
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

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  }
  pthread_cleanup_pop(1);
}
void* threadPiece(void* arg)
{
  while(1)
  {
    bool trouve = false;
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
    bool possible = true;
    for(int L = 0; L < 9 && !trouve; L++)
    {
      for (int C = 0; C < 9 && !trouve; C++)
      {
        possible = true;

        for(int i = 0; i < pieceEnCours.nbCases; i++)
        {
          int l = L + pieceEnCours.cases[i].ligne;
          int c = C + pieceEnCours.cases[i].colonne;
          
          if (l < 0 || l >= 9 || c < 0 || c >= 9)
          {
            possible = false;
            break;
          }

          if (tab[l][c] != VIDE)
          {
            possible = false;
            break;
          }
        }
        if (possible)
          trouve = true;
      }
    }
    if(!trouve)
    {
      setMessage(" Game Over ", false);
      pthread_exit(NULL);
    }
    bool correct = false;

    if(trouve)
    {
      while (!correct)
      {  
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
        correct = true;
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
          pthread_mutex_lock(&mutexTraitement);
          traitementEnCours = true;
          pthread_mutex_unlock(&mutexTraitement);

          DessineVoyant(8,10,BLEU);
          for (int i = 0; i < nbCasesInserees; i++)
          {
            int L = temp[i].ligne;
            int C = temp[i].colonne;

            tab[L][C] = BRIQUE;
            DessineBrique(L, C, false);
            pthread_kill(tabThreadCase[L][C], SIGUSR1);
          }
          score += nbCasesInserees;
          MAJScore = true;
          pthread_cond_signal(&condScore);
          pthread_mutex_lock(&mutexTraitement);
          while(traitementEnCours)
          {
            pthread_cond_wait(&condTraitement, &mutexTraitement);
          }
          pthread_mutex_unlock(&mutexTraitement);
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
          pthread_mutex_lock(&mutexCasesInserees);
          while(nbCasesInserees < pieceEnCours.nbCases)
          {
            pthread_cond_wait(&condCasesInserees, &mutexCasesInserees);
          }
          pthread_mutex_unlock(&mutexCasesInserees);
        }
        nbCasesInserees = 0;
      }
    }
  }
}
void* threadEvent(void* arg)
{
  int ligne, colonne;
  while(1)
  {
    event = ReadEvent();
    if (event.type == CROIX)
    {
      pthread_cancel(threadP);
      pthread_exit(NULL);
    }
    if (event.type == CLIC_GAUCHE)
    {
      
      if (event.ligne >= 0 && event.ligne < 9 && event.colonne >= 0 && event.colonne < 9)
      {
        if (tab[event.ligne][event.colonne] == VIDE)
        {
          pthread_mutex_lock(&mutexCasesInserees);
          if(traitementEnCours)
          {
            DessineVoyant(8,10,ROUGE);
            Attente(400);
            DessineVoyant(8,10,BLEU);
          }
          else if (!traitementEnCours)
          {
            DessineDiamant(event.ligne,event.colonne,pieceEnCours.couleur);
            tab[event.ligne][event.colonne] = DIAMANT;
            casesInserees[nbCasesInserees].ligne = event.ligne;
            casesInserees[nbCasesInserees].colonne = event.colonne;
            nbCasesInserees++;
            pthread_cond_signal(&condCasesInserees);
          }
          pthread_mutex_unlock(&mutexCasesInserees);
        }
        else if (tab[event.ligne][event.colonne] == BRIQUE ||tab[event.ligne][event.colonne] == DIAMANT)
        {
          DessineVoyant(8,10,ROUGE);
          Attente(400);
          if(traitementEnCours)
            DessineVoyant(8,10,BLEU);
          else
            DessineVoyant(8,10,VERT);
        }
      }
      else
      {
        DessineVoyant(8,10,ROUGE);
        Attente(400);
        if(traitementEnCours)
          DessineVoyant(8,10,BLEU);
        else
          DessineVoyant(8,10,VERT);
      }
    }
    if (event.type == CLIC_DROIT)
    {
      pthread_mutex_lock(&mutexCasesInserees);
      for (int i = 0; i < nbCasesInserees; i++)
      {
        EffaceCarre(casesInserees[i].ligne, casesInserees[i].colonne);
        ligne = casesInserees[i].ligne;
        colonne = casesInserees[i].colonne;
        tab[ligne][colonne] = VIDE;
      }
      nbCasesInserees = 0;
      pthread_mutex_unlock(&mutexCasesInserees);

    }
  }

  pthread_exit(NULL);
}
void* threadScore(void* arg)
{
  while(1)
  {
    pthread_mutex_lock(&mutexScore);
    while(!MAJScore && !MAJCombos)
    {
      pthread_cond_wait(&condScore, &mutexScore);
    }
    int co1 = (combos / 1000) % 10;
    int co2 = (combos / 100) % 10;
    int co3 = (combos / 10) % 10;
    int co4 = combos % 10;
    int c1 = (score / 1000) % 10; // séparation
    int c2 = (score / 100) % 10;
    int c3 = (score / 10) % 10;
    int c4 = score % 10;
    DessineChiffre(1, 14, c1); // 1000
    DessineChiffre(1, 15, c2); // 0100
    DessineChiffre(1, 16, c3); // 0010
    DessineChiffre(1, 17, c4); // 0001
    DessineChiffre(8, 14, co1);
    DessineChiffre(8, 15, co2);
    DessineChiffre(8, 16, co3);
    DessineChiffre(8, 17, co4);
    pthread_mutex_unlock(&mutexScore);
    MAJScore = false;
    MAJCombos = false;
  }
}
void* threadCases(void* arg)
{
  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask, SIGUSR1);
  pthread_sigmask(SIG_SETMASK, &mask, NULL);
  CASE* c = (CASE *) arg;
  pthread_setspecific(cletab, c);
  while(1)
  {
    pause();
  }
}
void* threadNettoyeur(void* arg)
{
  while(1)
  {
    int combo = 0;
    pthread_mutex_lock(&mutexAnalyse);
    while(nbAnalyses < pieceEnCours.nbCases)
    {
      pthread_cond_wait(&condAnalyse, &mutexAnalyse);
    }
    if(nbLignesCompletes == 0 && nbCarresComplets == 0 && nbColonnesCompletes == 0)
    {
      nbAnalyses = 0;
    }
    else
    {
      Attente(2000);
      for(int i = 0; i < nbLignesCompletes; i++)
      {
        int L = lignesCompletes[i];
        for(int C = 0; C < 9; C++)
        {
          EffaceCarre(L,C);
          tab[L][C] = VIDE;
        }
        combo++;
      }

      for(int i = 0; i < nbColonnesCompletes; i++)
      {
        int C = colonnesCompletes[i];
        for(int L = 0; L < 9; L++)
        {
          EffaceCarre(L,C);
          tab[L][C] = VIDE;
        }
        combo++;
      }

      for(int i = 0; i < nbCarresComplets; i++)
      {
        int num = carresComplets[i];
        int L0 = (num / 3) * 3;
        int C0 = (num % 3) * 3;
        for (int L = L0; L < L0 + 3; L++)
        {
          for(int C = C0; C < C0 + 3; C++)
          {
            EffaceCarre(L,C);
            tab[L][C] = VIDE;
          }
        }
        combo++;
      }
      if (combo == 1)
      {
        setMessage(" Simple Combo ", true);
        score += 10;
        MAJScore = true;
      }
      else if (combo == 2)
      {
        setMessage(" Double Combo ", true);
        score += 25;
        MAJScore = true;
      }
      else if (combo == 3)
      {
        setMessage(" Triple Combo ", true);
        score += 40;
        MAJScore = true;
      }
      else if (combo == 4)
      {
        setMessage(" Quadruple Combo ", true);
        score += 55;
        MAJScore = true;
      }
      combos += combo;
      nbAnalyses = 0;
      nbLignesCompletes = 0;
      nbColonnesCompletes = 0;
      nbCarresComplets = 0;
      MAJCombos = true;
      pthread_cond_signal(&condScore);
    }
    traitementEnCours = false;
    DessineVoyant(8,10,VERT);
    pthread_cond_signal(&condTraitement);
    pthread_mutex_unlock(&mutexAnalyse);
  }
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
void LiberationCle(void* p)
{
  free(p);
}
void HandlerSIGALRM(int sig)
{
  setMessage(" Jeu en cours ", false);
}
void HandlerSIGUSR1(int sig)
{
  CASE* c = (CASE*) pthread_getspecific(cletab);
  bool pleine = false;
  for (int C = 0; C < 9; C++)
  {
    if (tab[c->ligne][C] == BRIQUE)
    {
      pleine = true;
    }
    else
    {
      pleine = false;
      break;
    }
  }
  bool isTrue = false;
  for(int i = 0; i < nbLignesCompletes; i++)
  {
    if (c->ligne == lignesCompletes[i])
    {
      isTrue = true;
      break;
    }
  }
  pthread_mutex_lock(&mutexAnalyse);
  if (pleine)
  {
    if (!isTrue)
    {
      lignesCompletes[nbLignesCompletes] = c->ligne;
      nbLignesCompletes++;
    }
    int L = c->ligne;
    for(int C = 0; C < 9; C++)
    {
      DessineBrique(L, C, true);
    }
  }
  pleine = false;
  for (int L = 0; L < 9; L++)
  {
    if (tab[L][c->colonne] == BRIQUE)
    {
      pleine = true;
    }
    else
    {
      pleine = false;
      break;
    }
  }

  isTrue = false;
  for(int i = 0; i < nbColonnesCompletes; i++)
  {
    if (c->colonne == colonnesCompletes[i])
    {
      isTrue = true;
      break;
    }
  }

  if (pleine)
  {
    if (!isTrue)
    {
      colonnesCompletes[nbColonnesCompletes] = c->colonne;
      nbColonnesCompletes++;
    }
    int C = c->colonne;
    for(int L = 0; L < 9; L++)
    {
      DessineBrique(L, C, true);
    }
  }

  int numCarre = (c->ligne/3)*3 + (c->colonne/3);

  int L0 = (c->ligne/3)*3;
  int C0 = (c->colonne/3)*3;

  bool plein = true;

  for(int i = L0; i < L0+3 && plein; i++)
  {
    for(int j = C0; j < C0+3; j++)
    {
      if(tab[i][j] != BRIQUE)
      {
        plein = false;
        break;
      }
    }
  }

  bool deja = false;

  for(int i = 0; i < nbCarresComplets; i++)
  {
    if(carresComplets[i] == numCarre)
    {
      deja = true;
      break;
    }
  }

  if(plein && !deja)
  {
    carresComplets[nbCarresComplets] = numCarre;
    nbCarresComplets++;

    for(int i = L0; i < L0+3; i++)
    {
      for(int j = C0; j < C0+3; j++)
      {
        DessineBrique(i,j,true);
      }
    }
  }
  nbAnalyses++;
  pthread_cond_signal(&condAnalyse);
  pthread_mutex_unlock(&mutexAnalyse);
}
void FonctionTerminaison(void* arg)
{
  free(arg);
}