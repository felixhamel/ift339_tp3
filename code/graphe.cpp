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

uint8_t architectureMachine = 2;

#include <chrono>
#include <ctime>
#include <cmath>
#include <thread>
#include <set>
#include <vector>

graphe::graphe(const string& cheminVersFichier)
{
	DATA.open(cheminVersFichier.c_str(), ios::in|ios::binary);

	if(!DATA.is_open()) {
		cout << "Erreur d'ouverture du fichier, celui-ci n'existe pas." << endl;
		exit(1);
	}

	DATA >> nom >> nbNOEUDS >> architecture;
	DATA.ignore(1);
	DEBUT = DATA.tellg();

	// Determiner l'architecture de la machine courante
	short int word = 0x0001;
	char *byte = (char *) &word;
	architectureMachine = (byte[0] ? ___LITTLE_ENDIAN : ___BIG_ENDIAN);
}

graphe::~graphe()
{
	// Fermer le fichier à la sortie du programme.
	if (DATA.is_open()) {
		DATA.close();
	}
}

void graphe::lire_noeud(uint32_t noeud)
{
	if(noeud < nbNOEUDS) {
		// Si le noeud n'a jamais ete lu, alors il va l'être !
		if(lesNoeuds[noeud].partieVariable == 0) {

			// Lecture des donn.es statiques du noeud
			DATA.seekg(DEBUT + (28 * noeud), ios::beg);
			this->lire(lesNoeuds[noeud].partieVariable);
			this->lire(lesNoeuds[noeud].latitude);
			this->lire(lesNoeuds[noeud].longitude);
			for(int i = 0; i < 4; ++i) {
				this->lire(lesNoeuds[noeud].zone[i]);
			}

			// Lecture des donnees variable du noeud
			DATA.seekg(lesNoeuds[noeud].partieVariable);
			this->lire(lesNoeuds[noeud].nbArcs);
			for(int i = 0; i < lesNoeuds[noeud].nbArcs; ++i) {
				uint32_t numero;
				this->lire(numero);
				this->lire(lesNoeuds[noeud].liens[numero]);
		  }

			// Lecture du nom du noeud
			uint16_t nombreDeCaracteres;
			this->lire(nombreDeCaracteres);

			char* nom = new char[nombreDeCaracteres];
			DATA.read(nom, nombreDeCaracteres);
			lesNoeuds[noeud].nom = ((string)nom).substr(0, nombreDeCaracteres-1); // Pour enlever des espaces s'il y en a

			delete[] nom;
	  }
	}
}

void graphe::lire(uint16_t& noeud)
{
	DATA.read(reinterpret_cast<char*>(&noeud), 2);

	// Si l'architecture diffère du fichier, on swap les bits.
	if(architecture != architectureMachine) {
		// http://stackoverflow.com/a/2182184
		noeud = (noeud >> 8) | (noeud << 8);
	}
}

void graphe::lire(uint32_t& noeud)
{
	DATA.read(reinterpret_cast<char*>(&noeud), 4);

	// Si l'architecture diffère du fichier, on swap les bits.
	if(architecture != architectureMachine) {
		// http://stackoverflow.com/a/13001420
		noeud = (noeud >> 24) | ((noeud << 8) & 0x00FF0000) | ((noeud >> 8) & 0x0000FF00) | (noeud << 24);
	}
}

void graphe::lire(float& a)
{
	DATA.read(reinterpret_cast<char*>(&a), 4);

	// Si l'architecture diffère du fichier, on swap les bits.
	if(architecture != architectureMachine) {
		char *floatToConvert = ( char* ) & a;
		// http://stackoverflow.com/a/2782742
		swap(floatToConvert[0], floatToConvert[3]);
		swap(floatToConvert[1], floatToConvert[2]);
	}
}

const uint32_t graphe::size() const
{
	return this->nbNOEUDS;
}

