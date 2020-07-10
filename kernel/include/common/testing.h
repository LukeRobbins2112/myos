#ifndef _TESTING_H
#define _TESTING_H

#include <stdio.h>

// --------------------------------------
// Test Accounting
// --------------------------------------

extern int TEST_Failures;
extern int TEST_Successes;


// --------------------------------------
// Helpers
// --------------------------------------

#define STRINGIFY_ARG(arg) #arg

#define END_TEST(testname)				      \
  printf("Test results for %s\n", STRINGIFY_ARG(testname));   \
  printf("Passed: %d\n", TEST_Successes);		      \
  printf("Failed: %d\n", TEST_Failures);                      \
  TEST_Failures = 0;                                          \
  TEST_Successes = 0;					      \
  
// --------------------------------------
// Testing Macros
// --------------------------------------

#define ASSERT_EQ(lhs, rhs)				        \
  if (lhs != rhs){					        \
    printf("FAILED: expected was %x, actual was %x", rhs, lhs);	\
    TEST_Failures++;						\
    return;							\
  } else {							\
    TEST_Successes++;						\
  }                                                             \

#define ASSERT_TRUE(statement)				       \
  if (!(statement)){                                           \
    printf("FAILED: (%s) is false", STRINGIFY_ARG(statement)); \
    TEST_Failures++;					       \
  } else {						       \
    TEST_Successes++;					       \
  }							       \

#endif // _TESTING_H
