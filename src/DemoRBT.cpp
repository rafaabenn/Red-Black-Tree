// Demonstration : insertion pas a pas dans l'arbre rouge-noir
#include "ArbreRougeNoir.h"
#include <initializer_list>

void tester(const string& titre, initializer_list<int> valeurs)
{
    cout << "===== " << titre << " =====\n";
    ArbreRougeNoir a;
    for (int v : valeurs) {
        a.inserer(v);
        cout << "\nApres insertion de " << v << " :\n";
        a.afficher();
    }
    cout << "\nRegles respectees : " << (a.estValide() ? "oui" : "NON") << "\n\n";
}

int main()
{
    tester("7 58 48 2 17 84 82", {7, 58, 48, 2, 17, 84, 82});
}
