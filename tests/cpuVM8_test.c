#include "unity/unity.h"

void setUp(void) {}
void tearDown(void) {}

extern void initCPU_test(void);
extern void LDA_test(void);
extern void STA_test(void);
extern void NOP_test(void);
extern void LDX_test(void);
extern void STX_test(void);
extern void ADD_test(void);
extern void SUB_test(void);
extern void XOR_test(void);
extern void AND_test(void);
extern void OR_test(void);
extern void B_test(void);
extern void POP_test(void);
extern void PUSH_test(void);
extern void CMP_test(void);
extern void CPX_test(void);
extern void edge_cases_test(void);

int main(void) {
    printf("\nStart tests...\n\n");

    UNITY_BEGIN();
    RUN_TEST(initCPU_test);
    RUN_TEST(LDA_test);
    RUN_TEST(STA_test);
    RUN_TEST(NOP_test);
    RUN_TEST(LDX_test);
    RUN_TEST(STX_test);
    RUN_TEST(ADD_test);
    RUN_TEST(SUB_test);
    RUN_TEST(XOR_test);
    RUN_TEST(AND_test);
    RUN_TEST(OR_test);
    RUN_TEST(B_test);
    RUN_TEST(POP_test);
    RUN_TEST(PUSH_test);
    RUN_TEST(CMP_test);
    RUN_TEST(CPX_test);
    RUN_TEST(edge_cases_test);
    return UNITY_END();
}
