#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#include "comet.h"
#include "version.h"

#define BUILD_OUTPUT "_project_build"
#define SRC "src"
#define BUILD "build"
#define TEST "test"
#define LIB "lib"

#define MAIN_C "main.c"
#define BUILD_C "build.c"
#define COMPILER "gcc"
#define LAST_BUILD_PATH "/.comet/last_build"

int help() {
    printf("usage: comet <command> [options]\n\n");
    printf("commands:\n");
    printf("  init            scaffold a new c project\n");
    printf("  build          compile the project using build.c\n");
    printf("  run            build and run the executable\n");
    printf("  test           compile and run tests\n");
    printf("  clean          remove all build artifacts\n");
    printf("\n");
    printf("options:\n");
    printf("  --quiet        suppress output during scaffolding\n");
    printf("  --help, -h     show this help message\n");
    printf("  --version, -v  show version information\n");
    printf("\n");
 
    return 0;
}

int version() {
    printf("%s\n", APP_NAME_AND_VERSION);
    return 0;
}

int build() {
    int built = system(COMPILER " " BUILD_C " -o " BUILD_OUTPUT);
    if (built != 0) {
        fprintf(stderr, "failed to build the build file.");
        return 1;
    }

    int ran = system("./" BUILD_OUTPUT " build");
    remove(BUILD_OUTPUT);

    if (ran != 0) {
        fprintf(stderr, "failed to run the build file.");
        return 1;
    }

    return 0;
}

int clean() {
    system("rm -rf " BUILD);
    system("rm -f " BUILD_OUTPUT);
    system("mkdir " BUILD);

    printf("cleaned build artifacts.\n");

    return 0;
}

int fetch() {
    int built = system(COMPILER " " BUILD_C " -o " BUILD_OUTPUT);
    if (built != 0) {
        fprintf(stderr, "failed to build the build file.");
        return 1;
    }

    int ran = system("./" BUILD_OUTPUT " fetch");
    remove(BUILD_OUTPUT);

    if (ran != 0) {
        fprintf(stderr, "failed to run the build file.");
        return 1;
    }

    return 0;
}

int test() {
    int built = system(COMPILER " " TEST "/" MAIN_C " -o " BUILD "/_test_runner");
    if (built != 0) {
        fprintf(stderr, "failed to compile tests.\n");
        return 1;
    }

    int result = system("./" BUILD "/_test_runner");
    remove(BUILD "/_test_runner");

    if (result != 0) {
        fprintf(stderr, "\ntests failed.\n");
        return 1;
    }

    return 0;
}

int run() {
    int built = build();
    if (built != 0) return 1;

    FILE *f = fopen(BUILD LAST_BUILD_PATH, "r");
    if (!f) {
        fprintf(stderr, "no build found. run 'comet build' first.\n");
        return 1;
    }

    char exe_path[2048] = {0};
    fread(exe_path, 1, sizeof(exe_path) - 1, f);
    fclose(f);

    return system(exe_path);
}

int init() {
    printf("scaffolding project..\n\n");

    if (!create_project_directory(SRC)) return 1;
    if (!create_project_directory(BUILD)) return 1;
    if (!create_project_directory(TEST)) return 1;
    if (!create_project_directory(LIB)) return 1;

    if (!create_project_file(SRC "/" MAIN_C)) return 1;
    if (!setup_main_c(SRC "/" MAIN_C)) return 1;

    if (!create_project_file(TEST "/" MAIN_C)) return 1;
    if (!setup_test_c(TEST "/" MAIN_C)) return 1;

    if (!create_project_file(BUILD_C)) return 1;
    if (!setup_build_c(BUILD_C)) return 1;

    printf("\ncompleted c project scaffold!\n\n");

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return version();
    }

    for (int i = 0; i < argc; i++) {
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
    } else if (strcmp(command, "test") == 0) {
        return test();
    } else if (strcmp(command, "fetch") == 0) {
        return fetch();
    } else if (strcmp(command, "init") == 0) {
        return init();
    } else {
        fprintf(stderr, "unknown command '%s'", command);
        return 1;
    }
    
    return 0;
}