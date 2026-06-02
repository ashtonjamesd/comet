#ifndef comet_h
#define comet_h

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

typedef enum {
    C89,
    C99,
    C11,
    C17,
    C23,
} CStandard;

typedef enum {
    WARN_NONE    = 0,
    WARN_ALL     = 1 << 0,
    WARN_EXTRA   = 1 << 1,
    WARN_PEDANTIC = 1 << 2,
    WARN_ERROR   = 1 << 3,
} WarningFlags;

typedef struct {
    char *repo;
    char *file;
} Dependency;

typedef struct {
    char **srcs;
    size_t srcs_count;
    size_t srcs_capacity;

    Dependency *deps;
    size_t deps_count;
    size_t deps_capacity;

    Compiler compiler;
    OptimizeLevel optimize_level;
    CStandard standard;
    int warnings;
    char *cflags;
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

bool create_project_file(const char *path, ProjectScaffolder ps) {
    if (!ps.quiet) {
        printf("creating file '%s'\n", path);
    }

    return create_file(path);
}

bool fetch_dependency(const char *url, const char *dest) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "curl -sfL \"%s\" -o \"%s\"", url, dest);

    int result = system(cmd);
    if (result != 0) {
        fprintf(stderr, "failed to fetch dependency from '%s'\n", url);
        return false;
    }

    return true;
}

void comet_require(Project *p, char *repo, char *file) {
    if (p->deps_count == p->deps_capacity) {
        p->deps_capacity *= 2;
        p->deps = realloc(p->deps, sizeof(Dependency) * p->deps_capacity);
    }

    p->deps[p->deps_count].repo = repo;
    p->deps[p->deps_count].file = file;
    p->deps_count += 1;
}

bool setup_test_c(char *path) {
FILE *main_file = fopen(path, "w");
    if (!main_file) {
        fprintf(stderr, "failed to setup '%s'", path);
        return false;
    }

    char *content = 
        "#include \"../lib/ctest.h\"\n"
        "\n"
        "should(correctly_add_two_numbers) {\n"
        "   expect(2 + 5 == 7);\n"
        "}\n"
        "\n"
        "int main(void) {\n"
        "   // run a unit test\n"
        "   run_test(correctly_add_two_numbers);\n"
        "   conclude_test_runner();\n\n"
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

Project comet_project() {
    Project p = {
        .srcs = malloc(sizeof(char *)),
        .srcs_count = 0,
        .srcs_capacity = 1,
        .deps = malloc(sizeof(Dependency)),
        .deps_count = 0,
        .deps_capacity = 1,
        .compiler = GCC,
        .exe_name = "output",
    };

    return p;
}

void comet_build_exe_called(Project *p, char *name) {
    p->exe_name = name;
}

void comet_add_source_file(Project *p, char *src) {
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

void comet_use_directory(Project *p, char *root) {
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
        comet_add_source_file(p, path);
    }

    closedir(dir);
}

void comet_optimize_with(Project *p, OptimizeLevel optimize) {
    p->optimize_level = optimize;
}

void comet_standard(Project *p, CStandard standard) {
    p->standard = standard;
}

void comet_warnings(Project *p, int flags) {
    p->warnings = flags;
}

void comet_cflags(Project *p, const char *flags) {
    p->cflags = strdup(flags);
}

void comet_build_with(Project *p, Compiler compiler) {
    p->compiler = compiler;
}

bool comet_fetch_deps(Project *p) {
    if (p->deps_count == 0) return true;

    mkdir("lib", 0700);

    for (size_t i = 0; i < p->deps_count; i++) {
        char url[512];
        snprintf(url, sizeof(url),
            "https://raw.githubusercontent.com/%s/main/%s",
            p->deps[i].repo, p->deps[i].file);

        char dest[512];
        snprintf(dest, sizeof(dest), "lib/%s", p->deps[i].file);

        printf("fetching %s/%s -> %s\n", p->deps[i].repo, p->deps[i].file, dest);

        if (!fetch_dependency(url, dest)) {
            return false;
        }
    }

    return true;
}

static const char *get_standard_name(CStandard standard) {
    switch (standard) {
        case C89: return "-std=c89";
        case C99: return "-std=c99";
        case C11: return "-std=c11";
        case C17: return "-std=c17";
        case C23: return "-std=c23";
        default: return "";
    }
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

void comet_build(Project *p) {
    if (!comet_fetch_deps(p)) {
        fprintf(stderr, "failed to fetch dependencies\n");
        return;
    }

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

    const char *std = get_standard_name(p->standard);
    if (strlen(std) > 0) {
        offset += snprintf(
                cmd + offset, sizeof(cmd) - offset,
                " %s", std
            );
    }

    if (p->warnings & WARN_ALL)
        offset += snprintf(cmd + offset, sizeof(cmd) - offset, " -Wall");
    if (p->warnings & WARN_EXTRA)
        offset += snprintf(cmd + offset, sizeof(cmd) - offset, " -Wextra");
    if (p->warnings & WARN_PEDANTIC)
        offset += snprintf(cmd + offset, sizeof(cmd) - offset, " -pedantic");
    if (p->warnings & WARN_ERROR)
        offset += snprintf(cmd + offset, sizeof(cmd) - offset, " -Werror");

    if (p->deps_count > 0) {
        offset += snprintf(cmd + offset, sizeof(cmd) - offset, " -Ilib");
    }

    if (p->cflags) {
        offset += snprintf(
                cmd + offset, sizeof(cmd) - offset,
                " %s", p->cflags
            );
    }

    system(cmd);

    mkdir("build/.comet", 0700);
    char last_build[2048];
    snprintf(last_build, sizeof(last_build), "%s/%s", cwd, p->exe_name);
    FILE *f = fopen("build/.comet/last_build", "w");
    if (f) {
        fprintf(f, "%s", last_build);
        fclose(f);
    }
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

bool setup_build_c(char *path) {
    FILE *build_file = fopen(path, "w");
    if (!build_file) {
        fprintf(stderr, "failed to setup '%s'", path);
        return false;
    }

    char *content = 
        "#include <comet.h>\n"
        "\n"
        "int comet_build_project(void) {\n"
        "   // initialize project configuration\n"
        "   Project p = comet_project();\n"
        "   // specify a compiler\n"
        "   comet_build_with(&p, GCC);\n"
        "   \n"
        "   // specify a directory to compile sources from\n"
        "   comet_use_directory(&p, \"src\");\n"
        "   // specify the output name and location of the executable\n"
        "   comet_build_exe_called(&p, \"build/exe\");\n"
        "   \n"
        "   // specify compiler flags\n"
        "   comet_cflags(&p, \"-Wall -Wextra -Werror\");\n"
        "   \n"
        "   // build the project\n"
        "   comet_build(&p);\n"
        "   \n"
        "   return 0;\n"
        "}"
        "\n\n"
        "// the entry point to the project builder\n"
        "int main(void) {\n"
        "   comet_build_project();\n"
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