# comet

> **Warning:** comet is currently experimental and under active development. It is not recommended for use in production environments. APIs, commands, and behavior may change without notice.
***

<br/>
<br/>

Scaffold, build, and run your C projects from C.

## Install

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/ashtonjamesd/comet/main/install.sh)"
```

## Quick start

```bash
comet init && comet run
```

## Project structure

`comet init` generates the following:

```
├── build.c        # build configuration and dependency declarations
├── .gitignore     # C gitignore template
├── src/
│   └── main.c     # entry point
├── test/
│   └── main.c     # test entry point
├── lib/           # fetched dependencies
└── build/         # compiled output
```

## build.c

Every project has a `build.c` that defines how it compiles and what dependencies it needs.

```c
#include <comet.h>

Project comet_build_project(void) {
   Project p = comet_project();

   comet_build_with(&p, GCC);
   comet_use_directory(&p, "src");

   comet_build_exe_called(&p, "build/exe");
   comet_cflags(&p, "-Wall -Wextra -Werror");

   return p;
}

int comet_fetch(Project *p) {
   if (!comet_fetch_header("ashtonjamesd/claim", "claim.h")) return 1;
   return 0;
}

int main(int argc, char *argv[]) {
   if (argc < 2) return 1;

   Project p = comet_build_project();

   comet_on_build(&p, comet_build);
   comet_on_fetch(&p, comet_fetch);

   return comet_run(&p, argc, argv);
}
```

### Build configuration

| Function | Description |
|---|---|
| `comet_project()` | Create a project with default settings |
| `comet_build_with(&p, GCC)` | Set compiler (`GCC`, `CLANG`) |
| `comet_use_directory(&p, "src")` | Add all `.c` files from a directory |
| `comet_add_source_file(&p, "file.c")` | Add a single source file |
| `comet_build_exe_called(&p, "build/exe")` | Set output executable path |
| `comet_optimize_with(&p, GCC_O2)` | Set optimization level |
| `comet_standard(&p, C17)` | Set C standard (`C89`, `C99`, `C11`, `C17`, `C23`) |
| `comet_warnings(&p, WARN_ALL)` | Set warning flags (combinable with `\|`) |
| `comet_cflags(&p, "-DDEBUG")` | Pass additional compiler flags |

### Dependencies

Declare dependencies in `comet_fetch` to fetch header files from GitHub repos into `lib/`.

```c
comet_fetch_header("user/repo", "path/to/header.h");
```

Running `comet fetch` will always re-fetch all dependencies, replacing any existing files in `lib/`. This ensures your dependencies stay up to date with the latest version from the remote repo.

### Custom commands

You can register custom commands in `build.c` using `comet_command`. These are invoked the same way as built-in commands.

```c
int lint(Project *p) {
   return system("cppcheck src/");
}

int main(int argc, char *argv[]) {
   if (argc < 2) return 1;

   Project p = comet_build_project();

   comet_on_build(&p, comet_build);
   comet_on_fetch(&p, comet_fetch);
   comet_command(&p, "lint", lint);

   return comet_run(&p, argc, argv);
}
```

```bash
comet lint
```

### Passing arguments

Arguments passed after the command name are forwarded to the executable when using `comet run`.

```bash
comet run --verbose --port 8080
```

## Testing

Comet projects come shipped with [claim](https://github.com/ashtonjamesd/claim), a lightweight single-header unit testing framework, fetched automatically into `lib/` during `comet init`. Tests live in `test/main.c` and are run with `comet test`.

## Commands

```
comet init          scaffold a new project
comet build         compile the project using build.c
comet run           build and run the executable
comet test          compile and run tests
comet fetch         fetch dependencies into lib/
comet clean         remove build artifacts

comet <custom>      run a custom command registered in build.c
```

## License

MIT
