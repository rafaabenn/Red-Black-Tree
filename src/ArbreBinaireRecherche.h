// Arbre binaire de recherche (ABR)
// Regle : pour chaque noeud, les valeurs a gauche sont plus petites,
//         les valeurs a droite sont plus grandes.
#pragma once
#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>
using namespace std;

struct Noeud {
    int valeur;
    Noeud* gauche;
    Noeud* droite;
    Noeud(int v) : valeur(v), gauche(nullptr), droite(nullptr) {}
};

class ABR {
private:
    Noeud* racine = nullptr;
    int nb = 0;                                   // nombre de noeuds

    // ---------- fonctions recursives internes ----------
    static void detruire(Noeud* n)
    {
        if (n == nullptr) return;
        detruire(n->gauche);                      // les enfants d'abord
        detruire(n->droite);
        delete n;
    }
    static int hauteur(Noeud* n)
    {
        if (n == nullptr) return 0;
        return 1 + max(hauteur(n->gauche), hauteur(n->droite));
    }
    static void prefixe(Noeud* n, vector<int>& r)  // racine, gauche, droite
    {
        if (n == nullptr) return;
        r.push_back(n->valeur);
        prefixe(n->gauche, r);
        prefixe(n->droite, r);
    }
    static void infixe(Noeud* n, vector<int>& r)   // gauche, racine, droite -> ordre croissant
    {
        if (n == nullptr) return;
        infixe(n->gauche, r);
        r.push_back(n->valeur);
        infixe(n->droite, r);
    }
    static void suffixe(Noeud* n, vector<int>& r)  // gauche, droite, racine
    {
        if (n == nullptr) return;
        suffixe(n->gauche, r);
        suffixe(n->droite, r);
        r.push_back(n->valeur);
    }
    static void dessiner(Noeud* n, int espace)     // racine a gauche, sous-arbre droit en haut
    {
        if (n == nullptr) return;
        dessiner(n->droite, espace + 6);
        cout << string(espace, ' ') << n->valeur << "\n";
        dessiner(n->gauche, espace + 6);
    }

public:
    ~ABR() { detruire(racine); }

    // Insertion : on descend a gauche si plus petit, a droite si plus grand.
    // Renvoie false si la valeur existe deja (pas de doublons).
    bool inserer(int v)
    {
        Noeud** p = &racine;                      // adresse du lien a suivre
        while (*p != nullptr) {
            if (v < (*p)->valeur)      p = &(*p)->gauche;
            else if (v > (*p)->valeur) p = &(*p)->droite;
            else return false;                    // deja present
        }
        *p = new Noeud(v);                        // place vide trouvee
        nb++;
        return true;
    }

    // Recherche : meme descente que l'insertion
    bool rechercher(int v) const
    {
        Noeud* n = racine;
        while (n != nullptr) {
            if (v == n->valeur) return true;
            n = (v < n->valeur) ? n->gauche : n->droite;
        }
        return false;
    }

    // Minimum : tout a gauche ; maximum : tout a droite
    int minimum() const { Noeud* n = racine; while (n->gauche) n = n->gauche; return n->valeur; }
    int maximum() const { Noeud* n = racine; while (n->droite) n = n->droite; return n->valeur; }

    // Suppression : 3 cas
    //   1. feuille           -> on la retire
    //   2. un seul enfant    -> l'enfant prend sa place
    //   3. deux enfants      -> on copie le minimum du sous-arbre droit, puis on supprime ce minimum
    bool supprimer(int v)
    {
        Noeud** p = &racine;
        while (*p != nullptr && (*p)->valeur != v)
            p = (v < (*p)->valeur) ? &(*p)->gauche : &(*p)->droite;
        if (*p == nullptr) return false;          // absent

        Noeud* n = *p;
        if (n->gauche == nullptr) {               // cas 1 ou 2 (enfant droit ou aucun)
            *p = n->droite;
            delete n;
        } else if (n->droite == nullptr) {        // cas 2 (enfant gauche seulement)
            *p = n->gauche;
            delete n;
        } else {                                  // cas 3 : deux enfants
            Noeud** q = &n->droite;               // chercher le minimum a droite
            while ((*q)->gauche != nullptr) q = &(*q)->gauche;
            Noeud* m = *q;
            n->valeur = m->valeur;                // copier sa valeur
            *q = m->droite;                       // le minimum n'a pas d'enfant gauche
            delete m;
        }
        nb--;
        return true;
    }

    // Parcours
    vector<int> parcoursPrefixe() const { vector<int> r; prefixe(racine, r); return r; }
    vector<int> parcoursInfixe()  const { vector<int> r; infixe(racine, r);  return r; }
    vector<int> parcoursSuffixe() const { vector<int> r; suffixe(racine, r); return r; }
    vector<int> parcoursLargeur() const          // niveau par niveau, avec une file
    {
        vector<int> r;
        if (racine == nullptr) return r;
        queue<Noeud*> file;
        file.push(racine);
        while (!file.empty()) {
            Noeud* n = file.front(); file.pop();
            r.push_back(n->valeur);
            if (n->gauche) file.push(n->gauche);
            if (n->droite) file.push(n->droite);
        }
        return r;
    }

    int hauteur() const { return hauteur(racine); }
    int taille() const  { return nb; }
    bool vide() const   { return racine == nullptr; }
    void dessiner() const { dessiner(racine, 0); }
};
