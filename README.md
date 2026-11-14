# ABR, arbre rouge-noir, map et unordered_map (C++)

TP de C++

Comparaison de quatre structures pour stocker des clés et les retrouver :

| Structure | Ce que c'est |
|---|---|
| **ABR** | arbre binaire de recherche normal, **non équilibré** (écrit à la main) |
| **Rouge-noir** | arbre rouge-noir **écrit à la main** : insertion, suppression, rotations, recoloration |
| **`std::map`** | arbre rouge-noir de la bibliothèque standard |
| **`std::unordered_map`** | table de hachage de la bibliothèque standard |

On mesure l'**insertion** de N clés, la **recherche** des N clés dans un ordre aléatoire, puis leur **suppression**,
pour N = 1 000, 10 000, 100 000, 1 000 000 et 2 000 000, dans trois scénarios :

| Scénario | Clés | Qui est mis en difficulté |
|---|---|---|
| **aléatoire** | 0 … N-1 mélangées | personne : c'est le cas normal |
| **croissant** | 0, 1, 2, … N-1 | l'**ABR**, qui dégénère en liste |
| **collisions** | k clés par case de la table de hachage (k = 1, 10, 100, 1000) | `unordered_map`, dont les cases deviennent des listes |

## Organisation

```
src/
  ArbreBinaireRecherche.h   ABR non équilibré
  ArbreRougeNoir.h          arbre rouge-noir (insertion, suppression, rotations, vérification des règles)
  DemoRBT.cpp               insertion puis suppression pas à pas, arbre affiché à chaque étape
  Benchmark.cpp             comparaison des 4 structures
resultats/
  benchmark.txt             tableaux lisibles
  demo_rbt.txt              sortie de la démonstration
```

## Compiler et lancer

Démonstration de l'arbre rouge-noir (insertion puis suppression pas à pas) :

```
g++ -O2 -std=c++17 src/DemoRBT.cpp -o DemoRBT
./DemoRBT
```

Benchmark (environ 20 minutes, résultats affichés dans la console ; pour chaque scénario, un tableau par opération
avec les structures en colonnes et les tailles N en lignes) :

```
g++ -O2 -std=c++17 src/Benchmark.cpp -o Benchmark
./Benchmark
```

Pour vérifier rapidement que tout fonctionne (N = 1 000 seulement, quelques secondes) : `./Benchmark rapide`.

Avant de mesurer, le benchmark vérifie que l'arbre rouge-noir respecte ses règles après insertion et après suppression
(racine noire, jamais deux rouges de suite, même nombre de noirs sur chaque chemin),
et que chaque clé cherchée est bien trouvée.

### Comment on provoque les collisions

Avec g++, `std::hash<int>` renvoie la clé elle-même, et la case d'une clé est `clé % B`, où B est le nombre de cases
de la table. On fixe B dès le départ avec `reserve(N)` pour que la table ne se redimensionne jamais, puis on choisit
les clés : dans la case c, on met `c, c+B, c+2B, …, c+(k-1)B`. Elles ont toutes le même reste modulo B, donc elles
s'empilent dans la même case, qui devient une liste chaînée de k éléments.
k = 1 est le cas normal (aucune collision) ; k = 1000 oblige chaque opération à parcourir une liste de 1000 clés.
Le programme vérifie à chaque exécution que la répartition obtenue est bien celle voulue.
Les arbres reçoivent les mêmes clés : comme ils ne regardent que l'ordre des clés, et pas leur case, ils ne sont pas gênés.

## Résultats (N = 2 000 000, ns par opération)

