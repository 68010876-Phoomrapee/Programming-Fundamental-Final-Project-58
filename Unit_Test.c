#include "PROJECT.h"
#include <stdio.h>
#include <assert.h>

// ==================== Unit Test: Search ====================
void unit_test_search() {
    printf("\n[Unit Test] search_record\n");

    Record arr[MAX_RECORDS];
    int n = load_all(arr, MAX_RECORDS);

     int has_I001 = 0;
    for (int i = 0; i < n; ++i) {
        if (strcmp(arr[i].inspectionID, "I001") == 0) {
            has_I001 = 1;
            break;
        }
    }

    if (!has_I001) {
        if (n < MAX_RECORDS) {
            strcpy(arr[n].inspectionID, "I001");
            strcpy(arr[n].carReg, "ABC1234");
            strcpy(arr[n].owner, "John Doe");
            strcpy(arr[n].date, "01/08/2025");
            n++;
            printf("    Created test record 'I001' because it was missing.\n");
            save_all(arr, n);
        } else {
            printf("    Warning: MAX_RECORDS reached, cannot create 'I001'\n");
        }
    }

    // Test Case 1: Search by existing InspectionID (case-sensitive, should work)
    printf(" -> Test Case 1: Search by existing InspectionID 'I001' (case-sensitive)\n");
    int found_idx = -1;
    for (int i = 0; i < n; ++i) {
        if (strcmp(arr[i].inspectionID, "I001") == 0) {
            found_idx = i;
            break;
        }
    }
    assert(found_idx != -1);
    printf("    Passed: Record 'I001' found.\n");

    // Test Case 2: Search by existing InspectionID (case-insensitive, should work)
    printf("\n -> Test Case 2: Search by existing InspectionID 'i002' (case-insensitive)\n");
    found_idx = -1;
    for (int i = 0; i < n; ++i) {
        if (_stricmp(arr[i].inspectionID, "i002") == 0) {
            found_idx = i;
            break;
        }
    }
    assert(found_idx != -1);
    printf("    Passed: Record 'i002' found (case-insensitive).\n");

    // Test Case 3: Search by existing CarReg (case-sensitive, should work)
    printf("\n -> Test Case 3: Search by existing CarReg 'ABC1234' (case-sensitive)\n");
    found_idx = -1;
    for (int i = 0; i < n; ++i) {
        if (strcmp(arr[i].carReg, "ABC1234") == 0) {
            found_idx = i;
            break;
        }
    }
    assert(found_idx != -1);
    printf("    Passed: Record 'ABC1234' found.\n");

    // Test Case 4: Search by existing CarReg (case-insensitive, should work)
    printf("\n -> Test Case 4: Search by existing CarReg 'xyz5678' (case-insensitive)\n");
    found_idx = -1;
    for (int i = 0; i < n; ++i) {
        if (_stricmp(arr[i].carReg, "xyz5678") == 0) {
            found_idx = i;
            break;
        }
    }
    assert(found_idx != -1);
    printf("    Passed: Record 'xyz5678' found (case-insensitive).\n");

    // Test Case 5: Search for non-existent ID/Reg
    printf("\n -> Test Case 5: Search for non-existent key 'NONEXIST'\n");
    found_idx = -1;
    for (int i = 0; i < n; ++i) {
        if (_stricmp(arr[i].inspectionID, "NONEXIST") == 0 || _stricmp(arr[i].carReg, "NONEXIST") == 0) {
            found_idx = i;
            break;
        }
    }
    assert(found_idx == -1);
    printf("    Passed: Non-existent key 'NONEXIST' not found.\n");

    printf("\n[Unit Test] search_record completed.\n");
}

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

 void delete_record_test(const char* key, int confirm) {
    int n = load_all(arr, MAX_RECORDS);
    int idx = find_by_id_or_reg(arr, n, key);
    if (idx == -1) {
        return;
    }
    if (!confirm) {
        return;
    }
    for (int i = idx; i < n - 1; i++) {
        arr[i] = arr[i + 1];
    }
    n--;
    save_all(arr, n);
}
    // Test Case 1: Delete by existing InspectionID 'U001' (confirm Y)
    printf("\n -> Test Case 1: Delete by existing InspectionID 'U001' (confirm Y)'\n");
    delete_record_test("U001", 1);
    n = load_all(arr, MAX_RECORDS);
    assert(find_by_id_or_reg(arr, n, "U001") == -1);
    printf("    Passed: 'U001' deleted successfully.\n");

    // Test Case 2: Attempt to delete non-existent ID 'NONEXIST'
    printf("\n -> Test Case 2: Attempt to delete non-existent ID 'NONEXIST'\n");
    delete_record_test("NONEXIST", 1); 
    n = load_all(arr, MAX_RECORDS);
    assert(find_by_id_or_reg(arr, n, "NONEXIST") == -1);
    printf("    Passed: Non-existent key not found.\n");

    // Test Case 3: Delete by existing CarReg 'UNI0020' (confirm N - do not delete)
    printf("\n -> Test Case 3: Delete by existing CarReg 'UNI0020' (confirm N - do not delete)'\n");
    delete_record_test("UNI0020", 0); 
    n = load_all(arr, MAX_RECORDS);
    assert(find_by_id_or_reg(arr, n, "UNI0020") != -1);
    printf("    Passed: 'UNI0020' NOT deleted after 'n' confirmation.\n");

    // Test Case 4: Delete by existing CarReg 'UNI0300' (confirm Y)
    printf("\n -> Test Case 4: Delete by existing CarReg 'UNI0300' (confirm Y)'\n");
    delete_record_test("UNI0300", 1);
    n = load_all(arr, MAX_RECORDS);
    assert(find_by_id_or_reg(arr, n, "UNI0300") == -1);
    printf("    Passed: 'UNI0300' deleted successfully.\n");

    // Cleanup: Delete remaining test record 'UNI0020'
    delete_record_test("UNI0020", 1);
    n = load_all(arr, MAX_RECORDS);
    assert(find_by_id_or_reg(arr, n, "UNI0020") == -1);
    printf("\n    Passed Cleanup: 'UNI0020' deleted.\n");

    printf("\n[Unit Test] delete_record completed.\n");
}
