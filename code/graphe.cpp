#include "graphe.h"

uint8_t architectureMachine = 2;

#include <set>
#include <chrono>
#include <ctime>
#include <vector>
#include <deque>

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

			// Lecture des données statiques du noeud
			DATA.seekg(DEBUT + (28 * noeud), ios::beg);
			this->lire(lesNoeuds[noeud].partieVariable);
			this->lire(lesNoeuds[noeud].latitude);
			this->lire(lesNoeuds[noeud].longitude);
			for(int i = 0; i < 4; ++i) {
				this->lire(lesNoeuds[noeud].futur[i]);
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
	cout << " - Nom: " << leNoeud.nom << " - " << hex << leNoeud.nom << " - " << leNoeud.nom.length() << endl;
	for(int i = 0; i < 4; ++i) {
		cout << " -> Futur[" << i << "]: " << leNoeud.futur[i] << endl;
	}
	cout << " - Nombre d'arcs: " << leNoeud.nbArcs << endl;
	for(map<uint32_t, float>::iterator it = leNoeud.liens.begin(); it != leNoeud.liens.end(); ++it) {
		cout << " -> Arc vers le noeud " << it->first << " avec un poids de " << it->second << endl;
	}
	cout << "+--------------------------------------------------------------------+" << endl;
}

void graphe::trouver_chemin_optimal(const uint32_t premierNoeud, const uint32_t secondNoeud)
{
	map<uint32_t, uint32_t> predecesseurs;			// Noeud, prédécesseur du noeud
	map<uint32_t, float> total;								  // Noeud, poids
	multimap<float, uint32_t> totalInverse;		  // Poids, Noeuds
	set<uint32_t> noeudsObserve;								// Les noeuds déjà observés
  uint32_t noeudCourant = premierNoeud;
	float poidsTotal = 0.f;											// Calculer le poids total

	// On va ajouter 0 dans le total et le total inversé afin d'éviter de faire du
	// code custom juste pour la première boucle.
	total[premierNoeud] = 0;
	totalInverse.insert(pair<float, uint32_t>(0, premierNoeud));

	// Chrono
	chrono::time_point<chrono::system_clock> debut, fin;
	debut = chrono::system_clock::now();

	// Boucle afin de déterminer le chemin le plus optimal pour se rendre entre
	// le premier noeud et le deuxième.
	while(noeudCourant != secondNoeud) {

		// Aller chercher le noeud avec le poids le plus petit en partant
		while(noeudsObserve.find(noeudCourant) != noeudsObserve.end() && totalInverse.size() > 0) {
			noeudCourant = totalInverse.begin()->second;
			totalInverse.erase(totalInverse.begin());
		}
		this->lire_noeud(noeudCourant);

		// Si TotalInverse est vide et qu'on n'est plus sur le premier noeud,
		// alors on a fait le tour du graphe et on peut maintenant savoir le
		// chemin le plus efficace à l'aide du map des prédécesseurs.
		if(totalInverse.size() == 0 && noeudCourant != premierNoeud) break;

		// Puisqu'on a trouvé un noeud qu'on a pas encore parcouru, on l'ajoute a
		// la liste des noeuds parcourus et on le retire de TotalInverse.
		noeudsObserve.insert(noeudCourant);
		//totalInverse.erase(totalInverse.begin());

		// Si on est rendu au noeud qu'on voulait trouver, on a trouvé le chemin
		// le plus optimal !
		if(noeudCourant != secondNoeud) {
			for(map<uint32_t, float>::iterator it = lesNoeuds[noeudCourant].liens.begin(); it != lesNoeuds[noeudCourant].liens.end(); ++it) {

				// On conserve seulement le chemin avec le plus petit poids
				float poidsTotal = it->second + total[noeudCourant];
				if(total[it->first] == 0 || total[it->first] > poidsTotal) {
					total[it->first] = poidsTotal;
					totalInverse.insert(pair<float, uint32_t>(poidsTotal, it->first));
					predecesseurs[it->first] = noeudCourant;
				}
			}
		}
	}

	// On veut savoir le temps qu'a pris la méthode pour trouver le chemin le
	// plus optimal.
	fin = chrono::system_clock::now();
	chrono::duration<double> tempsEcoule = fin - debut;

	// Afficher le chemin qu'on vient de trouver.
	afficher_chemin(predecesseurs, premierNoeud, secondNoeud);

	// Afficher le temps
	cout << " - Temps pour trouver le meilleur chemin : " << fixed << tempsEcoule.count() << "s." << endl << endl;
	cout.unsetf(ios_base::fixed);
}

void graphe::afficher_chemin(map<uint32_t, uint32_t>& predecesseurs, const uint32_t& premierNoeud, const uint32_t& secondNoeud)
{
	// Calculer le poids total
	float poidsTotal = 0.f;
	deque<uint32_t> cheminNormal;

	// On va remettre le chemin en ordre du début jusqu'à la fin.
	uint32_t noeudCourant = secondNoeud;
	while(noeudCourant != premierNoeud) {
		cheminNormal.push_front(noeudCourant);
		noeudCourant = predecesseurs[noeudCourant];
	}
	cheminNormal.push_front(premierNoeud);

	// Afficher le chemin
	noeud* ptrNoeudPrecedent = nullptr;
	noeud* ptrNoeudCourant = nullptr;

	for(size_t i = 0; i < cheminNormal.size(); ++i) {
		ptrNoeudCourant = &lesNoeuds[cheminNormal[i]];

		if(i > 0) {
			// Retrouver le poids du lien entre les deux noeuds
			poidsTotal += ptrNoeudPrecedent->liens[cheminNormal[i]];
			cout << " " << ptrNoeudPrecedent->nom << " <=> " << ptrNoeudCourant->nom;
			cout << " | Poids du lien : " << ptrNoeudPrecedent->liens[cheminNormal[i]] << endl;
		}

		// Le noeud courant devient le noeud précédent dans la prochaine exécution
		ptrNoeudPrecedent = ptrNoeudCourant;
	}
	cout << "--------------------------------------------------------------------------" << endl;
	cout << " - Poids total du chemin le plus court : " << poidsTotal << endl;
}