| Scénario | Structure | Hauteur | Insertion | Recherche | Suppression |
|---|---|---:|---:|---:|---:|
| aléatoire | ABR | 57 | 765 | 669 | 789 |
| aléatoire | Rouge-noir | 25 | 742 | 402 | 928 |
| aléatoire | map | – | 801 | 478 | 1 033 |
| aléatoire | unordered_map | – | 311 | 68 | 415 |
| croissant | ABR (N = 10 000) | 10 000 | 12 377 | 15 724 | 7 068 |
| croissant | Rouge-noir | 39 | 163 | 376 | 724 |
| croissant | map | – | 167 | 422 | 831 |
| croissant | unordered_map | – | 95 | 69 | 357 |
| collisions | Rouge-noir | 25 | 751 | 396 | 892 |
| collisions | unordered_map k = 1 | – | 161 | 66 | 369 |
| collisions | unordered_map k = 10 | – | 692 | 641 | 622 |
| collisions | unordered_map k = 100 | – | 4 460 | 4 603 | 2 569 |
| collisions | unordered_map k = 1000 | – | 39 952 | 43 972 | 22 987 |

Dans l'ordre croissant, l'ABR n'est mesuré que jusqu'à N = 10 000 : au-delà, il est trop lent (O(N²)).

Tableaux complets dans `resultats/benchmark.txt`. Mesures faites sur un Intel Core i7-8550U, Windows 11,
MinGW g++ 15.2 `-O2` : les valeurs changent selon la machine, les tendances restent.

## Ce qu'on peut déduire

1. **L'équilibrage sauve l'arbre dans l'ordre croissant.** L'ABR devient une liste (hauteur = N) :
   à N = 10 000, il est environ **135 fois plus lent** à l'insertion, **380 fois** à la recherche
   et **60 fois** à la suppression que le rouge-noir, qui reste à une hauteur de 39 pour deux millions de clés.
2. **En ordre aléatoire, l'ABR tient bien** (hauteur 57 contre 25) : le rouge-noir cherche **1,7 fois plus vite**,
   insère aussi vite, mais **supprime environ 15 % plus lentement**, car il doit réparer les couleurs.
3. **Notre rouge-noir et `std::map` sont très proches**, ce qui est normal : ce sont deux arbres rouge-noir
   de même hauteur. Le nôtre cherche **10 à 15 % plus vite** (notre recherche s'arrête dès qu'elle trouve la clé,
   alors que `map::find` descend toujours jusqu'en bas) et supprime un peu plus vite ; l'insertion est identique.
4. **Sans collisions, `unordered_map` est la plus rapide** : environ **6 fois plus rapide** que le rouge-noir
   pour chercher, car elle va directement dans la bonne case (O(1) en moyenne). Mais elle **ne garde aucun ordre** :
   impossible d'obtenir les clés triées, le minimum ou toutes les clés entre deux valeurs.
5. **Avec des collisions, la table de hachage s'effondre.** Le temps par opération croît à peu près
   **proportionnellement à k**, le nombre de clés par case (recherche à N = 2 M : 66 → 641 → 4 603 → 43 972 ns).
   Dès k = 10, elle est **plus lente que le rouge-noir** à la recherche ; à k = 1000, elle est **110 fois plus lente**
   que lui et **660 fois plus lente** qu'elle-même sans collisions. Le rouge-noir, lui, garde exactement la même
   hauteur et les mêmes temps (396 ns contre 402 en aléatoire) : son O(log n) est **garanti**, celui de la table
   n'est qu'une moyenne.
   Ce n'est pas qu'un cas d'école : des attaques réelles (« hash flooding ») envoient exprès des clés qui entrent
   en collision pour ralentir un serveur.
6. **Chaque structure a son pire cas** : l'ordre croissant pour l'ABR, les collisions pour la table de hachage.
   Le rouge-noir est le seul des trois à rester rapide dans tous les scénarios.
7. **Choisir** : `unordered_map` si on veut seulement savoir si une clé existe et qu'on fait confiance à la fonction
   de hachage ; `map` (ou un arbre rouge-noir) si on a besoin de l'ordre ou d'un temps garanti ;
   jamais un ABR non équilibré si les données peuvent arriver triées.

Le rapport complet est dans `rapport/rapport_rbt.pdf`.
