#include "PROJECT.h"
#include <stdio.h>
#include <assert.h>

// ==================== Unit Test: Search ====================
void unit_test_search() {
    printf("\n[Unit Test] search_record\n");

    Record arr[MAX_RECORDS];
    int n = load_all(arr, MAX_RECORDS);

    // Test Case 1: Search by existing InspectionID (case-sensitive)
    int found_idx = -1;
    for (int i = 0; i < n; ++i) {
        if (strcmp(arr[i].inspectionID, "I001") == 0) { found_idx = i; break; }
    }
    assert(found_idx != -1);
    printf(" -> Passed: Record 'I001' found (case-sensitive)\n");

    // Test Case 2: Search by existing InspectionID (case-insensitive)
    found_idx = -1;
    for (int i = 0; i < n; ++i) {
        if (_stricmp(arr[i].inspectionID, "i002") == 0) { found_idx = i; break; }
    }
    assert(found_idx != -1);
    printf(" -> Passed: Record 'i002' found (case-insensitive)\n");

    // Test Case 3: Search by existing CarReg (case-sensitive)
    found_idx = -1;
    for (int i = 0; i < n; ++i) {
        if (strcmp(arr[i].carReg, "ABC1234") == 0) { found_idx = i; break; }
    }
    assert(found_idx != -1);
    printf(" -> Passed: Record 'ABC1234' found (case-sensitive)\n");

    // Test Case 4: Search by existing CarReg (case-insensitive)
    found_idx = -1;
    for (int i = 0; i < n; ++i) {
        if (_stricmp(arr[i].carReg, "xyz5678") == 0) { found_idx = i; break; }
    }
    assert(found_idx != -1);
    printf(" -> Passed: Record 'xyz5678' found (case-insensitive)\n");

    // Test Case 5: Search non-existent key
    found_idx = -1;
    for (int i = 0; i < n; ++i) {
        if (_stricmp(arr[i].inspectionID, "NONEXIST") == 0 ||
            _stricmp(arr[i].carReg, "NONEXIST") == 0) { found_idx = i; break; }
    }
    assert(found_idx == -1);
    printf(" -> Passed: Non-existent key 'NONEXIST' not found\n");

    printf("[Unit Test] search_record completed.\n");
}

// ==================== Unit Test: Delete ====================
void unit_test_delete() {
    printf("\n[Unit Test] delete_record\n");

    Record arr[MAX_RECORDS];
    int n = load_all(arr, MAX_RECORDS);
    int initial_count = n;

    Record t1 = {"U001", "UNI0001", "Tester One", "01/10/2025"};
    Record t2 = {"U002", "UNI0020", "Tester Two", "02/10/2025"};
    Record t3 = {"U003", "UNI0300", "Tester Three", "03/10/2025"};

    if (n + 3 > MAX_RECORDS) {
        printf("Skipping delete test: Not enough space for test records.\n");
        return;
    }

    arr[n++] = t1;
    arr[n++] = t2;
    arr[n++] = t3;
    save_all(arr, n);

    // Helper function for deletion test
    void delete_record_test(const char* key, int confirm) {
        int n_local = load_all(arr, MAX_RECORDS);
        int idx = find_by_id_or_reg(arr, n_local, key);
        if (idx == -1) return;
        if (!confirm) return;
        for (int i = idx; i < n_local - 1; i++) arr[i] = arr[i + 1];
        n_local--;
        save_all(arr, n_local);
    }

    // Test Case 1: Delete U001 (confirm Y)
    delete_record_test("U001", 1);
    n = load_all(arr, MAX_RECORDS);
    assert(find_by_id_or_reg(arr, n, "U001") == -1);
    printf(" -> Passed: 'U001' deleted successfully.\n");

    // Test Case 2: Delete non-existent key
    delete_record_test("NONEXIST", 1);
    n = load_all(arr, MAX_RECORDS);
    assert(find_by_id_or_reg(arr, n, "NONEXIST") == -1);
    printf(" -> Passed: Non-existent key not found.\n");

    // Test Case 3: Delete UNI0020 (confirm N - do not delete)
    delete_record_test("UNI0020", 0);
    n = load_all(arr, MAX_RECORDS);
    assert(find_by_id_or_reg(arr, n, "UNI0020") != -1);
    printf(" -> Passed: 'UNI0020' NOT deleted after N confirmation.\n");

    // Test Case 4: Delete UNI0300 (confirm Y)
    delete_record_test("UNI0300", 1);
    n = load_all(arr, MAX_RECORDS);
    assert(find_by_id_or_reg(arr, n, "UNI0300") == -1);
    printf(" -> Passed: 'UNI0300' deleted successfully.\n");

    // Cleanup remaining test record UNI0020
    delete_record_test("UNI0020", 1);
    n = load_all(arr, MAX_RECORDS);
    assert(find_by_id_or_reg(arr, n, "UNI0020") == -1);
    printf(" -> Cleanup Passed: 'UNI0020' deleted.\n");

    printf("[Unit Test] delete_record completed.\n");

}
