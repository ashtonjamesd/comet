#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#include "cinit.h"

int help() {
    printf("usage: cinit <name> [path]\n");
    printf("  - name: the name of your project\n");
    printf("\n");

    return 0;
}

int build() {
    int built = system("gcc build.c -o build");
    if (!built) {
        fprintf(stderr, "failed to build the build file.");
        return 1;
    }

    int ran = system("./build");
    if (!ran) {
        fprintf(stderr, "failed to run the build file.");
        return 1;
    }

    return 0;
}

int run() {
    int built = build();
    if (!built) return 1;

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: cinit <command>\n");
        return 1;
    }

    char *command = argv[1];
    if (strcmp(command, "build") == 0) {
        return build();
    }

    if (strcmp(command, "run") == 0) {
        return run();
    }

    bool quiet = false;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--quiet") == 0) {
            quiet = true;
        }

        if (strcmp(argv[i], "help") == 0) return help();
        if (strcmp(argv[i], "--help") == 0) return help();
        if (strcmp(argv[i], "-h") == 0) return help();
    }

    ProjectScaffolder ps = {
        .quiet = quiet,
    };

    char *name = argv[1];

    #define SRC "src"
    #define BUILD "build"
    #define TEST "test"
    #define LIB "lib"

    #define MAIN_C "main.c"
    #define BUILD_C "build.c"
    #define CINIT_C "cinit.h"

    printf("scaffolding project..\n\n");

    if (!create_project_directory(SRC, ps)) return 1;
    if (!create_project_directory(BUILD, ps)) return 1;
    if (!create_project_directory(TEST, ps)) return 1;
    if (!create_project_directory(LIB, ps)) return 1;

    if (!create_project_file(SRC "/" MAIN_C)) return 1;
    if (!setup_main_c(SRC "/" MAIN_C)) return 1;

    if (!create_project_file(TEST "/" MAIN_C)) return 1;
    if (!setup_main_c(TEST "/" MAIN_C)) return 1;

    if (!create_project_file(BUILD_C)) return 1;
    if (!setup_build_c(BUILD_C)) return 1;

    if (!create_project_file(LIB "/" CINIT_C)) return 1;
    if (!setup_cinit_c(LIB "/" CINIT_C)) return 1;
    
    printf("\ncompleted c project scaffold!\n\n");
    
    return 0;
}