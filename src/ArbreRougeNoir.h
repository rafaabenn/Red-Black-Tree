// Arbre rouge-noir ecrit a la main : insertion, recherche, rotations, verification des regles
#pragma once
#include <iostream>
#include <string>
#include <algorithm>
using namespace std;

enum Couleur { ROUGE, NOIR };

struct NoeudRN {
    int valeur;
    Couleur couleur = ROUGE;          // tout nouveau noeud est rouge
    NoeudRN* gauche = nullptr;
    NoeudRN* droite = nullptr;
    NoeudRN* parent = nullptr;          // necessaire pour remonter (parent, grand-parent, oncle)
    NoeudRN(int v) : valeur(v) {}
};

class ArbreRougeNoir {
private:
    NoeudRN* racine = nullptr;

    // Une case vide (nullptr) compte comme NOIRE
    static Couleur couleurDe(NoeudRN* n) { return n == nullptr ? NOIR : n->couleur; }

    // Rotation gauche autour de x : l'enfant droit y monte a la place de x.
    //       x                y         |
    //      / \              / \        |
    //     a   y     ->     x   c       |
    //        / \          / \          |
    //       b   c        a   b         |
    // Cout : O(1), on ne change que quelques pointeurs.
    void rotationGauche(NoeudRN* x)
    {
        NoeudRN* y = x->droite;
        x->droite = y->gauche;                       // 1. b passe a droite de x
        if (y->gauche != nullptr) y->gauche->parent = x;

        y->parent = x->parent;                       // 2. y prend la place de x sous son parent
        if (x->parent == nullptr)          racine = y;
        else if (x == x->parent->gauche)   x->parent->gauche = y;
        else                               x->parent->droite = y;

        y->gauche = x;                               // 3. x passe a gauche de y
        x->parent = y;
    }

    // Rotation droite autour de x : le miroir de la rotation gauche.
    void rotationDroite(NoeudRN* x)
    {
        NoeudRN* y = x->gauche;
        x->gauche = y->droite;
        if (y->droite != nullptr) y->droite->parent = x;

        y->parent = x->parent;
        if (x->parent == nullptr)          racine = y;
        else if (x == x->parent->droite)   x->parent->droite = y;
        else                               x->parent->gauche = y;

        y->droite = x;
        x->parent = y;
    }

    // Reparer apres insertion : tant que z et son parent sont rouges
    void reparerInsertion(NoeudRN* z)
    {
        while (z != racine && couleurDe(z->parent) == ROUGE) {
            NoeudRN* p = z->parent;                    // parent (rouge, donc pas la racine)
            NoeudRN* g = p->parent;                    // grand-parent

            if (p == g->gauche) {                    // ---- le parent est a GAUCHE ----
                NoeudRN* oncle = g->droite;
                if (couleurDe(oncle) == ROUGE) {     // CAS 1 : oncle rouge -> recolorer
                    p->couleur = NOIR;
                    oncle->couleur = NOIR;
                    g->couleur = ROUGE;
                    z = g;                           // on remonte au grand-parent
                } else {
                    if (z == p->droite) {            // CAS 2 : zigzag -> rotation sur le parent
                        z = p;
                        rotationGauche(z);
                        p = z->parent;
                    }
                    p->couleur = NOIR;               // CAS 3 : ligne droite
                    g->couleur = ROUGE;              // echange des couleurs
                    rotationDroite(g);               // rotation sur le grand-parent
                }
            } else {                                 // ---- le parent est a DROITE (miroir) ----
                NoeudRN* oncle = g->gauche;
                if (couleurDe(oncle) == ROUGE) {     // CAS 1
                    p->couleur = NOIR;
                    oncle->couleur = NOIR;
                    g->couleur = ROUGE;
                    z = g;
                } else {
                    if (z == p->gauche) {            // CAS 2 : zigzag
                        z = p;
                        rotationDroite(z);
                        p = z->parent;
                    }
                    p->couleur = NOIR;               // CAS 3
                    g->couleur = ROUGE;
                    rotationGauche(g);
                }
            }
        }
        racine->couleur = NOIR;                      // la racine est toujours noire
    }

    void afficher(NoeudRN* n, int espace) const
    {
        if (n == nullptr) return;
        afficher(n->droite, espace + 6);
        cout << string(espace, ' ') << n->valeur << (n->couleur == ROUGE ? "R" : "N") << "\n";
        afficher(n->gauche, espace + 6);
    }

    // Renvoie le nombre de noeuds noirs vers les feuilles, ou -1 si une regle est cassee
    int verifier(NoeudRN* n) const
    {
        if (n == nullptr) return 1;                  // NIL compte comme un noir
        if (n->couleur == ROUGE &&
            (couleurDe(n->gauche) == ROUGE || couleurDe(n->droite) == ROUGE))
            return -1;                               // deux rouges de suite
        int g = verifier(n->gauche), d = verifier(n->droite);
        if (g == -1 || d == -1 || g != d) return -1; // pas le meme nombre de noirs
        return g + (n->couleur == NOIR ? 1 : 0);
    }

    static int hauteur(NoeudRN* n) { return n == nullptr ? 0 : 1 + max(hauteur(n->gauche), hauteur(n->droite)); }

    void liberer(NoeudRN* n) { if (n) { liberer(n->gauche); liberer(n->droite); delete n; } }

public:
    ArbreRougeNoir() = default;
    ArbreRougeNoir(const ArbreRougeNoir&) = delete;
    ArbreRougeNoir& operator=(const ArbreRougeNoir&) = delete;
    ~ArbreRougeNoir() { liberer(racine); }

    // Insertion : 1) comme dans un ABR, en rouge   2) reparer
    void inserer(int v)
    {
        NoeudRN* parent = nullptr;
        NoeudRN* courant = racine;
        while (courant != nullptr) {                 // descendre jusqu'a une place vide
            parent = courant;
            if (v == courant->valeur) return;        // pas de doublons
            courant = (v < courant->valeur) ? courant->gauche : courant->droite;
        }
        NoeudRN* z = new NoeudRN(v);                     // nouveau noeud ROUGE
        z->parent = parent;
        if (parent == nullptr)          racine = z;
        else if (v < parent->valeur)    parent->gauche = z;
        else                            parent->droite = z;

        reparerInsertion(z);
    }

    // Recherche : exactement comme dans un ABR, O(log n) garanti car la hauteur est <= 2 log2(n+1)
    bool rechercher(int v) const
    {
        NoeudRN* c = racine;
        while (c != nullptr) {
            if (v == c->valeur) return true;
            c = (v < c->valeur) ? c->gauche : c->droite;
        }
        return false;
    }

    int hauteur() const { return hauteur(racine); }
    void afficher() const { afficher(racine, 0); }
    bool estValide() const { return couleurDe(racine) == NOIR && verifier(racine) != -1; }
};
