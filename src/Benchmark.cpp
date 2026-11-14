// Benchmark : ABR normal vs arbre rouge-noir (ecrit a la main) vs std::map vs std::unordered_map
//   ./bin/Benchmark        (resultats affiches dans la console)
// Pour chaque structure : insertion de N cles, recherche des N cles dans un ordre aleatoire,
// puis suppression des N cles dans ce meme ordre aleatoire.
// Scenarios : cles aleatoires, cles croissantes (pire cas de l'ABR),
// collisions (k cles par case de la table de hachage, k = 1, 10, 100, 1000 : pire cas de unordered_map).
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <numeric>
#include <algorithm>
#include <climits>
#include <map>
#include <unordered_map>
#include "ArbreBinaireRecherche.h"
#include "ArbreRougeNoir.h"
using namespace std;
using namespace std::chrono;

struct Mesure { double inserer = -1, rechercher = -1, supprimer = -1; int hauteur = -1; };

const int ESSAIS = 3;
const int MAX_ABR_CROISSANT = 10000;   // au-dela, l'ABR degenere est trop lent (O(N^2))

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

// Les 4 structures, mesurees avec les memes fonctions partout
Mesure mesurerABR(const vector<int>& cles, const vector<int>& requetes)
{
    return mesurer<ABR>(cles, requetes,
        [](ABR& s, int x) { s.inserer(x); },
        [](const ABR& s, int x) { return s.rechercher(x); },
        [](ABR& s, int x) { return s.supprimer(x); },
        [](const ABR& s) { return s.hauteur(); });
}
Mesure mesurerRougeNoir(const vector<int>& cles, const vector<int>& requetes)
{
    return mesurer<ArbreRougeNoir>(cles, requetes,
        [](ArbreRougeNoir& s, int x) { s.inserer(x); },
        [](const ArbreRougeNoir& s, int x) { return s.rechercher(x); },
        [](ArbreRougeNoir& s, int x) { return s.supprimer(x); },
        [](const ArbreRougeNoir& s) { return s.hauteur(); });
}
Mesure mesurerMap(const vector<int>& cles, const vector<int>& requetes)
{
    return mesurer<map<int, int>>(cles, requetes,
        [](map<int, int>& s, int x) { s.emplace(x, x); },
        [](const map<int, int>& s, int x) { return s.find(x) != s.end(); },
        [](map<int, int>& s, int x) { return s.erase(x) == 1; },
        [](const map<int, int>&) { return -1; });           // hauteur non accessible
}
template <typename Table>                                    // unordered_map ou TableReservee
Mesure mesurerTable(const vector<int>& cles, const vector<int>& requetes)
{
    return mesurer<Table>(cles, requetes,
        [](Table& s, int x) { s.emplace(x, x); },
        [](const Table& s, int x) { return s.find(x) != s.end(); },
        [](Table& s, int x) { return s.erase(x) == 1; },
        [](const Table&) { return -1; });                   // pas un arbre
}

// Scenario "collisions" (pire cas de la table de hachage).
// Avec g++, std::hash<int> renvoie la cle elle-meme et la case vaut (cle % B), B = nombre de cases.
// On fixe B des la construction (reserve) pour que la table ne se redimensionne jamais :
// deux cles qui ont le meme reste modulo B tombent dans la meme case et forment une liste chainee.
int CASES_RESERVEES = 0;
struct TableReservee : unordered_map<int, int> { TableReservee() { reserve(CASES_RESERVEES); } };

// Cles avec exactement k cles par case : on utilise M = N/k cases, et dans la case c on met
// les cles c, c+B, c+2B, ..., c+(k-1)B  (toutes ont le meme reste c modulo B).
// k = 1 : aucune collision (cas normal). k = N : toutes les cles dans la meme case.
vector<int> clesAvecCollisions(int N, int k, long long B)
{
    int M = (N + k - 1) / k;                                 // nombre de cases utilisees
    vector<int> cles(N);
    for (int i = 0; i < N; i++) cles[i] = (int)(i % M + (long long)(i / M) * B);
    shuffle(cles.begin(), cles.end(), mt19937(42));
    return cles;
}

string cellule(double v) { if (v < 0) return "-"; ostringstream o; o << fixed << setprecision(1) << v; return o.str(); }

// Resultats d'un scenario : r[N][structure]
typedef map<int, map<string, Mesure>> Resultats;

