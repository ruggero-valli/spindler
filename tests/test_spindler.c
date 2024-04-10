#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "spindler.h"

#define ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf("Assertion failed: %s, file: %s, line: %d\n", #condition, __FILE__, __LINE__); \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

#define TOLERANCE 1e-6

void test_spindler_init() {
    struct spindler_data_t spindler_data;
    int result = spindler_init("nonexistent_model", &spindler_data);
    ASSERT(result == SPINDLER_DIR_NOT_FOUND);
    spindler_free_data(&spindler_data); // Freeing allocated resources
    printf("spindler_init test passed.\n");
}

void test_spindler_get_De() {
    struct spindler_data_t spindler_data;

    // Test against "Siwek23"
    spindler_init("Siwek23", &spindler_data);
    double result = spindler_get_De(0.5, 0.5, &spindler_data);
    ASSERT(fabs(result - 0.0) < TOLERANCE);
    spindler_free_data(&spindler_data); // Freeing allocated resources

    // Test against "DD21"
    spindler_init("DD21", &spindler_data);
    result = spindler_get_De(0.5, 0.5, &spindler_data);
    ASSERT(fabs(result - 0.0) < TOLERANCE);
    spindler_free_data(&spindler_data); // Freeing allocated resources

    // Test against "Zrake21"
    spindler_init("Zrake21", &spindler_data);
    result = spindler_get_De(0.5, 0.5, &spindler_data);
    ASSERT(fabs((result - (-5.607))/result) < TOLERANCE);
    spindler_free_data(&spindler_data); // Freeing allocated resources

    printf("spindler_get_De test passed.\n");
}

void test_spindler_get_Dq() {
    struct spindler_data_t spindler_data;

    // Test against "Siwek23"
    spindler_init("Siwek23", &spindler_data);
    double result = spindler_get_Dq(0.5, 0.5, &spindler_data);
    ASSERT(fabs(result - 0.0) < TOLERANCE);
    spindler_free_data(&spindler_data); // Freeing allocated resources

    // Test against "DD21"
    spindler_init("DD21", &spindler_data);
    result = spindler_get_Dq(0.5, 0.5, &spindler_data);
    ASSERT(fabs(result - 0.0) < TOLERANCE);
    spindler_free_data(&spindler_data); // Freeing allocated resources

    // Test against "Zrake21"
    spindler_init("Zrake21", &spindler_data);
    result = spindler_get_Dq(0.5, 0.5, &spindler_data);
    ASSERT(fabs(result - 0.0) < TOLERANCE);
    spindler_free_data(&spindler_data); // Freeing allocated resources

    printf("spindler_get_Dq test passed.\n");
}

void test_spindler_get_Da() {
    struct spindler_data_t spindler_data;

    // Test against "Siwek23"
    spindler_init("Siwek23", &spindler_data);
    double result = spindler_get_Da(0.5, 0.5, &spindler_data);
    ASSERT(fabs(result - 0.0) < TOLERANCE);
    spindler_free_data(&spindler_data); // Freeing allocated resources

    // Test against "DD21"
    spindler_init("DD21", &spindler_data);
    result = spindler_get_Da(0.5, 0.5, &spindler_data);
    ASSERT(fabs(result - 0.0) < TOLERANCE);
    spindler_free_data(&spindler_data); // Freeing allocated resources

    // Test against "Zrake21"
    spindler_init("Zrake21", &spindler_data);
    result = spindler_get_Da(0.5, 0.5, &spindler_data);
    ASSERT(fabs((result - 0.1675)/result) < TOLERANCE);
    spindler_free_data(&spindler_data); // Freeing allocated resources

    printf("spindler_get_Da test passed.\n");
}

void test_spindler_get_DE() {
    struct spindler_data_t spindler_data;

    // Test against "Siwek23"
    spindler_init("Siwek23", &spindler_data);
    double result = spindler_get_DE(0.5, 0.5, &spindler_data);
    ASSERT(fabs(result - 0.0) < TOLERANCE);
    spindler_free_data(&spindler_data); // Freeing allocated resources

    // Test against "DD21"
    spindler_init("DD21", &spindler_data);
    result = spindler_get_DE(0.5, 0.5, &spindler_data);
    ASSERT(fabs(result - 0.0) < TOLERANCE);
    spindler_free_data(&spindler_data); // Freeing allocated resources

    // Test against "Zrake21"
    spindler_init("Zrake21", &spindler_data);
    result = spindler_get_DE(0.5, 0.5, &spindler_data);
    ASSERT(fabs(result - 0.0) < TOLERANCE);
    spindler_free_data(&spindler_data); // Freeing allocated resources

    printf("spindler_get_DE test passed.\n");
}

void test_spindler_get_DJ() {
    struct spindler_data_t spindler_data;

    // Test against "Siwek23"
    spindler_init("Siwek23", &spindler_data);
    double result = spindler_get_DJ(0.5, 0.5, &spindler_data);
    ASSERT(fabs(result - 0.0) < TOLERANCE);
    spindler_free_data(&spindler_data); // Freeing allocated resources

    // Test against "DD21"
    spindler_init("DD21", &spindler_data);
    result = spindler_get_DJ(0.5, 0.5, &spindler_data);
    ASSERT(fabs(result - 0.0) < TOLERANCE);
    spindler_free_data(&spindler_data); // Freeing allocated resources

    // Test against "Zrake21"
    spindler_init("Zrake21", &spindler_data);
    result = spindler_get_DJ(0.5, 0.5, &spindler_data);
    ASSERT(fabs(result - 0.0) < TOLERANCE);
    spindler_free_data(&spindler_data); // Freeing allocated resources

    printf("spindler_get_DJ test passed.\n");
}

int main() {
    test_spindler_init();
    test_spindler_get_De();
    test_spindler_get_Dq();
    test_spindler_get_Da();
    test_spindler_get_DE();
    test_spindler_get_DJ();

    return 0;
}
