/**
 * IFT339 - TP3
 * Recherche d’information bi-demensionnelle
 *
 * Membres de l'équipe :
 *  - Félix Hamel
 *  - Nabil Diab
 *
 * Universite de Sherbrooke, automne 2014
 */

#include "graphe.h"

using namespace std;

int main()
{
  float LAT,LON;
  uint32_t i;

  ifstream points("points.txt", ios::in);

  // Lecture du nom du fichier contenant le graphe
  string nomFichier;
  points >> nomFichier;

  // Création du graphe
  graphe G(nomFichier);

  // Lecture des points
  while(points >> LAT >> LON) {
    cout << "LAT:" << LAT << " LON:" << LON << endl;
    i=G.localiser(LAT,LON);
    cout << "#" << i << " " << G[i] << " (d=" << G.distance(i,LAT,LON) << ")" << endl;

    if(points.eof()) {
      break;
    }
  }
}
