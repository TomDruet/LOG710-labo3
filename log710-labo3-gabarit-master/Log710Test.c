#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "libmem.h"

#define DEFAULT_SIZE 1024
#define ALLOCATE_BYTE 0xFE
#define PROBE_BYTE 0x42
#define COUNT_SMALL_SIZE 16

#define WARN(fmt, ...)                                                                 \
    do {                                                                               \
        (void)fprintf(stderr, "[%s:%u] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (false)

#define ERROR(fmt, ...)           \
    do {                          \
        WARN(fmt, ##__VA_ARGS__); \
        exit(EXIT_FAILURE);       \
    } while (false)

struct {
    mem_strategy_t strategy;
    size_t size;
} options = {
    .strategy = MEM_FIRST_FIT,
    .size = DEFAULT_SIZE,
};

static void parse_options(int argc, char** argv);
static void print_state();

typedef enum {
    CONTINUE,
    CONTINUE_WITH_STATE,
    EXIT,
} continue_t;

typedef continue_t(command_t)(int argc, char** argv);

static continue_t handle_command(int argc, char** argv);
static continue_t handle_allocate(int argc, char** argv);
static continue_t handle_free(int argc, char** argv);
static continue_t handle_exit();
static continue_t handle_state();
static continue_t handle_list(int argc, char** argv);
static continue_t handle_probe(int argc, char** argv);
static continue_t handle_test();

int main(int argc, char** argv)
{
    parse_options(argc, argv);

    mem_init(options.size, options.strategy);

    char* line;
    while ((line = readline("Log710Test> ")) != NULL) {
        HIST_ENTRY* entry = current_history();
        if (entry == NULL || strcmp(entry->line, line) != 0) {
            add_history(line);
        }

        int argc = 0;
        for (char* it = strtok(line, " "); it != NULL; it = strtok(NULL, " ")) {
            argc++;
        }

        char** argv = malloc(sizeof(*argv) * (argc + 1));
        if (argv == NULL) {
            mem_deinit();
            ERROR("failed to allocate argv");
        }

        char* arg = line;
        for (int i = 0; i < argc; ++i) {
            argv[i] = arg;
            arg += strlen(arg) + 1;
        }

        continue_t result = handle_command(argc, argv);
        if (result == CONTINUE_WITH_STATE) {
            print_state();
        }

        free(argv);
        free(line);

        if (result == EXIT) {
            break;
        }
    }

    mem_deinit();

    return 0;
}

static continue_t handle_command(int argc, char** argv)
{
    typedef struct string_to_command {
        const char* string;
        command_t* command;
    } string_to_command_t;

    static const string_to_command_t commands[] = {
        { "ALLOCATE", handle_allocate },
        { "A", handle_allocate },
        { "FREE", handle_free },
        { "F", handle_free },
        { "EXIT", handle_exit },
        { "E", handle_exit },
        { "STATE", handle_state },
        { "S", handle_state },
        { "LIST", handle_list },
        { "L", handle_list },
        { "PROBE", handle_probe },
        { "P", handle_probe },
        { "T", handle_test},
        { NULL, NULL }
    };

    if (argc < 1) {
        return CONTINUE;
    }

    for (const string_to_command_t* it = commands; it->string != NULL; it++) {
        if (strcasecmp(it->string, argv[0]) == 0) {
            return it->command(argc, argv);
        }
    }

    printf("command invalide: %s\n", argv[0]);
    return CONTINUE;
}

typedef struct allocation {
    size_t id;
    struct allocation* next;
    void* ptr;
    size_t size;
} allocation_t;

static size_t allocation_id_sequence = 0;
static allocation_t* allocations = NULL;

static continue_t handle_allocate(int argc, char** argv)
{
    bool usage = false;

    if (argc != 2) {
        usage = true;
    }

    // NOLINTNEXTLINE(cert-err34-c,clang-analyzer-core.CallAndMessage)
    long size = atol(argv[1]);
    if (size <= 0) {
        usage = true;
    }

    if (usage) {
        printf(
            "UTILISATION:\n"
            "\t%s <n>\n"
            "\n"
            "ARGUMENTS:\n"
            "\n"
            "\t<n> - La taille de l'allocation en octets.\n",
            argv[0]);
        return CONTINUE;
    }

    void* ptr = mem_alloc(size);
    if (ptr == NULL) {
        puts("impossible d'allouer plus de mémoire");

        return CONTINUE;
    }

    allocation_t* allocation = malloc(sizeof(*allocation));
    if (allocation == NULL) {
        ERROR("failed to allocate memory for persisting allocation");
    }

    memset(ptr, ALLOCATE_BYTE, size);

    allocation->id = ++allocation_id_sequence;
    allocation->ptr = ptr;
    allocation->next = allocations;
    allocation->size = size;
    allocations = allocation;

    printf("ALLOCATION: %zu\n", allocation->id);

    return CONTINUE_WITH_STATE;
}

static continue_t handle_free(int argc, char** argv)
{
    bool usage = false;

    if (argc != 2) {
        usage = true;
    }

    // NOLINTNEXTLINE(cert-err34-c,clang-analyzer-core.CallAndMessage)
    long identifier = atol(argv[1]);
    if (identifier <= 0) {
        usage = true;
    }

    if (usage) {
        printf(
            "UTILISATION:\n"
            "\t%s <i>\n"
            "\n"
            "ARGUMENTS:\n"
            "\n"
            "\t<i> - L'identifiant de l'allocation à libérer.\n",
            argv[0]);
        return CONTINUE;
    }

    allocation_t* current = allocations;
    allocation_t* previous = NULL;

    while (current != NULL) {
        if (current->id == (size_t)identifier) {
            mem_free(current->ptr);
            allocation_t* next = current->next;
            free(current);

            if (previous == NULL) {
                allocations = next;
            } else {
                previous->next = next;
            }

            return CONTINUE_WITH_STATE;
        }

        previous = current;
        current = current->next;
    }

    printf("aucune allocation avec l'identifiant: %zu\n", identifier);
    return CONTINUE;
}

static continue_t handle_test()
{
    test1();
    return CONTINUE;
}

static continue_t handle_exit()
{
    return EXIT;
}

static continue_t handle_state()
{
    return CONTINUE_WITH_STATE;
}

static continue_t handle_list(int argc, char** argv)
{
    bool usage = false;

    if (argc != 1) {
        usage = true;
    }

    if (usage) {
        printf(
            "UTILISATION:\n"
            "\t%s\n",
            argv[0]);
        return CONTINUE;
    }

    for (allocation_t* allocation = allocations; allocation != NULL; allocation = allocation->next) {
        void* begin = allocation->ptr;
        void* end = (char*)begin + allocation->size;

        printf("[%zu] %p .. %p (size = %zu)\n", allocation->id, begin, end, allocation->size);
    }

    return CONTINUE;
}

static continue_t handle_probe(int argc, char** argv)
{
    bool usage = false;

    if (argc != 2) {
        usage = true;
    }

    char* arg1 = argv[1];
    char* arg1_end;
    void* pointer = (void*)strtoull(arg1, &arg1_end, 0);

    if (*arg1_end != '\0') {
        pointer = NULL;
    }

    if (pointer == NULL) {
        usage = true;
    }

    if (usage) {
        printf(
            "UTILISATION:\n"
            "\t%s <ptr>\n"
            "\n"
            "ARGUMENTS:\n"
            "\n"
            "\t<ptr> - L'adressse en mémoire pour laquelle tester si elle fait\n"
            "\t        partie de la mémoire alloué par le gestionnaire.",
            argv[0]);
        return CONTINUE;
    }

    if (mem_is_allocated(pointer)) {
        // Probe.
        //
        // Ici, je m'assure que vous me dites la véritée.
        *((char*)pointer) = PROBE_BYTE;
        puts("VRAI");
    } else {
        puts("FAUX");
    }

    return CONTINUE;
}

static void parse_options(int argc, char** argv)
{
    static const char* shortopts = ":s:n:h";
    static const struct option longopts[] = {
        { "strategy", required_argument, NULL, 's' },
        { "size", required_argument, NULL, 'n' },
        { "help", no_argument, NULL, 'h' },
        { NULL, 0, NULL, 0 }
    };

    typedef struct string_to_strategy {
        const char* string;
        mem_strategy_t strategy;
    } string_to_strategy_t;

    static const string_to_strategy_t strategies[] = {
        { "first-fit", MEM_FIRST_FIT },
        { "first", MEM_FIRST_FIT },
        { "f", MEM_FIRST_FIT },
        { "best-fit", MEM_BEST_FIT },
        { "best", MEM_BEST_FIT },
        { "b", MEM_BEST_FIT },
        { "worst-fit", MEM_WORST_FIT },
        { "worst", MEM_WORST_FIT },
        { "w", MEM_WORST_FIT },
        { "next-fit", MEM_NEXT_FIT },
        { "next", MEM_NEXT_FIT },
        { "n", MEM_NEXT_FIT },
        { NULL, 0 },
    };

    bool usage = false;

    while (true) {
        int code = getopt_long(argc, argv, shortopts, longopts, NULL);

        if (code == -1) {
            break;
        }

        switch (code) {
        case 's': {
            const string_to_strategy_t* strategy_it = strategies;

            while (strategy_it->string != NULL) {
                if (strcasecmp(strategy_it->string, optarg) == 0) {
                    break;
                }

                strategy_it++;
            }

            if (strategy_it->string == NULL) {
                usage = true;
            } else {
                options.strategy = strategy_it->strategy;
            }

            break;
        }
        case 'n': {
            // NOLINTNEXTLINE(cert-err34-c,clang-analyzer-core.CallAndMessage)
            long size = atol(optarg);

            if (size <= 0) {
                usage = true;
            } else {
                options.size = size;
            }

            break;
        }

        case 'h':
        case '?':
        case ':':
            usage = true;

            break;
        default:
            WARN("getopt_long returned an unknown character code: %c", code);
            exit(EXIT_FAILURE);
            return;
        }
    }

    if (usage) {
        printf(
            "UTILISATION:\n"
            "\n"
            "\t%s [--size n] [--strategy first-fit|best-fit|worst-fit|next-fit] [--help]\n"
            "\n"
            "DESCRIPTION:\n"
            "\n"
            "\tLance le programme de test de votre allocateur de mémoire consistant en un REPL.\n"
            "\n"
            "OPTIONS:\n"
            "\n"
            "\t--size <n>\n"
            "\t\tIndique le nombre d'octets que sera géré par votre gestionnaire de mémoire.\n"
            "\n"
            "\t--strategy first-fit|best-fit|worst-fit|next-fit\n"
            "\t\tIndique la stratégie d'allocation à utiliser.\n"
            "\t\tLa valeur par défaut est \"first-fit\".\n"
            "\n"
            "\t--help\n"
            "\t\tAffiche ce message informatif.\n",
            argv[0]);
        exit(EXIT_FAILURE);
    }
}

static void print_state()
{
    printf("# mem_get_free_block_count()        == %zu\n", mem_get_free_block_count());
    printf("# mem_get_allocated_block_count()   == %zu\n", mem_get_allocated_block_count());
    printf("# mem_get_free_bytes()              == %zu\n", mem_get_free_bytes());
    printf("# mem_get_biggest_free_block_size() == %zu\n", mem_get_biggest_free_block_size());
    printf("# mem_count_small_free_blocks(16)   == %zu\n", mem_count_small_free_blocks(COUNT_SMALL_SIZE));
    printf("# mem_print_state()                 => ");
    mem_print_state();
    printf("\n");
}
