#include "graphe.h"

uint8_t architectureMachine = 2;

#include <chrono>
#include <ctime>
#include <cmath>

#define PI (3.141592653589793)

graphe::graphe(const string cheminVersFichier)
{
	DATA.open(cheminVersFichier.c_str(), ios::in|ios::binary);

	if(!DATA.is_open()) {
		cout << "Erreur d'ouverture du fichier, celui-ci n'existe pas." << endl;
		exit(1);
	}

	DATA >> nom >> nbNOEUDS >> architecture;
	DATA.ignore(1);
	DEBUT = DATA.tellg();

	// Déterminer l'architecture de la machine courante
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
		// Si le noeud n'a jamais été lu, alors il va l'être !
		if(lesNoeuds[noeud].partieVariable == 0) {

			// Lecture des donn.es statiques du noeud
			DATA.seekg(DEBUT + (28 * noeud), ios::beg);
			this->lire(lesNoeuds[noeud].partieVariable);
			this->lire(lesNoeuds[noeud].latitude);
			this->lire(lesNoeuds[noeud].longitude);
			for(int i = 0; i < 4; ++i) {
				this->lire(lesNoeuds[noeud].zone[i]);
			}

			// Lecture des données variable du noeud
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
	/*cout << " - Nombre d'arcs: " << leNoeud.nbArcs << endl;
	for(map<uint32_t, float>::iterator it = leNoeud.liens.begin(); it != leNoeud.liens.end(); ++it) {
		cout << " -> Arc vers le noeud " << it->first << " avec un poids de " << it->second << endl;
	}*/
	cout << "+--------------------------------------------------------------------+" << endl;
}

uint32_t graphe::localiser(float latitude, float longitude)
{
	this->afficher_noeud(372);

	uint32_t depart = 0;
	return this->trouver_noeud_le_plus_proche(depart, latitude, longitude);
}

uint32_t graphe::trouver_noeud_le_plus_proche(uint32_t& numero_noeud, float& latitude, float& longitude)
{
	float distance = this->distance(numero_noeud, latitude, longitude);
	noeud* noeud = &lesNoeuds[numero_noeud];

	cout << "Debut -> NoeudCourant: " << noeud->nom << endl;
	this->afficher_noeud(numero_noeud);

	uint32_t noeudPlusProche = 0;
	int zoneDuNoeudAExplorer = 0;

	// Trouver la zone a explorer
	if(latitude > noeud->latitude) {
		if(longitude > noeud->longitude) { // Zone 0
			zoneDuNoeudAExplorer = 0;
		} else if(longitude < noeud->longitude) { // Zone 1
			zoneDuNoeudAExplorer = 1;
		}
	} else {
		if(longitude > noeud->longitude) { // Zone 2
			zoneDuNoeudAExplorer = 2;
		} else if(longitude < noeud->longitude) { // Zone 3
			zoneDuNoeudAExplorer = 3;
		}
	}

	// Calculer la distance
	float distance_zone = this->distance(noeud->zone[zoneDuNoeudAExplorer], latitude, longitude);
	if(distance < distance_zone) {
		cout << "Debut -> NoeudCourant: " << noeud->nom << endl;
		return numero_noeud;
	}
	noeudPlusProche = this->trouver_noeud_le_plus_proche(noeud->zone[zoneDuNoeudAExplorer], latitude, longitude);

	// Regarder si nous devrions regarder dans une autre zone
	this->lire_noeud(noeud->zone[zoneDuNoeudAExplorer]);
	auto* noeudAExplorer = &lesNoeuds[noeud->zone[zoneDuNoeudAExplorer]];
	set<int> autresZones = this->trouver_zones_a_explorer(noeud, noeudAExplorer, zoneDuNoeudAExplorer, latitude, longitude);

	if(autresZones.size() > 0) {
		cout << "Explorer d'autres zones : ";
		for(set<int>::iterator it = autresZones.begin(); it != autresZones.end(); ++it) {
			cout << "Zone additionnelle : " <<  lesNoeuds[noeud->zone[*it]].nom  << " # " << noeud->zone[*it] << " ## " << *it << endl;
			float noeudZonePlusProche = this->trouver_noeud_le_plus_proche(noeud->zone[*it], latitude, longitude);
			float distanceNoeudZoneEtPoint = this->distance(noeud->zone[*it], latitude, longitude);
			if(distanceNoeudZoneEtPoint < distance_zone) {
				distance_zone = distanceNoeudZoneEtPoint;
				noeudPlusProche = noeud->zone[*it];
			}
		}
	}

	cout << "Fin -> NoeudCourant: " << noeud->nom << endl;


	return noeudPlusProche;
}

set<int> graphe::trouver_zones_a_explorer(noeud* noeudCourant, noeud* noeudAExplorer, int zoneDuNoeudAExplorer, float& latitude, float& longitude)
{
	set<int> zones;

	float distanceEntreNoeudCourantEtNoeudAExplorer = this->distance(noeudCourant, noeudAExplorer);
	float distanceEntreLatutideNoeudCourantEtLatitudeDuPoint = abs(noeudCourant->latitude - latitude);
	float distanceEntreLongitudeNoeudCourantEtLongitudeDuPoint = abs(noeudCourant->longitude - longitude);

	if(distanceEntreLatutideNoeudCourantEtLatitudeDuPoint < distanceEntreNoeudCourantEtNoeudAExplorer) {
		if(zoneDuNoeudAExplorer > 1) {
			zones.insert(zoneDuNoeudAExplorer - 2);
		} else {
			zones.insert(zoneDuNoeudAExplorer + 2);
		}
	}
	if(distanceEntreLongitudeNoeudCourantEtLongitudeDuPoint < distanceEntreNoeudCourantEtNoeudAExplorer) {
		if(zoneDuNoeudAExplorer % 2 == 0) {
			zones.insert(zoneDuNoeudAExplorer + 1);
		} else {
			zones.insert(zoneDuNoeudAExplorer - 1);
		}
	}

	return zones;
}

string graphe::operator[](uint32_t numero_noeud)
{
	this->lire_noeud(numero_noeud);
	return lesNoeuds[numero_noeud].nom;
}

float graphe::distance(uint32_t& numero_noeud, float& latitude, float& longitude)
{
	this->lire_noeud(numero_noeud);
	noeud* noeud = &lesNoeuds[numero_noeud];

	float x2 = pow((longitude - noeud->longitude), 2);
	float y2 = pow((latitude - noeud->latitude), 2);
	float c2 = pow(cos((noeud->latitude + latitude) / 2 * PI / 180), 2);
	return sqrt(y2 + x2 * c2) * 111;
}

float graphe::distance(noeud* noeudCourant, noeud* noeudDistant)
{
	float x2 = pow((noeudDistant->longitude - noeudCourant->longitude), 2);
	float y2 = pow((noeudDistant->latitude - noeudCourant->latitude), 2);
	float c2 = pow(cos((noeudCourant->latitude + noeudDistant->latitude) / 2 * PI / 180), 2);
	return sqrt(y2 + x2 * c2);
}
