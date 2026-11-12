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
avec des clés insérées dans un ordre **aléatoire** puis dans l'ordre **croissant**.

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

Benchmark (environ une minute, résultats affichés dans la console) :

```
g++ -O2 -std=c++17 src/Benchmark.cpp -o Benchmark
./Benchmark
```

Avant de mesurer, le benchmark vérifie que l'arbre rouge-noir respecte ses règles après insertion et après suppression
(racine noire, jamais deux rouges de suite, même nombre de noirs sur chaque chemin),
et que chaque clé cherchée est bien trouvée.

## Résultats (N = 1 000 000, ns par opération)

| Ordre | Structure | Hauteur | Insertion | Recherche | Suppression |
|---|---|---:|---:|---:|---:|
| aléatoire | ABR | 50 | 1 551 | 896 | 1 402 |
| aléatoire | Rouge-noir | 24 | 1 365 | 708 | 1 613 |
| aléatoire | map | – | 1 398 | 1 386 | 1 823 |
| aléatoire | unordered_map | – | 620 | 129 | 709 |
| croissant | ABR (N = 20 000) | 20 000 | 66 420 | 84 231 | 44 146 |
| croissant | Rouge-noir | 37 | 408 | 662 | 1 419 |
| croissant | map | – | 358 | 1 314 | 1 542 |
| croissant | unordered_map | – | 248 | 81 | 595 |

Tableaux complets dans `resultats/benchmark.txt`. Mesures faites sur un Intel Core i7-8550U, Windows 11,
MinGW g++ 15.2 `-O2` : les valeurs changent selon la machine, les tendances restent.

## Ce qu'on peut déduire

1. **L'équilibrage sauve l'arbre dans l'ordre croissant.** L'ABR devient une liste (hauteur = N) :
   à N = 20 000, il est environ **240 fois plus lent** à l'insertion, **460 fois** à la recherche
   et **90 fois** à la suppression que le rouge-noir, qui reste à une hauteur de 37 pour un million de clés.
2. **En ordre aléatoire, l'ABR tient bien** (hauteur 50) : le rouge-noir cherche **1,3 fois plus vite**,
   insère à peu près aussi vite, mais **supprime environ 13 % plus lentement**, car il doit réparer les couleurs.
3. **Notre rouge-noir cherche environ 2 fois plus vite que `std::map`.** Les deux sont des arbres
   rouge-noir de même hauteur. La différence vient du code machine : notre boucle de recherche
   choisit l'enfant gauche ou droit **sans branchement** (instruction `cmov`), alors que `map::find`
   descend avec un `if` que le processeur prédit mal une fois sur deux ; `map::find` descend aussi
   toujours jusqu'en bas, alors que notre recherche s'arrête dès qu'elle trouve la clé.
   `map` reste un peu plus rapide à l'insertion en ordre croissant.
4. **`unordered_map` est la plus rapide** : environ 5,5 fois plus rapide que le rouge-noir pour chercher,
   car elle va directement dans la bonne case (O(1) en moyenne). Mais elle **ne garde aucun ordre** :
   impossible d'obtenir les clés triées, le minimum ou toutes les clés entre deux valeurs.
5. **Choisir** : `unordered_map` si on veut seulement savoir si une clé existe ; `map` (ou un arbre
   rouge-noir) si on a besoin de l'ordre ; jamais un ABR non équilibré si les données peuvent arriver triées.

Le rapport complet est dans `rapport/rapport_rbt.pdf`.