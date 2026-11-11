# ABR, arbre rouge-noir, map et unordered_map (C++)

TP de C++ — ILISI 2e année — BENNOUR RAFAA

Comparaison de quatre structures pour stocker des clés et les retrouver :

| Structure | Ce que c'est |
|---|---|
| **ABR** | arbre binaire de recherche normal, **non équilibré** (écrit à la main) |
| **Rouge-noir** | arbre rouge-noir **écrit à la main** : insertion, rotations, recoloration |
| **`std::map`** | arbre rouge-noir de la bibliothèque standard |
| **`std::unordered_map`** | table de hachage de la bibliothèque standard |

On mesure l'**insertion** de N clés, puis la **recherche** des N clés dans un ordre aléatoire,
avec des clés insérées dans un ordre **aléatoire** puis dans l'ordre **croissant**.

## Organisation

```
src/
  ArbreBinaireRecherche.h   ABR non équilibré
  ArbreRougeNoir.h          arbre rouge-noir (insertion, rotations, vérification des règles)
  DemoRBT.cpp               insertion pas à pas, arbre affiché après chaque valeur
  Benchmark.cpp             comparaison des 4 structures
resultats/
  benchmark.txt             tableaux lisibles
  demo_rbt.txt              sortie de la démonstration
```

## Compiler et lancer

```
make          # compile dans bin/   (Windows : mingw32-make)
make demo     # démonstration de l'arbre rouge-noir
make bench    # benchmark (environ 30 secondes)
```

Sans make :

```
g++ -O2 -std=c++17 src/Benchmark.cpp -o Benchmark
./Benchmark          # résultats affichés dans la console
```

Avant de mesurer, le benchmark vérifie que l'arbre rouge-noir respecte ses règles
(racine noire, jamais deux rouges de suite, même nombre de noirs sur chaque chemin),
et que chaque clé cherchée est bien trouvée.

## Résultats (N = 1 000 000, ns par opération)

| Ordre | Structure | Hauteur | Insertion | Recherche |
|---|---|---:|---:|---:|
| aléatoire | ABR | 50 | 800 | 865 |
| aléatoire | Rouge-noir | 24 | 829 | 345 |
| aléatoire | map | – | 1 251 | 1 131 |
| aléatoire | unordered_map | – | 200 | 37 |
| croissant | ABR (N = 20 000) | 20 000 | 23 006 | 28 216 |
| croissant | Rouge-noir | 37 | 134 | 361 |
| croissant | map | – | 113 | 925 |
| croissant | unordered_map | – | 38 | 30 |

Tableaux complets dans `resultats/benchmark.txt`. Mesures faites sur un Intel Xeon 2,1 GHz,
g++ 13.3 `-O2` : les valeurs changent selon la machine, les tendances restent.

## Ce qu'il faut retenir

1. **L'équilibrage sauve l'arbre dans l'ordre croissant.** L'ABR devient une liste (hauteur = N) :
   à N = 20 000, il est environ **430 fois plus lent** à l'insertion que le rouge-noir.
   Le rouge-noir reste à une hauteur de 37 pour un million de clés.
2. **En ordre aléatoire, l'ABR tient bien à l'insertion** (hauteur 50, assez proche du rouge-noir),
   mais le rouge-noir **cherche 2,5 fois plus vite**, car son arbre est deux fois moins haut.
3. **Notre rouge-noir cherche environ 3 fois plus vite que `std::map`.** Les deux sont des arbres
   rouge-noir de même hauteur. La différence vient du code machine : notre boucle de recherche
   choisit l'enfant gauche ou droit **sans branchement** (instruction `cmov`), alors que `map::find`
   descend avec un `if` que le processeur prédit mal une fois sur deux ; `map::find` descend aussi
   toujours jusqu'en bas, alors que notre recherche s'arrête dès qu'elle trouve la clé.
   De plus, un nœud de `map<int,int>` est plus gros (il stocke une paire clé-valeur).
4. **`unordered_map` est la plus rapide** : environ 10 fois plus rapide que le rouge-noir pour chercher,
   car elle va directement dans la bonne case (O(1) en moyenne). Mais elle **ne garde aucun ordre** :
   impossible d'obtenir les clés triées, le minimum ou toutes les clés entre deux valeurs.
5. **Choisir** : `unordered_map` si on veut seulement savoir si une clé existe ; `map` (ou un arbre
   rouge-noir) si on a besoin de l'ordre ; jamais un ABR non équilibré si les données peuvent arriver triées.
