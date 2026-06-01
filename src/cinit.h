#ifndef cinit_h
#define cinit_h

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

typedef struct {
    bool quiet;
} ProjectScaffolder;

typedef enum {
    CLANG,
    GCC,
} Compiler;

typedef enum {
    GCC_O0,
    GCC_O1,
    GCC_O2,
    GCC_O3,
    GCC_Os,
    GCC_Oz,
    GCC_OFast,
    GCC_Og,
} OptimizeLevel;

typedef struct {
    char **srcs;
    size_t srcs_count;
    size_t srcs_capacity;

    Compiler compiler;
    OptimizeLevel optimize_level;
    char *exe_name;
} Project;

bool make_directory(const char *name) {
    if (mkdir(name, 0700) != -1) {
        return true;
    }

    fprintf(stderr, "failed to create directory '%s'\n", name);

    return false;
}

bool create_project_directory(const char *name, ProjectScaffolder ps) {
    if (!ps.quiet) {
        printf("creating directory '%s'\n", name);
    }

    return make_directory(name);
}

bool create_file(const char *path) {
    FILE *fptr = fopen(path, "w");

    if (!fptr) {
        fprintf(stderr, "failed to create file '%s'\n", path);
        return false;
    }

    fclose(fptr);

    return true;
}

bool create_project_file(const char *path) {
    printf("creating file '%s'\n", path);

    return create_file(path);
}

bool setup_main_c(char *path) {
    FILE *main_file = fopen(path, "w");
    if (!main_file) {
        fprintf(stderr, "failed to setup '%s'", path);
        return false;
    }

    char *content = 
        "#include <stdio.h>\n"
        "\n"
        "int main(void) {\n"
        "   printf(\"%s\", \"Hello, World!\\n\");\n"
        "   return 0;\n"
        "}\n";

    fwrite(
        content,
        1,
        strlen(content),
        main_file
    );

    fclose(main_file);

    return true;
}

Project cinit_project() {
    Project p = {
        .srcs = malloc(sizeof(char *)),
        .srcs_count = 0,
        .srcs_capacity = 1,
        .compiler = GCC,
        .exe_name = "output",
    };

    return p;
}

void cinit_build_exe_called(Project *p, char *name) {
    p->exe_name = name;
}

void cinit_add_source_file(Project *p, char *src) {
    if (p->srcs_count == p->srcs_capacity) {
        p->srcs_capacity *= 2;
        
        p->srcs = realloc(
            p->srcs, 
            sizeof(char *) * p->srcs_capacity
        );
    }
    p->srcs[p->srcs_count] = src;
    p->srcs_count += 1;
}

void cinit_use_directory(Project *p, char *root) {
    DIR *dir = opendir(root);
    if (!dir) {
        fprintf(stderr, "failed to open directory '%s'\n", root);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;
        size_t len = strlen(name);

        if (len < 3 || strcmp(name + len - 2, ".c") != 0) {
            continue;
        }

        char *path = malloc(strlen(root) + 1 + len + 1);
        sprintf(path, "%s/%s", root, name);
        cinit_add_source_file(p, path);
    }

    closedir(dir);
}

void cinit_optimize_with(Project *p, OptimizeLevel optimize) {
    p->optimize_level = optimize;
}

void cinit_build_with(Project *p, Compiler compiler) {
    p->compiler = compiler;
}

static char *get_optimize_name(OptimizeLevel optimize) {
    switch (optimize) {
        case GCC_O0: return "-O0";
        case GCC_O1: return "-O1";
        case GCC_O2: return "-O2";
        case GCC_O3: return "-O3";
        case GCC_Os: return "-Os";
        case GCC_Oz: return "-Oz";
        case GCC_OFast: return "-Ofast";
        case GCC_Og: return "-Og";
        default: return "-O0";
    }
}

static char *get_compiler_name(Compiler compiler) {
    switch (compiler) {
        case GCC: return "gcc";
        case CLANG: return "clang";
        default: return "gcc";
    }
}

void cinit_build(Project *p) {
    char cwd[1024];
    if (!getcwd(cwd, sizeof(cwd))) {
        fprintf(stderr, "failed to get current directory\n");
        return;
    }

    char cmd[4096];
    size_t offset = 0;

    offset += snprintf(
            cmd + offset, sizeof(cmd) - offset,
            "%s", get_compiler_name(p->compiler)
        );

    for (size_t i = 0; i < p->srcs_count; i++) {
        offset += snprintf(
                cmd + offset, sizeof(cmd) - offset,
                " %s", p->srcs[i]
            );
    }

    offset += snprintf(
            cmd + offset, sizeof(cmd) - offset,
            " -o %s/%s", cwd, p->exe_name
        );
    
    offset += snprintf(
            cmd + offset, sizeof(cmd) - offset,
            " %s", get_optimize_name(p->optimize_level)
        );

    system(cmd);
}

static char *read_file(char *path) {
    FILE *fptr = fopen((char *)path, "r");
    if (!fptr) {
        fprintf(stderr, "unable to open file: %s\n", path);
        return NULL;
    }

    fseek(fptr, 0, SEEK_END);
    long sz = ftell(fptr);
    if (sz < 0) {
        fclose(fptr);
        return NULL;
    }
    rewind(fptr);

    char *buf = malloc(sz + 1);
    if (!buf) {
        fclose(fptr);
        return NULL;
    }

    size_t amountRead = fread(buf, 1, sz, fptr);
    if ((long)amountRead != sz) {
        fclose(fptr);
        free(buf);
        return NULL;
    }

    buf[sz] = '\0';
    fclose(fptr);

    return buf;
}

bool setup_cinit_c(char *path) {
    FILE *cinit_file = fopen(path, "w");
    if (!cinit_file) {
        fprintf(stderr, "failed to setup '%s'", path);
        return false;
    }

    // char *content = read_file("cinit.h");

    // fwrite(
    //     content,
    //     1,
    //     strlen(content),
    //     cinit_file
    // );

    fclose(cinit_file);

    return true;
}

bool setup_build_c(char *path) {
    FILE *build_file = fopen(path, "w");
    if (!build_file) {
        fprintf(stderr, "failed to setup '%s'", path);
        return false;
    }

    char *content = 
        "#include <cinit.h>\n"
        "\n"
        "void cinit_build_project(void) {\n"
        "   Project p = cinit_project();\n"
        "   cinit_build_exe_called(&p, \"myprogram\");\n"
        "   cinit_build_with(&p, GCC);\n\n"
        "   cinit_optimize_with(&p, GCC_O3);\n"
        "   cinit_use_directory(&p, \"src\");\n\n"
        "   cinit_build(&p);\n"
        "}\n\n"
        "int main(void) {\n"
        "   cinit_build_project();\n"
        "   return 0;\n"
        "}\n";

    fwrite(
        content,
        1,
        strlen(content),
        build_file
    );

    fclose(build_file);

    return true;
}

#endif