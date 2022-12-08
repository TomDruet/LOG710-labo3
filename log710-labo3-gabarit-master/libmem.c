#include "./libmem.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/mman.h>

// IMPORTANT(Alexis Brodeur): Dans ce fichier, et tout code utilisé par ce fichier,
// vous ne pouvez pas utiliser `malloc`, `free`, etc.

static struct {
    void* ptr;
    size_t len;
    mem_strategy_t strategy;
    // TODO(Alexis Brodeur): Ajouter au moins 1 champ pour le *next-fit*.
} state;

// IMPORTANT(Alexis Brodeur): Avant de commencer à implémenter le code de ce
// laboratoire, discuter en équipe afin d'être sûr de tous avoir compris la
// structure de données ce-dessous.

typedef struct block {
    struct block* previous;
    size_t size;
    bool free;
    // NOTE(Alexis Brodeur): Vous pouvez ajouter des champs à cette structure de
    // données, mais vous aller perdre des points pour la qualitée.
} block_t;

/**
 * @brief Retourne le premier bloc dans la liste de blocs.
 *
 * @return Le premier bloc
 */
static inline block_t* block_first()
{
    // IMPORTANT(Alexis Brodeur): Voici un indice !
    return state.ptr;
}

/**
 * @brief Retourne le prochain bloc dans la liste de blocks.
 * @note Retourne @e NULL s'il n'y a pas de prochain bloc.
 *
 * @param block Un bloc
 * @return Le prochain bloc
 */
static block_t* block_next(block_t* block)
{
    // TODO(Alexis Brodeur): À implémenter.

    ((void)block);
    block_t* next_block;
    next_block = ((char*)block) + sizeof(block_t) + block->size;

    if ((char*)next_block < (char*)state.ptr + state.len) {
        return next_block;
    } else {
        return NULL;
    }
}

/**
 * @brief Acquiert un nombre d'octet du bloc dans le cadre d'une allocation de
 * mémoire.
 *
 * @param block Le noeud libre à utiliser
 * @param size La taille de l'allocation
 */
static void block_acquire(block_t* block, size_t size)
{
    assert(block != NULL);
    assert(block->size >= size);
    assert(block->free);

    size_t size_temp = block->size - size - sizeof(block_t);

    block->free = false;
    block->size = size;

    block_t* nouveau_block = ((char*)block) + sizeof(block_t) + block->size;
    nouveau_block->previous = block;
    nouveau_block->free = true;
    nouveau_block->size = size_temp;

    if (block_next(nouveau_block) != NULL) {
        block_next(nouveau_block)->previous = nouveau_block;
    }
}

/**
 * @brief Relâche la mémoire utilisé par une allocation, et fusionne le bloc
 * avec son précédant et suivant lorsque nécessaire.
 *
 * @param block Un bloc à relâcher
 */
