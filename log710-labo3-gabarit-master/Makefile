# Définition de variables qui seront réutilisées plus tard.
CC                 ?= gcc
CPPFLAGS           +=
CFLAGS             += -Wall -Wextra -Werror=vla -Werror=alloca -Werror=main -std=gnu11 -ggdb
LDFLAGS            +=

# En utilisant:
#
#     $ make clean all EYEBLEACH=1
#
# D'autres avertissements seront activés dans GCC.
ifdef EYEBLEACH
CFLAGS             += -Wpedantic
endif

# En utilisant:
#
#     $ make clean all IMPORTANT=1
#
# Certains avertissements seront convertit en erreurs, ce qui peut vous aider à
# trouver des problèmes dans votre code.
ifdef IMPORTANT
CFLAGS             += -Werror=uninitialized -Werror=format \
                      -Werror=implicit-function-declaration -Werror=format-overflow \
					  -Werror=misleading-indentation -Werror=return-type \
					  -Werror=maybe-uninitialized -Werror=conversion \
					  -Werror=sign-compare -Werror=address
endif

# Ne pas utiliser si vous ne savez pas ce que vous faites.
#
#     $ make clean all ASAN=1
#
# Active les outils *asan*, et *ubsan* de Google:
#     https://github.com/google/sanitizers
ifdef ASAN
CFLAGS             += -fsanitize=address -fsanitize=undefined
endif

# Ne pas utiliser si vous ne savez pas ce que vous faites.
#
#     $ make clean all LEAKSAN=1
#
# Active l'outil *leaksan* de Google:
#     https://github.com/google/sanitizers
ifdef LEAKSAN
LDFLAGS            += -fsanitize=leak
endif

# La première règle apparaissant dans le GNUMakefile est la règle par défaut
# lorsque le programme `make` est appelé sans arguments.
.PHONY: all
all: libmem.so Log710Test

.PHONY: clean
.SILENT: clean
clean:
	rm -f libmem.so
	rm -f Log710Test

.PHONY: test
.SILENT: test
test: Log710Test
	./Log710Test

libmem.so: libmem.h libmem.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -shared -Wl,-soname,libmem.so -fPIC -o $@ $^

# Indique comment construire la commande `Log710Test`.
Log710Test: Log710Test.c libmem.so
	$(CC) $(CPPFLAGS) $(CFLAGS) $(shell pkg-config --cflags readline) -Wl,-rpath='$${ORIGIN}' -o $@ Log710Test.c -L. -lmem $(shell pkg-config --libs readline)
