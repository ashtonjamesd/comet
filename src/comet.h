#ifndef comet_h
#define comet_h

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

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

typedef struct Project Project;
typedef int (*CometCommandFunc)(Project *p);

typedef struct {
    const char *name;
    CometCommandFunc func;
} CometCommand;

#define COMET_MAX_COMMANDS 32

struct Project {
    char **srcs;
    size_t srcs_count;
    size_t srcs_capacity;

    Compiler compiler;
    OptimizeLevel optimize_level;
    CStandard standard;
    int warnings;
    char *cflags;
    char *exe_name;

    CometCommand commands[COMET_MAX_COMMANDS];
    size_t command_count;
};

bool make_directory(const char *name) {
    if (mkdir(name, 0700) != -1) {
        return true;
    }

    fprintf(stderr, "failed to create directory '%s'\n", name);

    return false;
}

bool create_project_directory(const char *name) {
    printf("creating directory '%s'\n", name);
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

bool setup_test_c(char *path) {
    FILE *main_file = fopen(path, "w");
    if (!main_file) {
        fprintf(stderr, "failed to setup '%s'", path);
        return false;
    }

    char *content =
        "#include \"../lib/claim.h\"\n"
        "\n"
        "should(correctly_add_two_numbers) {\n"
        "   expect(2 + 5 == 7);\n"
        "}\n"
        "\n"
        "int main(void) {\n"
        "   // run a unit test\n"
        "   run_test(correctly_add_two_numbers);\n"
        "   \n"
        "   return conclude_test_runner();\n"
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

bool setup_gitignore(char *path) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
        "curl -sfL \"https://raw.githubusercontent.com/github/gitignore/main/C.gitignore\" -o \"%s\"",
        path);

    if (system(cmd) != 0) {
        fprintf(stderr, "failed to fetch .gitignore template\n");
        return false;
    }

    FILE *f = fopen(path, "a");
    if (!f) {
        fprintf(stderr, "failed to append to '%s'\n", path);
        return false;
    }

    fprintf(f, "\n# Build output\nbuild/\n_project_build\n");
    fclose(f);

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
        .compiler = GCC,
        .optimize_level = GCC_O0,
        .standard = C89,
        .warnings = WARN_NONE,
        .cflags = NULL,
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

int comet_build(Project *p) {
    char cwd[1024];
    if (!getcwd(cwd, sizeof(cwd))) {
        fprintf(stderr, "failed to get current directory\n");
        return 1;
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

    if (p->cflags) {
        offset += snprintf(
                cmd + offset, sizeof(cmd) - offset,
                " %s", p->cflags
            );
    }

    int result = system(cmd);
    if (result != 0) {
        fprintf(stderr, "failed to build project");
        return 1;
    }

    mkdir("build/.comet", 0700);
    char last_build[2048];
    snprintf(last_build, sizeof(last_build), "%s/%s", cwd, p->exe_name);
    FILE *f = fopen("build/.comet/last_build", "w");
    if (f) {
        fprintf(f, "%s", last_build);
        fclose(f);
    }

    return 0;
}

void comet_command(Project *p, const char *name, CometCommandFunc func) {
    if (p->command_count >= COMET_MAX_COMMANDS) {
        fprintf(stderr, "too many commands registered\n");
        return;
    }
    p->commands[p->command_count].name = name;
    p->commands[p->command_count].func = func;
    p->command_count++;
}

void comet_on_build(Project *p, CometCommandFunc func) {
    comet_command(p, "build", func);
}

void comet_on_fetch(Project *p, CometCommandFunc func) {
    comet_command(p, "fetch", func);
}

int comet_run(Project *p, int argc, char *argv[]) {
    if (argc < 2) return 1;
    
    const char *arg = argv[1];
    for (size_t i = 0; i < p->command_count; i++) {
        if (strcmp(arg, p->commands[i].name) == 0) {
            return p->commands[i].func(p);
        }
    }

    fprintf(stderr, "unknown command '%s'\n", arg);
    return 1;
}

bool comet_fetch_header(char *repo, char *header) {
    const char *filename = strrchr(header, '/');
    filename = filename ? filename + 1 : header;

    char dest[512];
    snprintf(dest, sizeof(dest), "lib/%s", filename);

    if (access(dest, F_OK) == 0) {
        char del_command[512];
        snprintf(del_command, sizeof(del_command), "rm -rf %s", dest);

        if (system(del_command) != 0) {
            fprintf(stderr, "failed to delete and re-fetch dependency");
            return false;
        }
    }

    char url[512];
    snprintf(url, sizeof(url), "https://raw.githubusercontent.com/%s/main/%s", repo, header);

    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "curl -sfL \"%s\" -o \"%s\"", url, dest);

    printf("fetching %s -> %s\n", url, dest);

    if (system(cmd) != 0) {
        fprintf(stderr, "failed to fetch '%s' from '%s'\n", header, repo);
        return false;
    }

    return true;
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
        "// configure the project build settings\n"
        "Project comet_build_project(void) {\n"
        "   // create a new project with default settings\n"
        "   Project p = comet_project();\n"
        "   // set the compiler to use\n"
        "   comet_build_with(&p, GCC);\n"
        "   // add all .c files from a directory\n"
        "   comet_use_directory(&p, \"src\");\n"
        "   // set the output executable path\n"
        "   comet_build_exe_called(&p, \"build/exe\");\n"
        "   // set additional compiler flags\n"
        "   comet_cflags(&p, \"-Wall -Wextra -Werror\");\n"
        "   return p;\n"
        "}\n"
        "\n"
        "// fetch project dependencies into lib/\n"
        "int comet_fetch(Project *p) {\n"
        "   // fetch a single header file from a github repo\n"
        "   if (!comet_fetch_header(\"ashtonjamesd/ctest\", \"ctest.h\")) return 1;\n"
        "   return 0;\n"
        "}\n"
        "\n"
        "int main(int argc, char *argv[]) {\n"
        "   if (argc < 2) return 1;\n"
        "   \n"
        "   Project p = comet_build_project();\n"
        "   \n"
        "   comet_on_build(&p, comet_build);\n"
        "   comet_on_fetch(&p, comet_fetch);\n"
        "   \n"
        "   return comet_run(&p, argc, argv);\n"
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