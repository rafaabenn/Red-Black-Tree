// Benchmark : ABR normal vs arbre rouge-noir (ecrit a la main) vs std::map vs std::unordered_map
//   ./bin/Benchmark        (resultats affiches dans la console)
// Pour chaque structure : insertion de N cles, recherche des N cles dans un ordre aleatoire,
// puis suppression des N cles dans ce meme ordre aleatoire.
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <numeric>
#include <algorithm>
#include <map>
#include <unordered_map>
#include "ArbreBinaireRecherche.h"
#include "ArbreRougeNoir.h"
using namespace std;
using namespace std::chrono;

struct Mesure { double inserer = -1, rechercher = -1, supprimer = -1; int hauteur = -1; };

const int ESSAIS = 3;
const int MAX_ABR_CROISSANT = 20000;   // au-dela, l'ABR degenere est trop lent (O(N^2))

double ns(steady_clock::time_point t0) { return duration<double, nano>(steady_clock::now() - t0).count(); }

// Mesure generique : construire la structure, inserer, chercher, puis supprimer.
// "inserer", "chercher", "supprimer" et "hauteur" sont des fonctions qui savent manipuler la structure S.
template <typename S, typename Ins, typename Rech, typename Supp, typename Haut>
Mesure mesurer(const vector<int>& cles, const vector<int>& requetes, Ins inserer, Rech chercher, Supp supprimer, Haut hauteur)
{
    Mesure meilleure;
    for (int essai = 0; essai < ESSAIS; essai++) {
        S s;
        auto t0 = steady_clock::now();
        for (int x : cles) inserer(s, x);
        double ti = ns(t0) / cles.size();

        long long trouves = 0;
        t0 = steady_clock::now();
        for (int x : requetes) trouves += chercher(s, x);
        double tr = ns(t0) / requetes.size();
        if (trouves != (long long)requetes.size()) cout << "ERREUR : une cle n'a pas ete trouvee\n";

        meilleure.hauteur = hauteur(s);              // hauteur avant de tout supprimer

        long long supprimes = 0;
        t0 = steady_clock::now();
        for (int x : requetes) supprimes += supprimer(s, x);
        double ts = ns(t0) / requetes.size();
        if (supprimes != (long long)requetes.size()) cout << "ERREUR : une cle n'a pas ete supprimee\n";

        if (essai == 0 || ti < meilleure.inserer) meilleure.inserer = ti;
        if (essai == 0 || tr < meilleure.rechercher) meilleure.rechercher = tr;
        if (essai == 0 || ts < meilleure.supprimer) meilleure.supprimer = ts;
    }
    return meilleure;
}

string cellule(double v) { if (v < 0) return "-"; ostringstream o; o << fixed << setprecision(1) << v; return o.str(); }

int main()
{
    const char* noms[4] = {"ABR", "Rouge-noir", "map", "unordered_map"};

    // Verification : l'arbre rouge-noir respecte ses regles sur 100 000 cles aleatoires et croissantes
    {
        vector<int> v(100000); iota(v.begin(), v.end(), 0);
        ArbreRougeNoir a1; for (int x : v) a1.inserer(x);
        shuffle(v.begin(), v.end(), mt19937(1));
        ArbreRougeNoir a2; for (int x : v) a2.inserer(x);
        if (!a1.estValide() || !a2.estValide()) { cout << "ERREUR : regles rouge-noir cassees\n"; return 1; }

        // Suppression de la moitie des cles (une sur deux), dans un ordre aleatoire
        shuffle(v.begin(), v.end(), mt19937(2));
        for (int x : v) if (x % 2 == 0 && !a2.supprimer(x)) { cout << "ERREUR : suppression ratee\n"; return 1; }
        if (!a2.estValide()) { cout << "ERREUR : regles rouge-noir cassees apres suppression\n"; return 1; }
        for (int x = 0; x < 100000; x++)
            if (a2.rechercher(x) != (x % 2 == 1)) { cout << "ERREUR : mauvais contenu apres suppression\n"; return 1; }
        for (int x : v) a2.supprimer(x);             // tout supprimer
        if (a2.hauteur() != 0) { cout << "ERREUR : l'arbre n'est pas vide\n"; return 1; }

        cout << "Verification : les regles rouge-noir sont respectees (100 000 cles, insertion et suppression).\n\n";
    }

    cout << "Temps moyen par operation, en nanosecondes. '-' = non mesure (trop long) ou sans objet.\n";

    for (string ordre : {"aleatoire", "croissant"}) {
        cout << "\n=== Cles inserees dans l'ordre " << ordre << " ===\n\n";
        cout << left << setw(10) << "N" << setw(16) << "Structure" << right
             << setw(10) << "Hauteur" << setw(13) << "Insertion" << setw(13) << "Recherche"
             << setw(13) << "Suppression" << "\n";
        cout << string(75, '-') << "\n";
        for (int N : {1000, 10000, 20000, 100000, 1000000}) {
            vector<int> cles(N);
            iota(cles.begin(), cles.end(), 0);
            if (ordre == "aleatoire") shuffle(cles.begin(), cles.end(), mt19937(42));
            vector<int> requetes = cles;
            shuffle(requetes.begin(), requetes.end(), mt19937(7));

            Mesure m[4];
            if (ordre == "aleatoire" || N <= MAX_ABR_CROISSANT)
                m[0] = mesurer<ABR>(cles, requetes,
                    [](ABR& s, int x) { s.inserer(x); },
                    [](const ABR& s, int x) { return s.rechercher(x); },
                    [](ABR& s, int x) { return s.supprimer(x); },
                    [](const ABR& s) { return s.hauteur(); });
            m[1] = mesurer<ArbreRougeNoir>(cles, requetes,
                [](ArbreRougeNoir& s, int x) { s.inserer(x); },
                [](const ArbreRougeNoir& s, int x) { return s.rechercher(x); },
                [](ArbreRougeNoir& s, int x) { return s.supprimer(x); },
                [](const ArbreRougeNoir& s) { return s.hauteur(); });
            m[2] = mesurer<map<int, int>>(cles, requetes,
                [](map<int, int>& s, int x) { s.emplace(x, x); },
                [](const map<int, int>& s, int x) { return s.find(x) != s.end(); },
                [](map<int, int>& s, int x) { return s.erase(x) == 1; },
                [](const map<int, int>&) { return -1; });           // hauteur non accessible
            m[3] = mesurer<unordered_map<int, int>>(cles, requetes,
                [](unordered_map<int, int>& s, int x) { s.emplace(x, x); },
                [](const unordered_map<int, int>& s, int x) { return s.find(x) != s.end(); },
                [](unordered_map<int, int>& s, int x) { return s.erase(x) == 1; },
                [](const unordered_map<int, int>&) { return -1; }); // pas un arbre

            for (int k = 0; k < 4; k++) {
                cout << left << setw(10) << (k == 0 ? to_string(N) : "") << setw(16) << noms[k] << right
                     << setw(10) << (m[k].hauteur < 0 ? string("-") : to_string(m[k].hauteur))
                     << setw(13) << cellule(m[k].inserer) << setw(13) << cellule(m[k].rechercher)
                     << setw(13) << cellule(m[k].supprimer) << "\n";
            }
            cout << "\n";
        }
    }
    cout << "Hauteur : nombre de niveaux de l'arbre (map : non accessible ; unordered_map : table de hachage).\n";
}
