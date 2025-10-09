#include "PROJECT.h"
#include <stdio.h>
#include <assert.h>

void e2e_test() {
    clear_screen();
    printf("-----------------------------------------------------\n");
    printf("                       E2E TEST\n");
    printf("   (Type 0 at any prompt to go back to main menu)\n");
    printf("-----------------------------------------------------\n\n");
    
    Record arr[MAX_RECORDS];
    int n = load_all(arr, MAX_RECORDS);

    struct {
        const char *inspectionID;
        const char *carReg;
        const char *owner;
        const char *date;
    } test_records[] = {
        {"G001", "ABC5678", "Jane Doe", "01/02/2025"},
        {"G002", "XYZ4321", "John Smith", "20/03/2025"},
        {"G003", "DEF0001", "Alice Lee", "30/04/2025"}
    };
    int expected_count = 3;

    printf("\n[ACT] Add test records\n");
    for (int i = 0; i < expected_count; i++) {
        Record r;
        strcpy(r.inspectionID, test_records[i].inspectionID);
        strcpy(r.carReg, test_records[i].carReg);
        strcpy(r.owner, test_records[i].owner);
        strcpy(r.date, test_records[i].date);
        arr[n++] = r;
        printf("   Added: %s, %s, %s, %s\n", r.inspectionID, r.carReg, r.owner, r.date);
    }
    fake_save_all(arr, n);

    printf("\n[ASSERT] Validate added records and formats\n");
    for (int i = n - expected_count; i < n; i++) {
        Record *r = &arr[i];

        assert_equal_int(is_valid_id(r->inspectionID), 1, "InspectionID format");
        assert_equal_int(is_valid_car_reg(r->carReg), 1, "CarRegNumber format");
        assert_equal_int(is_valid_owner_name(r->owner), 1, "OwnerName format");

        int d, m, y;
        int valid_date = sscanf(r->date, "%d/%d/%d", &d, &m, &y) == 3;
        assert_equal_int(valid_date, 1, "InspectionDate format");

        printf("   Verified Record: %s, %s, %s, %s\n", r->inspectionID, r->carReg, r->owner, r->date);
    }

    /* ---------- SEARCH TEST ---------- */
    printf("\n[SEARCH] Case-insensitive search test\n");
    const char *search_keys[] = {"G001", "g001", "ABC5678", "abc5678"};
    for (int i = 0; i < 4; i++) {
        int idx = find_case_insensitive(arr, n, search_keys[i]);
        char msg[64];
        sprintf(msg, "Search key '%s' should be found", search_keys[i]);
        assert_equal_int(idx != -1, 1, msg);
    }

    /* ---------- UPDATE TEST ---------- */
    printf("\n[UPDATE] Modify all fields of record 'G001'\n");
    int idx = find_case_insensitive(arr, n, "G001");
    assert_equal_int(idx != -1, 1, "Record G001 found for update");
    if (idx != -1) {
        strcpy(arr[idx].inspectionID, "G010");
        strcpy(arr[idx].carReg, "ADC7865");
        strcpy(arr[idx].owner, "Jenny Doe");
        strcpy(arr[idx].date, "02/11/2025");
        fake_save_all(arr, n);
        printf("   Updated record -> %s, %s, %s, %s\n",
               arr[idx].inspectionID, arr[idx].carReg, arr[idx].owner, arr[idx].date);
    }

    idx = find_case_insensitive(arr, n, "G010");
    assert_equal_int(idx != -1, 1, "Updated record G010 found");
    if (idx != -1) {
        Record *r = &arr[idx];
        assert_equal_string(r->inspectionID, "G010", "InspectionID updated");
        assert_equal_string(r->carReg, "ADC7865", "CarRegNumber updated");
        assert_equal_string(r->owner, "Jenny Doe", "OwnerName updated");
        assert_equal_string(r->date, "02/11/2025", "InspectionDate updated");
    }

    /* ---------- DELETE TEST ---------- */
    printf("\n[DELETE] Case-insensitive delete test\n");
    const char *delete_keys[] = {"G010", "g010", "ADC7865", "adc7865"};

    for (int i = 0; i < 4; i++) {
    int idx;
    while ((idx = find_case_insensitive(arr, n, delete_keys[i])) != -1) {
        for (int j = idx; j < n - 1; j++) arr[j] = arr[j + 1];
        n--;
        fake_save_all(arr, n);
        printf("   Record '%s' deleted.\n", delete_keys[i]);
    }
}

    for (int i = 0; i < 4; i++) {
        int idx = find_case_insensitive(arr, n, delete_keys[i]);
        char msg[64];
        sprintf(msg, "Record '%s' deleted successfully", delete_keys[i]);
        assert_equal_int(idx == -1, 1, msg);
    }

    printf("\nE2E Test Completed. Press Enter to return to main menu...");
    while (getchar() != '\n');
}

