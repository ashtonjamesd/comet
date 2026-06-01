#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#include "cinit.h"
#include "version.h"

int help() {
    printf("usage: cinit <name> [path]\n");
    printf("  - name: the name of your project\n");
    printf("\n");

    return 0;
}

int version() {
    printf("%s\n", CINIT_VERSION_LONG);
    return 0;
}

int build() {
    int built = system("gcc build.c -o _project_build");
    if (built == -1) {
        fprintf(stderr, "failed to build the build file.");
        return 1;
    }

    int ran = system("./_project_build");
    remove("_project_build");
    if (ran == -1) {
        fprintf(stderr, "failed to run the build file.");
        return 1;
    }

    return 0;
}

int clean() {
    system("rm -rf build");
    system("rm -rf .cinit");
    system("rm -f _project_build");
    system("mkdir build");
    printf("cleaned build artifacts.\n");
    return 0;
}

int run() {
    int built = build();
    if (built != 0) return 1;

    FILE *f = fopen(".cinit/last_build", "r");
    if (!f) {
        fprintf(stderr, "no build found. run 'cinit build' first.\n");
        return 1;
    }

    char exe_path[2048] = {0};
    fread(exe_path, 1, sizeof(exe_path) - 1, f);
    fclose(f);

    return system(exe_path);
}

int new(ProjectScaffolder ps, char *name) {
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: cinit <command>\n");
        return 1;
    }

    bool quiet = false;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--quiet") == 0) {
            quiet = true;
        }

        if (strcmp(argv[i], "help") == 0) return help();
        if (strcmp(argv[i], "--help") == 0) return help();
        if (strcmp(argv[i], "-h") == 0) return help();
        
        if (strcmp(argv[i], "-v") == 0) return version();
        if (strcmp(argv[i], "--version") == 0) return version();
        if (strcmp(argv[i], "version") == 0) return version();
    }

    char *command = argv[1];
    if (strcmp(command, "build") == 0) {
        return build();
    } else if (strcmp(command, "clean") == 0) {
        return clean();
    } else if (strcmp(command, "run") == 0) {
        return run();
    } else if (strcmp(command, "new") == 0) {
        ProjectScaffolder ps = {
            .quiet = quiet,
        };
        
        return new(ps, argv[1]);
    } else {
        fprintf(stderr, "unknown command '%s'", command);
        return 1;
    }
    
    return 0;
}