static void block_release(block_t* block)
{
    assert(block != NULL);
    assert(!block->free);

    block_t* previous = block->previous;
    block_t* next = block_next(block);

    if (previous != NULL && previous->free) {
        previous->size += sizeof(block_t) + block->size;
        block = previous;

        if (next != NULL) {
            next->previous = block;
        }
    }

    if (next != NULL && next->free) {
        block_t* after_next = block_next(next);
        if (after_next != NULL) {
            after_next->previous = block;
        }

        block->size += sizeof(block_t) + next->size;
    }
    block->free = true;

    // IMPORTANT(Alexis Brodeur):
    // Que faire si le bloc suivant est libre ?
    // Que faire si le bloc précédent est libre ?
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void mem_init(size_t size, mem_strategy_t strategy)
{
    assert(size > 0);
    assert(strategy >= 0);
    assert(strategy < NUM_MEM_STRATEGIES);

    printf("before\n");
    state.ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    printf("after mmap\n");
    if (state.ptr == MAP_FAILED) {
        printf("Mapping Failed\n");
    }
    state.len = size;
    state.strategy = strategy;
    block_t* a_block = block_first();
    a_block->previous = NULL;
    a_block->free = true;
    a_block->size = state.len - sizeof(block_t);
}

void mem_deinit(void)
{
    // TODO(Alexis Brodeur): Libérez la mémoire utilisée par votre gestionnaire.
    int err;
    err = munmap(state.ptr, state.len);
}

void* mem_alloc(size_t size)
{
    assert(size > 0);

    // TODO(Alexis Brodeur): Alloue un bloc de `size` octets.
    //
    // Ce bloc et ses métadonnées doivent être réservées dans la mémoire pointée
    // par `state`..ptr

    // NOTE(Alexis Brodeur): Utiliser la structure `block_t` ci-dessus et les
    // ses fonctions associées.
    //
    // Venez me poser des questions si cela n'est pas clair !
    // switch pour la strategie

    // boucle for pour trouver le bon espace libre

    switch (state.strategy) {
    case MEM_FIRST_FIT: {
        for (block_t* block = block_first(); block != NULL; block = block_next(block)) {
            if (block->free && block->size + sizeof(block_t) >= size) {
                printf("rentrer dans condition. /n");
                block_acquire(block, size);
                return block + 1;
            }
        }
    } break;

    case MEM_BEST_FIT: {

        size_t min_size = state.len;
        block_t* temp = NULL;

        for (block_t* block = block_first(); block != NULL; block = block_next(block)) {
            if (block->free && block->size + sizeof(block_t) >= size && block->size < min_size) {
                temp = block;
                min_size = block->size;
            }
        }

        block_acquire(temp, size);
        return temp + 1;
    } break;

    case MEM_WORST_FIT: {
        size_t max_size = 0;
        block_t* temp_worst = NULL;

        for (block_t* block = block_first(); block != NULL; block = block_next(block)) {
            if (block->free && block->size + sizeof(block_t) >= size && block->size > max_size) {
                temp_worst = block;
                max_size = block->size;
            }
        }

        block_acquire(temp_worst, size);
        return temp_worst + 1;
    } break;

    case MEM_NEXT_FIT: {
        static block_t* next = NULL;
        if (next == NULL) {
            next = block_first();
        }

        block_t* temp_next = next;
        do {
            if (temp_next->free && temp_next->size + sizeof(block_t) >= size) {
                block_acquire(temp_next, size);
                next = block_next(temp_next);
                return temp_next + 1;
            }
            temp_next = block_next(temp_next);
        } while (temp_next != next);

        next = block_first();
        return NULL;
    } break;
    }
}

void mem_free(void* ptr)
{
    assert(ptr != NULL);
    block_t* block = (block_t*)ptr - 1;
    block_release(block);
}

size_t mem_get_free_block_count()
{
    size_t compteur = 0;
    for (block_t* block = block_first(); block != NULL; block = block_next(block)) {
        if (block->free) {
            compteur++;
        }
    }
    return compteur;
}

size_t mem_get_allocated_block_count()
{
    size_t compteur = 0;
    for (block_t* block = block_first(); block != NULL; block = block_next(block)) {
        if (block->free == false) {
            compteur++;
        }
    }
    return compteur;
    return 0;
}

size_t mem_get_free_bytes()
{
    // TODO(Alexis Brodeur): Indiquez combien d'octets sont disponibles pour
    // des allocations de mémoire.

    return 0;
}

size_t mem_get_biggest_free_block_size()
{
    // TODO(Alexis Brodeur): Indiquez la taille en octets du plus gros plus de
    // mémoire libre.

    return 0;
}

size_t mem_count_small_free_blocks(size_t max_bytes)
{
    assert(max_bytes > 0);

    // TODO(Alexis Brodeur): Indiquez combien de blocs de mémoire plus petit que
    // `max_bytes` sont disponible.

    return 0;
}

bool mem_is_allocated(void* ptr)
{
    assert(ptr != NULL);

    // TODO(Alexis Brodeur): Indiquez si l'octet pointé par `ptr` est alloué.

    // NOTE(Alexis Brodeur): Ce pointeur peut pointer vers n'importe quelle
    // adresse mémoire.

    return false;
}

void mem_print_state(void)
{
    // TODO(Alexis Brodeur): Imprimez l'état de votre structure de données.
    //
    //   - Affichez les blocs en ordre.
    //   - Un bloc alloué commence par un 'A', tandis qu'un bloc libre commence
    //     par 'F'.
    //   - Après la lettre, imprimez la taille du bloc.
    //   - Séparez les blocs par un espace.
    //   - Cela ne dérange pas s'il y a un espace supplémentaire à la fin de la
    //     ligne.
    //
    // Ex.:
    //
    // ```
    // A100 F24 A20 A58 F20 A27 F600
    // ```
    for (block_t* block = block_first(); block != NULL; block = block_next(block)) {
        if (block->free) {
            printf("F%zu ", block->size);
        } else {
            printf("A%zu ", block->size);
        }
    }
}

void test1()
{
    printf("1");
    block_acquire(block_first(), 100);
    block_t* nouveau_block = ((char*)state.ptr) + sizeof(block_t) + 100;
    assert(nouveau_block != NULL);
    assert(nouveau_block->free);
    assert(nouveau_block->previous == state.ptr);
    assert(nouveau_block->size == 876);
    assert(block_next(state.ptr) == nouveau_block);
    assert(block_next(block_next(state.ptr)) == NULL);
    assert(block_first()->free == false);
}

void test2()
{
    printf("2");
    block_acquire(state.ptr, 100);
    block_t* nouveau_block = ((char*)state.ptr) + sizeof(block_t) + 100;
    block_release(state.ptr);
    assert(state.ptr != NULL);
    assert(nouveau_block != NULL);
    assert(block_first()->free);
    assert(block_first()->size == 1000);
    assert(block_next(block_first()) == NULL);
}