void graphe::afficher_noeud(const uint32_t noeud)
{
	this->lire_noeud(noeud);
	auto leNoeud = lesNoeuds[noeud];

	cout << "+--------------------------------------------------------------------+" << endl;
	cout << " Noeud #" << noeud << endl;
	cout << " - Latitude: " << leNoeud.latitude << endl;
	cout << " - Longitude: " << leNoeud.longitude << endl;
	cout << " - Nom: " << leNoeud.nom << endl;
	for(int i = 0; i < 4; ++i) {
		this->lire_noeud(leNoeud.zone[i]);
		cout << " -> Zone[" << i << "]: " << leNoeud.zone[i] << " - nom = " << lesNoeuds[leNoeud.zone[i]].nom << endl;
	}
	cout << " - Nombre d'arcs: " << leNoeud.nbArcs << endl;
	for(map<uint32_t, float>::iterator it = leNoeud.liens.begin(); it != leNoeud.liens.end(); ++it) {
		cout << " -> Arc vers le noeud " << it->first << " avec un poids de " << it->second << endl;
	}
	cout << "+--------------------------------------------------------------------+" << endl;
}

uint32_t graphe::localiser(float& latitude, float& longitude)
{
	// Distance entre chaque noeud et le point donne par la latitude et la longitude.
	pair<float, uint32_t> distance_point_noeud = make_pair(std::numeric_limits<float>::infinity(), -1);

	// Essayer de trouver le noeud le plus proche en debutant par le noeud 0
	uint32_t debut = 0;
	this->trouver_noeud_le_plus_proche(debut, distance_point_noeud, latitude, longitude);

	// Retourner le noeud le plus proche
	return distance_point_noeud.second;
}

void graphe::trouver_noeud_le_plus_proche(uint32_t& numero_noeud, pair<float, uint32_t>& distance_point_noeud, float& latitude, float& longitude)
{
	// Aller chercher les informations du noeud dans le graphe
	this->lire_noeud(numero_noeud);
	noeud* noeud = &lesNoeuds[numero_noeud];

	// Calculer la distance entre le noeud et le point
	float distance = this->distance(numero_noeud, latitude, longitude);

	// On conserve uniquement l'information qui peut nous être utile
	if(distance <= distance_point_noeud.first) {
		distance_point_noeud = make_pair(distance, numero_noeud);
	}

	// Trouver la zone a explorer
	int zone_du_noeud_a_explorer;
	if(latitude > noeud->latitude) {
		if(longitude > noeud->longitude) { // Zone 0
			zone_du_noeud_a_explorer = 0;
		} else if(longitude < noeud->longitude) { // Zone 1
			zone_du_noeud_a_explorer = 1;
		}
	} else {
		if(longitude > noeud->longitude) { // Zone 2
			zone_du_noeud_a_explorer = 2;
		} else if(longitude < noeud->longitude) { // Zone 3
			zone_du_noeud_a_explorer = 3;
		}
	}

	// Trouver quelles zones explorer
	set<int> zones_a_explorer;
	zones_a_explorer.insert(zone_du_noeud_a_explorer);

	// Il ne sert a rien d'aller voir une zone où nous nous éloignons du point a trouver
	if((latitude - noeud->latitude) < distance_point_noeud.first) {
		zones_a_explorer.insert((zone_du_noeud_a_explorer > 1 ? (zone_du_noeud_a_explorer - 2) : (zone_du_noeud_a_explorer + 2)));
	}
	if((longitude - noeud->longitude) < distance_point_noeud.first) {
		zones_a_explorer.insert((zone_du_noeud_a_explorer % 2 == 0 ? (zone_du_noeud_a_explorer + 1) : (zone_du_noeud_a_explorer - 1)));
	}

	// Lancer l'exploration dans ces zones
	for(set<int>::iterator it = zones_a_explorer.begin(); it != zones_a_explorer.end(); ++it) {
		if(noeud->zone[*it] > 0) {
			this->trouver_noeud_le_plus_proche(noeud->zone[*it], distance_point_noeud, latitude, longitude);
		}
	}
}

string graphe::operator[](uint32_t& numero_noeud)
{
	this->lire_noeud(numero_noeud);
	return lesNoeuds[numero_noeud].nom;
}

float graphe::distance(uint32_t& numero_noeud, float& latitude, float& longitude)
{
	this->lire_noeud(numero_noeud);
	noeud& noeud = lesNoeuds[numero_noeud];

	float x2 = pow((longitude - noeud.longitude), 2);
	float y2 = pow((latitude - noeud.latitude), 2);
	float c2 = pow(cos((noeud.latitude + latitude) / 2 * M_PI / 180), 2);
	return sqrt(y2 + x2 * c2) * 111;
}
