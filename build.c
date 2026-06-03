#include <comet.h>

Project comet_build_project(void) {
   Project p = comet_project();

   comet_build_with(&p, GCC);
   
   comet_use_directory(&p, "src");
   comet_build_exe_called(&p, "build/comet");

   comet_cflags(&p, "-Wall -Wextra -Werror");
   comet_standard(&p, C99);
   
   return p;
}

int comet_fetch(Project *p) {
   if (!comet_fetch_header("ashtonjamesd/claim", "claim.h")) return 1;
   return 0;
}

int comet_something(Project *p) {
   printf("test");
   return 0;
}

int main(int argc, char *argv[]) {
   if (argc < 2) return 1;
   
   Project p = comet_build_project();

   comet_on_build(&p, comet_build);
   comet_on_fetch(&p, comet_fetch);

   return comet_run(&p, argc, argv);
}
