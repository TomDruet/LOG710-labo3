# LOG710-01 Laboratoire 3

## Instructions de compilation

Installer les logiciels prérequis:
```sh
sudo apt install build-essential
```

Pour compiler le tout et obtenir les exécutables, faire:
```sh
make
```

## Programme de test

Pour exécuter le programme de test, faire:
```sh
$ make
$ ./Log710Test
```

## Commandes du programme de test

### `ALLOCATE <size>` (raccourci: `A`)

Tente l'allocation d'un bloc de mémoire depuis votre gestionnaire, et affiche
les statistiques et l'état de votre gestionnaire.

`ALLOCATE` vous informe de l'identifiant de l'allocation utilisé par le
programme de test pour retrouver cette allocation lors de la libération.

Dans le cas ou le programme de test a obtenu une allocation du gestionnaire de
mémoire, tous les octets de la mémoire retournée seront remplis par `0xFE`.

### `FREE <id>` (raccourci: `F`)

Libère une allocation de mémoire auprès de votre gestionnaire de mémoire, et
affiche les statististiques et l'état de votre gestionnaire.

### `EXIT` (raccourci: `E`)

Quitte le programme de test.

### `STATE` (raccourci: `S`)

Affiche les statistiquies et l'état de votre gestionnaire.

### `LIST` (raccourci: `L`)

Affiche les allocations gérées par le programme de test de la plus récente à la
plus ancienne.

### `PROBE` (raccourci: `P`)

Indique si l'adresse en mémoire donnée fait partie de la mémoire gérée par votre
gestionnaire de mémoire.

Si l'adresse fait partie de la mémoire allouée, l'octet `0x42` y sera écrit.
