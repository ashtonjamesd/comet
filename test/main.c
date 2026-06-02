#include "../lib/ctest.h"
#include "../src/comet.h"

should(set_exe_name) {
   Project p = comet_project();

   comet_build_exe_called(&p, "myapp");
   expect_str_eq(p.exe_name, "myapp");
}

should(set_compiler) {
   Project p = comet_project();

   comet_build_with(&p, CLANG);
   expect(p.compiler == CLANG);
}

should(set_standard) {
   Project p = comet_project();

   comet_standard(&p, C11);
   expect(p.standard == C11);

   comet_standard(&p, C99);
   expect(p.standard == C99);
}

should(set_cflags) {
   Project p = comet_project();

   comet_cflags(&p, "-DDEBUG");
   expect_str_eq(p.cflags, "-DDEBUG");
}

should(set_warnings) {
   Project p = comet_project();
   comet_warnings(&p, WARN_ALL | WARN_EXTRA);
   
   expect(p.warnings & WARN_ALL);
   expect(p.warnings & WARN_EXTRA);
   expect(!(p.warnings & WARN_ERROR));
}

should(grow_sources_on_overflow) {
   Project p = comet_project();
   expect(p.srcs_capacity == 1);

   comet_add_source_file(&p, "a.c");
   comet_add_source_file(&p, "b.c");
   comet_add_source_file(&p, "c.c");

   expect(p.srcs_count == 3);
   expect(p.srcs_capacity >= 3);
}

should(add_source_file) {
   Project p = comet_project();
   comet_add_source_file(&p, "src/main.c");

   expect(p.srcs_count == 1);
   expect_str_eq(p.srcs[0], "src/main.c");
}

should(create_project_with_defaults) {
   Project p = comet_project();

   expect(p.compiler == GCC);
   expect(p.srcs_count == 0);
   expect(p.srcs_capacity == 1);
   expect_null(p.cflags);
   expect_str_eq(p.exe_name, "output");
}

int main(void) {
   run_test(set_exe_name);
   run_test(set_compiler);
   run_test(set_standard);
   run_test(set_cflags);
   run_test(set_warnings);
   run_test(grow_sources_on_overflow);
   run_test(add_source_file);
   run_test(create_project_with_defaults);

   return conclude_test_runner();
}
