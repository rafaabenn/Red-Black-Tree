// Arbre rouge-noir ecrit a la main : insertion, suppression, recherche, rotations, verification des regles
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

    // Remplace le sous-arbre de racine u par le sous-arbre de racine v (v peut etre nullptr)
    void transplanter(NoeudRN* u, NoeudRN* v)
    {
        if (u->parent == nullptr)          racine = v;
        else if (u == u->parent->gauche)   u->parent->gauche = v;
        else                               u->parent->droite = v;
        if (v != nullptr) v->parent = u->parent;
    }

    static NoeudRN* minimum(NoeudRN* n) { while (n->gauche != nullptr) n = n->gauche; return n; }

    // Reparer apres suppression d'un noeud NOIR.
    // x a un "noir en trop" (double noir) : il manque un noir sur les chemins qui passent par x.
    // x peut etre nullptr, c'est pourquoi on garde aussi son parent p.
    void reparerSuppression(NoeudRN* x, NoeudRN* p)
    {
        while (x != racine && couleurDe(x) == NOIR) {
            if (x == p->gauche) {                    // ---- x est a GAUCHE ----
                NoeudRN* frere = p->droite;          // existe forcement (hauteur noire >= 1)
                if (couleurDe(frere) == ROUGE) {     // CAS 1 : frere rouge -> on se ramene aux cas 2-4
                    frere->couleur = NOIR;
                    p->couleur = ROUGE;
                    rotationGauche(p);
                    frere = p->droite;
                }
                if (couleurDe(frere->gauche) == NOIR && couleurDe(frere->droite) == NOIR) {
                    frere->couleur = ROUGE;          // CAS 2 : neveux noirs -> le frere devient rouge,
                    x = p;                           //         le probleme remonte au parent
                    p = x->parent;
                } else {
                    if (couleurDe(frere->droite) == NOIR) {  // CAS 3 : neveu proche rouge, lointain noir
                        frere->gauche->couleur = NOIR;
                        frere->couleur = ROUGE;
                        rotationDroite(frere);
                        frere = p->droite;
                    }
                    frere->couleur = p->couleur;     // CAS 4 : neveu lointain rouge -> rotation, termine
                    p->couleur = NOIR;
                    frere->droite->couleur = NOIR;
                    rotationGauche(p);
                    x = racine;
                }
            } else {                                 // ---- x est a DROITE (miroir) ----
                NoeudRN* frere = p->gauche;
                if (couleurDe(frere) == ROUGE) {     // CAS 1
                    frere->couleur = NOIR;
                    p->couleur = ROUGE;
                    rotationDroite(p);
                    frere = p->gauche;
                }
                if (couleurDe(frere->droite) == NOIR && couleurDe(frere->gauche) == NOIR) {
                    frere->couleur = ROUGE;          // CAS 2
                    x = p;
                    p = x->parent;
                } else {
                    if (couleurDe(frere->gauche) == NOIR) {  // CAS 3
                        frere->droite->couleur = NOIR;
                        frere->couleur = ROUGE;
                        rotationGauche(frere);
                        frere = p->gauche;
                    }
                    frere->couleur = p->couleur;     // CAS 4
                    p->couleur = NOIR;
                    frere->gauche->couleur = NOIR;
                    rotationDroite(p);
                    x = racine;
                }
            }
        }
        if (x != nullptr) x->couleur = NOIR;         // un rouge qui recoit le noir en trop devient noir
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

    // Suppression : 1) retirer comme dans un ABR   2) reparer si on a retire un noeud NOIR
    // Les noeuds sont deplaces (on change les pointeurs), jamais les valeurs copiees :
    // seul le noeud supprime disparait, les autres restent a la meme adresse (comme std::map).
    // Renvoie false si la valeur est absente.
    bool supprimer(int v)
    {
        NoeudRN* z = racine;                         // chercher le noeud
        while (z != nullptr && z->valeur != v)
            z = (v < z->valeur) ? z->gauche : z->droite;
        if (z == nullptr) return false;

        Couleur couleurRetiree = z->couleur;         // couleur du noeud qui quitte vraiment sa place
        NoeudRN* x;                                  // noeud qui prend cette place (peut etre nullptr)
        NoeudRN* parentX;                            // son parent

        if (z->gauche == nullptr) {                  // cas 1 : pas d'enfant gauche
            x = z->droite;
            parentX = z->parent;
            transplanter(z, z->droite);
        } else if (z->droite == nullptr) {           // cas 2 : pas d'enfant droit
            x = z->gauche;
            parentX = z->parent;
            transplanter(z, z->gauche);
        } else {                                     // cas 3 : deux enfants
            NoeudRN* y = minimum(z->droite);         // successeur : il prendra la place de z
            couleurRetiree = y->couleur;
            x = y->droite;                           // y n'a pas d'enfant gauche
            if (y->parent == z) {
                parentX = y;
            } else {
                parentX = y->parent;
                transplanter(y, y->droite);          // x remonte a la place de y
                y->droite = z->droite;
                y->droite->parent = y;
            }
            transplanter(z, y);                      // y prend la place de z...
            y->gauche = z->gauche;
            y->gauche->parent = y;
            y->couleur = z->couleur;                 // ...et sa couleur
        }
        delete z;

        if (couleurRetiree == NOIR)                  // retirer un rouge ne casse aucune regle
            reparerSuppression(x, parentX);
        return true;
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
