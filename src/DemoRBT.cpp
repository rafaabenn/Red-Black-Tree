// Demonstration : insertion puis suppression pas a pas dans l'arbre rouge-noir
#include "ArbreRougeNoir.h"
#include <initializer_list>

void tester(const string& titre, initializer_list<int> aInserer, initializer_list<int> aSupprimer)
{
    cout << "===== " << titre << " =====\n";
    ArbreRougeNoir a;
    for (int v : aInserer) {
        a.inserer(v);
        cout << "\nApres insertion de " << v << " :\n";
        a.afficher();
    }
    cout << "\nRegles respectees : " << (a.estValide() ? "oui" : "NON") << "\n";

    for (int v : aSupprimer) {
        bool ok = a.supprimer(v);
        cout << "\nApres suppression de " << v << (ok ? "" : " (absent)") << " :\n";
        a.afficher();
        cout << "Regles respectees : " << (a.estValide() ? "oui" : "NON") << "\n";
    }
    cout << "\n";
}

int main()
{
    tester("7 58 48 2 17 84 82", {7, 58, 48, 2, 17, 84, 82}, {2, 58, 48, 7, 99});
}
