#include "graphe.h"

using namespace std;

/**
 * Fonction mère du programme. Va lire un fichier chemin.txt et
 * trouver le chemin pour chaque couple de noeud contenu dans ce dernier.
 * @return code de sortie du programme.
 */
int main()
{
  ifstream chemin("chemins.txt", ios::in);
  if(!chemin.is_open()) {
    cout << "Fichier chemins.txt introuvable. Veuillez corriger le problème." << endl;
    exit(1);
  }

  // Lecture du nom du fichier contenant le graphe
  string nomFichier;
  chemin >> nomFichier;

  // Création du graphe
  graphe dijkstra(nomFichier);

  // Lecture des chemins
  while(true) {
    uint32_t premierNoeud;
    uint32_t secondNoeud;
    chemin >> premierNoeud >> secondNoeud;
    chemin.ignore(1);

    if(chemin.eof()) {
      break;
    }

    cout << "Trouver le chemin entre le premier noeud " << premierNoeud;
    cout << " et le deuxieme " << secondNoeud << " : " << endl;

    dijkstra.trouver_chemin_optimal(premierNoeud, secondNoeud);
  }
}
