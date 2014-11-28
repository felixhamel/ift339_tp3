#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <stdint.h> // Compatibilité avec uint32_t

using namespace std;

#define ___LITTLE_ENDIAN 1
#define ___BIG_ENDIAN    0

extern uint8_t architectureMachine;

class graphe
{
  private:

    struct noeud  // Description de toutes les composantes d'un noeud
    {
      // Partie statique
      uint32_t partieVariable;
      float latitude;
      float longitude;
      uint32_t futur[4];            // Autres trucs pour un usage futur

      // Partie variable
      uint16_t nbArcs;              // Nombre d'arcs
      map<uint32_t, float> liens;   // Liens entre ce noeud et d'autres noeuds
      string nom;                   // Nom du noeud
    };

    map<uint32_t, noeud> lesNoeuds;   // Les noeuds deja lus
    uint32_t nbNOEUDS;                // Le nombre de noeuds
    ifstream DATA;                    // Le flot d'entrée
    uint32_t DEBUT;                   // Debut de la partie fixe
    string nom;                       // Nom du graphe
    uint8_t architecture;             // Architecture du fichier (Little or Big endian)

    /**
     * Lire le noeud avec le numéro donné en paramètre.
     * @param noeud Numéro du noeud a lire dans le fichier et a charger en mémoire.
     */
    void lire_noeud(uint32_t noeud);

    /**
     * Lire un uint32_t à la position courante de DATA. (4 octets)
     * @param noeud variable a extraire du fichier.
     */
    void lire(uint32_t& noeud);

    /**
     * Lire un uint16_t à la position courante de DATA. (2 octets)
     * @param noeud variable à extraire du fichier.
     */
    void lire(uint16_t& noeud);

    /**
     * Lire un float à la position courante de DATA. (4 octets)
     * @param a variable à extraire du fichier.
     */
    void lire(float& a);

    /**
     * Constructeur par copie.
     */
    graphe(const graphe &graphe)=delete;

    /**
     * Désactiver l'opérateur =. Il vaut mieux utiliser le constructeur par copie.
     */
    graphe& operator=(const graphe &graphe)=delete;

    /**
     * Afficher le chemin trouvé.
     * @param predecesseurs Predecesseurs de chaque noeud (le plus optimal)
     * @param premierNoeud  Le premier noeud
     * @param secondNoeud   Le dernier noeud du chemin
     */
    void afficher_chemin(map<uint32_t, uint32_t>& predecesseurs, const uint32_t& premierNoeud, const uint32_t& secondNoeud);

  public:

    /**
     * Constructeur.
     * @param: cheminVersFichier Chemin vers le fichier contenant le graphe a lire.
     */
    graphe(const string cheminVersFichier);

    /**
     * Destructeur. Va fermer le fichier contenant le graphe.
     */
    ~graphe();

    /**
     * Retourne le nombre de noeud dans le fichier du graphe.
     * @return uint32_t Nombre de noeuds.
     */
    const uint32_t size() const;

    /**
     * Afficher le noeud avec le numéro donné en paramètre.
     * S'il n'est pas en mémoire, ce dernier va être lu.
     * @param noeud numéro du noeud a afficher.
     */
    void afficher_noeud(const uint32_t noeud);

    /**
     * Trouve et affiche le chemin le plus optimal entre les 2 noeuds.
     * @param premierNoeud Numéro du premier noeud.
     * @param secondNoeud  Numéro du second noeud.
     */
    void trouver_chemin_optimal(const uint32_t premierNoeud, const uint32_t secondNoeud);
};
