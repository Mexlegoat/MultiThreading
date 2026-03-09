#ifndef RESSOURCES_H
#define RESSOURCES_H

// Macros pour les couleurs de fantome
#define ROUGE                     400000
#define VIOLET                    400001
#define VERT                      400002
#define ORANGE                    400003
#define BLEU                      400004
#define JAUNE                     400005

int  OuvertureFenetreGraphique();
int  FermetureFenetreGraphique();

void DessineDiamant(int l,int c,int couleur);
void DessineBrique(int l,int c,bool fusion);
void DessineVoyant(int l,int c,int couleur);
void DessineChiffre(int l,int c,int val);
void DessineLettre(int L,int C,char c);


#endif
