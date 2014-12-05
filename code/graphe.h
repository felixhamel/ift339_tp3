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

#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <map>

#include <stdint.h> // Compatibilite avec uint32_t

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
      uint32_t zone[4];            // Autres trucs pour un usage futur

      // Partie variable
      uint16_t nbArcs;              // Nombre d'arcs
      map<uint32_t, float> liens;   // Liens entre ce noeud et d'autres noeuds
      string nom;                   // Nom du noeud
    };

    map<uint32_t, noeud> lesNoeuds;   // Les noeuds deja lus
    uint32_t nbNOEUDS;                // Le nombre de noeuds
    ifstream DATA;                    // Le flot d'entree
    uint32_t DEBUT;                   // Debut de la partie fixe
    string nom;                       // Nom du graphe
    uint8_t architecture;             // Architecture du fichier (Little or Big endian)

    /**
     * Lire le noeud avec le numero donne en paramatre.
     * @param noeud Numero du noeud a lire dans le fichier et a charger en memoire.
     */
    void lire_noeud(uint32_t);

    /**
     * Lire un uint32_t a la position courante de DATA. (4 octets)
     * @param noeud variable a extraire du fichier.
     */
    void lire(uint32_t&);

    /**
     * Lire un uint16_t a la position courante de DATA. (2 octets)
     * @param noeud variable a extraire du fichier.
     */
    void lire(uint16_t&);

    /**
     * Lire un float a la position courante de DATA. (4 octets)
     * @param a variable a extraire du fichier.
     */
    void lire(float&);

    /**
     * Constructeur par copie.
     */
    graphe(const graphe &graphe)=delete;

    /**
     * Desactiver l'operateur =. Il vaut mieux utiliser le constructeur par copie.
     */
    graphe& operator=(const graphe &graphe)=delete;

  public:

    /**
     * Constructeur.
     * @param: cheminVersFichier Chemin vers le fichier contenant le graphe a lire.
     */
    graphe(const string&);

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
     * Afficher le noeud avec le numero donne en paramatre.
     * S'il n'est pas en memoire, ce dernier va etre lu.
     * @param noeud numero du noeud a afficher.
     */
    void afficher_noeud(const uint32_t);

    /**
     * Trouver un numero de noeud
     *
     * @param  LAT      Latitude du point.
     * @param  LON      Longitude du point.
     * @return uint32_t Numero du noeud le plus proche du point.
     */
    uint32_t localiser(float&, float&);


    /**
     * Trouver si le noeud courant est le noeud le plus proche ou si c'est un des noeud lie a ce point.
     *
     * @param numero_noeud          Noeud a verifier
     * @param distance_point_noeud  Distance la plus courte entre le point et le noeud le plus proche trouve jusqu'a maintenant.
     * @param latitude              Latitude du point.
     * @param longitude             Longitude du point.
     */
    void trouver_noeud_le_plus_proche(uint32_t&, pair<float, uint32_t>&, float&, float&);

    /**
     * Une chaine qui identifie un noeud
     *
     * @param  uint32_t Numero du noeud.
     * @return string   Nom du noeud
     */
    string operator[](uint32_t&);

    /**
     * Distance avec le point et le noeud.
     *
     * @param  uint32_t Numero du noeud a calculer la distance ave le point
     * @param  float    Latitude du point.
     * @param  float    Longitude du point.
     * @return float    Distance entre le noeud et le point.
     */
    float distance(uint32_t&, float&, float&);
};