// Un tableau par operation : les structures en colonnes, les tailles N en lignes
void afficherTableaux(const vector<string>& colonnes, const vector<int>& tailles, const Resultats& r)
{
    const int LARG = 14;
    auto tableau = [&](const string& titre, const vector<string>& cols, auto valeur) {
        cout << left << setw(16) << titre;
        for (const string& c : cols) cout << right << setw(LARG) << c;
        cout << "\n" << string(16 + LARG * cols.size(), '-') << "\n";
        for (int N : tailles) {
            cout << left << setw(16) << N;
            for (const string& c : cols) cout << right << setw(LARG) << valeur(r.at(N).at(c));
            cout << "\n";
        }
        cout << "\n";
    };
    tableau("Insertion (ns)",   colonnes, [](const Mesure& m) { return cellule(m.inserer); });
    tableau("Recherche (ns)",   colonnes, [](const Mesure& m) { return cellule(m.rechercher); });
    tableau("Suppression (ns)", colonnes, [](const Mesure& m) { return cellule(m.supprimer); });
    tableau("Hauteur", {"ABR", "Rouge-noir"},
            [](const Mesure& m) { return m.hauteur < 0 ? string("-") : to_string(m.hauteur); });
}

void progression(const string& scenario, int N) { cerr << "  mesure : " << scenario << ", N = " << N << "\n"; }

int main(int argc, char** argv)
{
    // "Benchmark rapide" : seulement N = 1000, pour verifier que tout marche en quelques secondes
    bool rapide = argc > 1 && string(argv[1]) == "rapide";
    vector<int> tailles = rapide ? vector<int>{1000} : vector<int>{1000, 10000, 100000, 1000000, 2000000};

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

    // ---------- Scenarios 1 et 2 : ordre aleatoire, ordre croissant ----------
    for (string ordre : {"aleatoire", "croissant"}) {
        Resultats r;
        for (int N : tailles) {
            progression(ordre, N);
            vector<int> cles(N);
            iota(cles.begin(), cles.end(), 0);
            if (ordre == "aleatoire") shuffle(cles.begin(), cles.end(), mt19937(42));
            vector<int> requetes = cles;
            shuffle(requetes.begin(), requetes.end(), mt19937(7));

            if (ordre == "aleatoire" || N <= MAX_ABR_CROISSANT) r[N]["ABR"] = mesurerABR(cles, requetes);
            else                                                r[N]["ABR"] = Mesure();   // non mesure
            r[N]["Rouge-noir"]    = mesurerRougeNoir(cles, requetes);
            r[N]["map"]           = mesurerMap(cles, requetes);
            r[N]["unordered_map"] = mesurerTable<unordered_map<int, int>>(cles, requetes);
        }
        cout << "\n=== Cles inserees dans l'ordre " << ordre << " ===\n\n";
        afficherTableaux({"ABR", "Rouge-noir", "map", "unordered_map"}, tailles, r);
    }

    // ---------- Scenario 3 : collisions dans la table de hachage ----------
    // k = nombre de cles par case. Plus k est grand, plus la liste a parcourir dans chaque case est longue.
    // Les arbres ne dependent que de l'ordre des cles, pas de leur case : on les mesure une seule fois
    // (avec les cles du niveau k = 1000) pour montrer qu'ils ne sont pas genes.
    const int NIVEAUX[] = {1, 10, 100, 1000};
    Resultats r;
    vector<string> colonnes = {"ABR", "Rouge-noir", "map"};
    for (int k : NIVEAUX) colonnes.push_back("u_map k=" + to_string(k));
    string cases;                                            // "N : B" pour chaque N
    for (int N : tailles) {
        progression("collisions", N);
        CASES_RESERVEES = N;
        long long B = TableReservee().bucket_count();
        cases += "  " + to_string(N) + " : " + to_string(B);

        vector<int> clesArbres = clesAvecCollisions(N, 1000, B);
        vector<int> reqArbres = clesArbres;
        shuffle(reqArbres.begin(), reqArbres.end(), mt19937(7));
        r[N]["ABR"]        = mesurerABR(clesArbres, reqArbres);
        r[N]["Rouge-noir"] = mesurerRougeNoir(clesArbres, reqArbres);
        r[N]["map"]        = mesurerMap(clesArbres, reqArbres);

        for (int k : NIVEAUX) {
            string nom = "u_map k=" + to_string(k);
            if ((long long)(k - 1) * B + N > INT_MAX) {          // la plus grande cle ne tient pas dans un int
                r[N][nom] = Mesure();
                continue;
            }
            vector<int> cles = clesAvecCollisions(N, k, B);
            vector<int> requetes = cles;
            shuffle(requetes.begin(), requetes.end(), mt19937(7));

            // Verification : la table ne s'est pas agrandie et la case la plus remplie contient bien k cles
            TableReservee t;
            for (int x : cles) t.emplace(x, x);
            size_t pire = 0;
            for (size_t c = 0; c < t.bucket_count(); c++) pire = max(pire, t.bucket_size(c));
            if ((long long)t.bucket_count() != B || pire != (size_t)min(k, N))
                cout << "ATTENTION : repartition des cles inattendue (autre compilateur que g++ ?)\n";

            r[N][nom] = mesurerTable<TableReservee>(cles, requetes);
        }
    }
    cout << "\n=== Collisions : k cles par case de unordered_map (u_map) ===\n\n";
    afficherTableaux(colonnes, tailles, r);
    cout << "Nombre de cases B de la table, pour chaque N :" << cases << "\n";
}
