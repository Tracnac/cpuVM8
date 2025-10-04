#include "unity/unity.h"

void setUp(void) {}
void tearDown(void) {}

extern void initCPU_test(void);

int main(void) {
    printf("\nStart tests...\n\n");

    UNITY_BEGIN();
    RUN_TEST(initCPU_test);
    return UNITY_END();
}
