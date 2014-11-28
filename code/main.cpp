#include "graphe.h"

using namespace std;

/**
 * Fonction mère du programme. Va lire un fichier chemin.txt et
 * trouver le chemin pour chaque couple de noeud contenu dans ce dernier.
 * @return code0 de sortie du programme.
 */
int main()
{
  float LAT,LON;
  uint32_t i;
  string nom_graphe;

  cout << "Entrez le nom du fichier: ";
  cin >> nom_graphe;

  graphe G(nom_graphe);

  cout << "Veuillez entrer une latitude et une longitude: ";
  while(cin>>LAT>>LON){
    cout << "LAT:" << LAT << " LON:" << LON << endl;
    i=G.localiser(LAT,LON);
    cout << "#" << i << " " << G[i] << " (d=" << G.distance(i,LAT,LON) << ")" << endl;
    cout << "Veuillez entrer une latitude et une longitude: ";
  }
}
