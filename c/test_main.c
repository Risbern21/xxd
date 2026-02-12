#include <stddef.h>
#include <stdio.h>
#include <string.h>
#define DOING_UNIT_TESTS
#include "main.c"
#include <CUnit/Basic.h>
#include <CUnit/CUError.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>

static FILE *temp_in_file = NULL;
static FILE *temp_out_file = NULL;

int init_b_to_h_suite1(void) { return 0; }
int clean_b_to_h_suite1(void) { return 0; }

int init_b_to_h_suite2(void) {
  if (NULL == (temp_in_file = fopen("temp1.txt", "w+")) ||
      NULL == (temp_out_file = fopen("temp2.txt", "w+"))) {
    return -1;
  } else {
    // write data to temp_in_file
    fprintf(temp_in_file, "HELLO");
    rewind(temp_in_file);
    return 0;
  }
}
int clean_b_to_h_suite2(void) {
  if (0 != fclose(temp_in_file) || 0 != fclose(temp_out_file)) {
    return -1;
  } else {
    temp_in_file = NULL;
    return 0;
  }
}

int init_h_to_b_suite1(void) { return 0; }
int clean_h_to_b_suite1(void) { return 0; }

int init_h_to_b_suite2(void) {
  if (NULL == (temp_in_file = fopen("temp.hex", "w+"))) {
    return -1;
  } else {
    return 0;
  }
}
int clean_h_to_b_suite2(void) {
  if (0 != fclose(temp_in_file)) {
    return -1;
  } else {
    temp_in_file = NULL;
    return 0;
  }
}

void test_line_len(void) {
  struct test {
    const char *input;
    int output;
  };

  struct test tests[] = {
      {"HELLO WORLD", 11},
      {"TEST 2", 6},
      {"TEST 3121 defwew\n", 17},
      {"\n\n\nTEST 3121 defwew\n", 20},
  };

  size_t num_tests = sizeof(tests) / sizeof(tests[0]);

  for (size_t i = 0; i < num_tests; i++) {
    CU_ASSERT_EQUAL(line_len((byte *)tests[i].input), tests[i].output)
  }

  CU_ASSERT_NOT_EQUAL(line_len((byte *)"nhilistic penguin"), 19);
}

void test_hex_dump(void) {
  hex_dump(temp_in_file, temp_out_file);
  rewind(temp_out_file);

  char buf[2048];
  int n = fread(buf, 1, sizeof(buf) - 1, temp_out_file);
  buf[n] = '\0';

  const char *expected =
      "00000000: 4845 4c4c 4f                                  HELLO";

  CU_ASSERT_STRING_EQUAL(buf, expected);
}

void test_hex_to_int(void) {
  CU_ASSERT_EQUAL(hex_to_int('1'), 1);
  CU_ASSERT_EQUAL(hex_to_int('a'), 10);

  CU_ASSERT_NOT_EQUAL(hex_to_int('b'), 12);
}

void test_hex_to_ascii(void) {
  CU_ASSERT_EQUAL(hex_to_ascii('4', '8'), 'H');
  CU_ASSERT_EQUAL(hex_to_ascii('6', 'c'), 'l');
  CU_ASSERT_EQUAL(hex_to_ascii('0', 'a'), '\n');

  CU_ASSERT_NOT_EQUAL(hex_to_ascii('6', 'c'), '\n');
}

void test_bin_dump(void) {}

int main(void) {
  CU_pSuite h_to_b_Suite1 = NULL;
  CU_pSuite h_to_b_Suite2 = NULL;
  CU_pSuite b_to_h_Suite1 = NULL;
  CU_pSuite b_to_h_Suite2 = NULL;

  if (CUE_SUCCESS != CU_initialize_registry()) {
    return CU_get_error();
  }

  b_to_h_Suite1 = CU_add_suite("binary to hexa suite1", init_b_to_h_suite1,
                               clean_b_to_h_suite1);
  b_to_h_Suite2 = CU_add_suite("binary to hexa suite2", init_b_to_h_suite2,
                               clean_b_to_h_suite2);
  h_to_b_Suite1 = CU_add_suite("hexa to binary suite1:", init_h_to_b_suite1,
                               init_h_to_b_suite1);
  h_to_b_Suite2 = CU_add_suite("hexa to binary suite2:", init_h_to_b_suite2,
                               init_h_to_b_suite2);

  if (NULL == b_to_h_Suite1 || NULL == b_to_h_Suite2 || NULL == h_to_b_Suite1 ||
      NULL == h_to_b_Suite2) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  /* add the tests to the suite */
  if ((NULL ==
       CU_add_test(b_to_h_Suite1, "test of line_len()", test_line_len)) ||
      (NULL ==
       CU_add_test(b_to_h_Suite2, "test for hex_dump()", test_hex_dump)) ||
      (NULL ==
       CU_add_test(h_to_b_Suite1, "test of hex_to_int()", test_hex_to_int)) ||
      (NULL == CU_add_test(h_to_b_Suite1, "test of hex_to_ascii()",
                           test_hex_to_ascii)) ||
      (NULL ==
       CU_add_test(h_to_b_Suite2, "test for bin_dump()", test_bin_dump))) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  // run tests
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();

  return CU_get_error();
}
