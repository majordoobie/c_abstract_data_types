#include <check.h>
#include <stdlib.h>
#include <locale.h>

extern Suite * bst_test_suite(void);
extern Suite * tree_map_test_suite(void);
extern Suite * intersect_test_suite(void);
extern Suite * intersect_cli_test_suite(void);

int main(int argc, char ** argv)
{
    setlocale(LC_ALL, "en_US.UTF-8");
    // Suppress unused parameter warnings
    (void) argv[argc];

    // create test suite runner
    SRunner *sr = srunner_create(NULL);

    // prepare the test suites
    srunner_add_suite(sr, bst_test_suite());
    srunner_add_suite(sr, tree_map_test_suite());
    srunner_add_suite(sr, intersect_test_suite());
    srunner_add_suite(sr, intersect_cli_test_suite());

    // run the test suites
    srunner_run_all(sr, CK_VERBOSE);

    // report the test failed status
    int tests_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (tests_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
