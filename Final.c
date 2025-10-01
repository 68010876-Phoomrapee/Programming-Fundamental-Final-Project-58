#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

// Named Constants for limits and file
#define MAX_RECORDS 1000
#define MAX_LINE 512
#define CSV_FILE "users_data.csv"

// Named Constants for field lengths (including null terminator)
#define ID_REG_MAX_LEN 20
#define ID_REG_BUFFER_LEN (ID_REG_MAX_LEN + 1) // For strings like InspectionID, CarReg
#define OWNER_MAX_LEN 60
#define OWNER_BUFFER_LEN (OWNER_MAX_LEN + 1) // For OwnerName
#define DATE_MAX_LEN 10 // DD/MM/YYYY
#define DATE_BUFFER_LEN (DATE_MAX_LEN + 1) // For Date (DD/MM/YYYY)
#define INPUT_BUFFER_SIZE 128 // General buffer for user input

// Date validation constants
#define MIN_YEAR 1880
#define MAX_YEAR 2100
#define MONTHS_IN_YEAR 12
#define DAYS_IN_FEBRUARY_LEAP 29
#define DAYS_IN_FEBRUARY_COMMON 28
#define DAYS_IN_MONTH_30 30
#define DAYS_IN_MONTH_31 31

// Menu options
#define MENU_ADD_RECORD 1
#define MENU_SEARCH_RECORD 2
#define MENU_UPDATE_RECORD 3
#define MENU_DELETE_RECORD 4
#define MENU_DISPLAY_ALL 5
#define MENU_UNIT_TESTS 6
#define MENU_E2E_TEST 7
#define MENU_EXIT 8
#define MENU_BACK 0

// Record structure (4 columns)
typedef struct {
    char inspectionID[ID_REG_BUFFER_LEN];
    char carReg[ID_REG_BUFFER_LEN];
    char owner[OWNER_BUFFER_LEN];
    char date[DATE_BUFFER_LEN]; // DD/MM/YYYY
} Record;

/* ---------- Validation helpers ---------- */

// Convert string to lowercase for case-insensitive comparison
void to_lower_str(char *str) {
    for (char *p = str; *p; ++p) {
        *p = tolower((unsigned char)*p);
    }
}

// return 1 if character allowed in ID/Reg (letters and digits)
int is_alnum_char(char c) {
    return isalpha((unsigned char)c) || isdigit((unsigned char)c);
}

// validate InspectionID or CarRegNumber: allowed letters A-Z a-z and digits 0-9 only,
// and length between 1 and ID_REG_MAX_LEN
int is_valid_id_or_reg(const char *s) {
    if (!s || !s[0]) return 0;
    size_t n = strlen(s);
    if (n > ID_REG_MAX_LEN) return 0;
    for (size_t i = 0; i < n; ++i) {
        if (!is_alnum_char(s[i])) return 0;
    }
    return 1;
}

// OwnerName: letters and spaces only (no digits, no punctuation)
int is_valid_owner_name(const char *s) {
    if (!s) return 0;
    size_t n = strlen(s);
    if (n == 0 || n > OWNER_MAX_LEN) return 0;
    int has_letter = 0;
    for (size_t i = 0; i < n; ++i) {
        char c = s[i];
        if (isalpha((unsigned char)c)) has_letter = 1;
        else if (c == ' ') continue;
        else return 0;
    }
    return has_letter;
}

// check leap year
int is_leap_year(int y) {
    if (y % 400 == 0) return 1;
    if (y % 100 == 0) return 0;
    return (y % 4 == 0);
}

// validate date "DD/MM/YYYY", range DD valid per month, year MIN_YEAR..MAX_YEAR
int is_valid_date(const char *s) {
    if (!s) return 0;
    // expected format 2 digits '/' 2 digits '/' 4 digits or 1-2 digits for day/month allowed
    int d, m, y;
    if (sscanf(s, "%d/%d/%d", &d, &m, &y) != 3) return 0;
    if (y < MIN_YEAR || y > MAX_YEAR) return 0;
    if (m < 1 || m > MONTHS_IN_YEAR) return 0;
    if (d < 1) return 0;
    int mdays = DAYS_IN_MONTH_31;
    if (m == 2) {
        mdays = is_leap_year(y) ? DAYS_IN_FEBRUARY_LEAP : DAYS_IN_FEBRUARY_COMMON;
    } else if (m == 4 || m == 6 || m == 9 || m == 11) mdays = DAYS_IN_MONTH_30;
    if (d > mdays) return 0;
    // Also ensure string had no extra chars
    // Build normalized string and compare length-ish (simple)
    char buf[DATE_BUFFER_LEN];
    snprintf(buf, sizeof(buf), "%d/%d/%d", d, m, y);
    // allow leading zeros in input by not strict comparing; but ensure input only digits/slashes/spaces
    for (const char *p = s; *p; ++p) {
        if (!isdigit((unsigned char)*p) && *p != '/' && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') return 0;
    }
    return 1;
}

/* ---------- CSV I/O ---------- */

// If file doesn't exist, create and write sample data (20 records)
void ensure_csv_has_sample() {
    FILE *f = fopen(CSV_FILE, "r");
    if (f) {
        fclose(f);
        return;
    } // exists
    f = fopen(CSV_FILE, "w");
    if (!f) {
        perror("Cannot create CSV file");
        return;
    }
    // write header optional (we will not use header to simplify parsing)
    // Write 20 sample records (InspectionID,CarRegNumber,OwnerName,InspectionDate (DD/MM/YYYY))
    const char *samples[] = {
        "I001,ABC1234,John Doe,01/08/2025",
        "I002,XYZ5678,Jane Smith,03/08/2025",
        "I003,DEF1112,Junho Kim,05/08/2025",
        "I004,HIJ5060,Jordan Brown,07/08/2025",
        "I005,MYW5791,Justin Jackson,09/08/2025",
        "I006,ZBA7777,Zephyr Diaz,11/08/2025",
        "I007,QWE6006,Gale Norton,13/08/2025",
        "I008,MNO7788,Fiora Campbell,15/08/2025",
        "I009,JQR1111,Astarion Williams,17/08/2025",
        "I010,STR9633,Wyll Phillips,19/08/2025",
        "I011,WIS4002,Halsin Walker,21/08/2025",
        "I012,MSL5533,Karlach Harris,23/08/2025",
        "I013,LGD9889,Furuya Wataru,25/08/2025",
        "I014,FVA3022,Taeho Park,27/08/2025",
        "I015,SDH9966,Minju Hwang,29/08/2025",
        "I016,THZ5050,Shen Howard,31/08/2025",
        "I017,ZAQ4446,Mateo Ramos,02/09/2025",
        "I018,XHU1267,Luciana Esposito,04/09/2025",
        "I019,SOL2233,Kunibert Schneider,06/09/2025",
        "I020,GEN8047,Leon Lee,08/09/2025"
    };
    for (int i = 0; i < ID_REG_MAX_LEN; ++i) { // Use ID_REG_MAX_LEN for number of samples
        fprintf(f, "%s\n", samples[i]);
    }
    fclose(f);
}

// load CSV into records array; return count
int load_all(Record recs[], int max) {
    ensure_csv_has_sample();
    FILE *f = fopen(CSV_FILE, "r");
    if (!f) {
        perror("open csv");
        return 0;
    }
    char line[MAX_LINE];
    int count = 0;
    while (fgets(line, sizeof(line), f) && count < max) {
        // trim newline
        char *p = strchr(line, '\n');
        if (p) *p = '\0';
        p = strchr(line, '\r');
        if (p) *p = '\0';
        if (line[0] == '\0') continue;
        // split on commas into 4 tokens
        char *tk;
        tk = strtok(line, ",");
        if (!tk) continue;
        strncpy(recs[count].inspectionID, tk, sizeof(recs[count].inspectionID) - 1);
        recs[count].inspectionID[sizeof(recs[count].inspectionID) - 1] = 0;

        tk = strtok(NULL, ",");
        if (!tk) continue;
        strncpy(recs[count].carReg, tk, sizeof(recs[count].carReg) - 1);
        recs[count].carReg[sizeof(recs[count].carReg) - 1] = 0;

        tk = strtok(NULL, ",");
        if (!tk) continue;
        strncpy(recs[count].owner, tk, sizeof(recs[count].owner) - 1);
        recs[count].owner[sizeof(recs[count].owner) - 1] = 0;

        tk = strtok(NULL, ",");
        if (!tk) continue;
        strncpy(recs[count].date, tk, sizeof(recs[count].date) - 1);
        recs[count].date[sizeof(recs[count].date) - 1] = 0;

        ++count;
    }
    fclose(f);
    return count;
}

// save all records back to CSV (overwrite)
int save_all(Record recs[], int n) {
    FILE *f = fopen(CSV_FILE, "w");
    if (!f) {
        perror("save_all");
        return 0;
    }
    for (int i = 0; i < n; ++i) {
        fprintf(f, "%s,%s,%s,%s\n", recs[i].inspectionID, recs[i].carReg, recs[i].owner, recs[i].date);
    }
    fclose(f);
    return 1;
}

/* ---------- Utility to read line from stdin and handle '0' for back ---------- */

// read a line into buf, return 0 if user entered "0" EXACTLY (meaning back), else 1
int input_line(char *prompt, char *buf, int bufsize) {
    while (1) {
        printf("%s", prompt);
        if (!fgets(buf, bufsize, stdin)) return 0; // EOF treated as back
        // trim newline
        size_t L = strlen(buf);
        if (L > 0 && (buf[L - 1] == '\n' || buf[L - 1] == '\r')) {
            buf[L - 1] = 0;
            --L;
            if (L > 0 && buf[L - 1] == '\r') {
                buf[L - 1] = 0;
                --L;
            }
        }
        if (strcmp(buf, "0") == 0) return 0; // back
        // allow empty string (means keep current in edit)
        return 1;
    }
}

/* ---------- CRUD operations ---------- */

void display_all() {
    Record arr[MAX_RECORDS];
    int n = load_all(arr, MAX_RECORDS);
    printf("\n---- All inspections (%d) ----\n", n);
    printf("%-*s | %-*s | %-*s | %-*s\n",
           ID_REG_MAX_LEN, "InspectionID",
           ID_REG_MAX_LEN, "CarReg",
           OWNER_MAX_LEN, "Owner",
           DATE_MAX_LEN, "Date");
    printf("------------------------------------------------------------------------------------------------\n"); // Adjusted line length
    for (int i = 0; i < n; ++i) {
        printf("%-*s | %-*s | %-*s | %-*s\n",
               ID_REG_MAX_LEN, arr[i].inspectionID,
               ID_REG_MAX_LEN, arr[i].carReg,
               OWNER_MAX_LEN, arr[i].owner,
               DATE_MAX_LEN, arr[i].date);
    }
    printf("------------------------------------------------------------------------------------------------\n"); // Adjusted line length
}

// helper: find index by inspectionID or carReg (case-insensitive exact match); return -1 if not found
int find_by_id_or_reg(Record arr[], int n, const char *key) {
    char lower_key[ID_REG_BUFFER_LEN];
    strncpy(lower_key, key, sizeof(lower_key) - 1);
    lower_key[sizeof(lower_key) - 1] = '\0';
    to_lower_str(lower_key);

    char lower_id[ID_REG_BUFFER_LEN];
    char lower_reg[ID_REG_BUFFER_LEN];

    for (int i = 0; i < n; ++i) {
        strncpy(lower_id, arr[i].inspectionID, sizeof(lower_id) - 1);
        lower_id[sizeof(lower_id) - 1] = '\0';
        to_lower_str(lower_id);

        strncpy(lower_reg, arr[i].carReg, sizeof(lower_reg) - 1);
        lower_reg[sizeof(lower_reg) - 1] = '\0';
        to_lower_str(lower_reg);

        if (strcmp(lower_id, lower_key) == 0) return i;
        if (strcmp(lower_reg, lower_key) == 0) return i;
    }
    return -1;
}

void add_record() {
    Record arr[MAX_RECORDS];
    int n = load_all(arr, MAX_RECORDS);
    if (n >= MAX_RECORDS) {
        printf("Max records reached.\n");
        return;
    }

    Record r;
    char buf[INPUT_BUFFER_SIZE];

    printf("--- Add new inspection (type 0 to go back at any prompt) ---\n");

    // InspectionID
    while (1) {
        if (!input_line("InspectionID (letters+digits only): ", buf, sizeof(buf))) return;
        if (!is_valid_id_or_reg(buf)) {
            printf("Invalid ID format. Use letters and digits only (1-%d chars).\n", ID_REG_MAX_LEN);
            continue;
        }
        // ensure unique (case-insensitive)
        if (find_by_id_or_reg(arr, n, buf) != -1) {
            printf("This InspectionID (or CarReg) already exists. Choose another.\n");
            continue;
        }
        strncpy(r.inspectionID, buf, sizeof(r.inspectionID) - 1);
        r.inspectionID[sizeof(r.inspectionID) - 1] = 0;
        break;
    }

    // CarRegNumber
    while (1) {
        if (!input_line("CarRegNumber (letters+digits only): ", buf, sizeof(buf))) return;
        if (!is_valid_id_or_reg(buf)) {
            printf("Invalid CarReg format. Use letters and digits only (1-%d chars).\n", ID_REG_MAX_LEN);
            continue;
        }
        // ensure unique (case-insensitive)
        if (find_by_id_or_reg(arr, n, buf) != -1) {
            printf("This InspectionID (or CarReg) already exists. Choose another.\n");
            continue;
        }
        strncpy(r.carReg, buf, sizeof(r.carReg) - 1);
        r.carReg[sizeof(r.carReg) - 1] = 0;
        break;
    }

    // OwnerName
    while (1) {
        if (!input_line("OwnerName (letters and spaces only): ", buf, sizeof(buf))) return;
        if (!is_valid_owner_name(buf)) {
            printf("Invalid owner name. Use letters and spaces only (1-%d chars).\n", OWNER_MAX_LEN);
            continue;
        }
        strncpy(r.owner, buf, sizeof(r.owner) - 1);
        r.owner[sizeof(r.owner) - 1] = 0;
        break;
    }

    // Date
    while (1) {
        if (!input_line("InspectionDate DD/MM/YYYY (AD year): ", buf, sizeof(buf))) return;
        if (!is_valid_date(buf)) {
            printf("Invalid date. Ensure format DD/MM/YYYY and valid day/month/year range.\n");
            continue;
        }
        strncpy(r.date, buf, sizeof(r.date) - 1);
        r.date[sizeof(r.date) - 1] = 0;
        break;
    }

    // Show to user and confirm
    printf("\nAbout to add record:\n");
    printf("%s | %s | %s | %s\n", r.inspectionID, r.carReg, r.owner, r.date);
    printf("Confirm add? (Y/N): ");
    char c = getchar();
    while (getchar() != '\n'); // clear rest
    if (c != 'Y' && c != 'y') {
        printf("Add cancelled.\n");
        return;
    }

    // append and save
    arr[n++] = r;
    if (save_all(arr, n)) printf("Record added and saved.\n");
    else printf("Error saving file.\n");
}

void search_record() {
    Record arr[MAX_RECORDS];
    int n = load_all(arr, MAX_RECORDS);
    if (n == 0) {
        printf("No records.\n");
        return;
    }

    char buf[INPUT_BUFFER_SIZE];
    printf("--- Search by InspectionID or CarReg (type 0 to go back) ---\n");
    if (!input_line("Enter key: ", buf, sizeof(buf))) return;

    // Convert search key to lowercase for case-insensitive comparison
    char lower_search_key[ID_REG_BUFFER_LEN];
    strncpy(lower_search_key, buf, sizeof(lower_search_key) - 1);
    lower_search_key[sizeof(lower_search_key) - 1] = '\0';
    to_lower_str(lower_search_key);

    int found = 0;
    char lower_id[ID_REG_BUFFER_LEN];
    char lower_reg[ID_REG_BUFFER_LEN];

    for (int i = 0; i < n; ++i) {
        strncpy(lower_id, arr[i].inspectionID, sizeof(lower_id) - 1);
        lower_id[sizeof(lower_id) - 1] = '\0';
        to_lower_str(lower_id);

        strncpy(lower_reg, arr[i].carReg, sizeof(lower_reg) - 1);
        lower_reg[sizeof(lower_reg) - 1] = '\0';
        to_lower_str(lower_reg);

        // Case-insensitive exact match or substring match
        if (strstr(lower_id, lower_search_key) || strstr(lower_reg, lower_search_key)) {
            printf("Match: %s | %s | %s | %s\n", arr[i].inspectionID, arr[i].carReg, arr[i].owner, arr[i].date);
            found++;
        }
    }
    if (!found) printf("No matches found.\n");
    else printf("%d match(es) shown.\n", found);
}

void update_record() {
    Record arr[MAX_RECORDS];
    int n = load_all(arr, MAX_RECORDS);
    if (n == 0) {
        printf("No records.\n");
        return;
    }

    char buf[INPUT_BUFFER_SIZE];
    printf("--- Update (search by InspectionID or CarReg). Type 0 to go back ---\n");
    if (!input_line("Enter key: ", buf, sizeof(buf))) return;

    int idx = find_by_id_or_reg(arr, n, buf);
    if (idx == -1) {
        printf("No exact match found for '%s'. Try search first.\n", buf);
        return;
    }

    printf("Found record:\n");
    printf("%s | %s | %s | %s\n", arr[idx].inspectionID, arr[idx].carReg, arr[idx].owner, arr[idx].date);

    // Edit each field; blank input keeps current
    char in[INPUT_BUFFER_SIZE];

    // InspectionID - if change, must validate and unique
    printf("New InspectionID (enter to keep current '%s', or 0 to back): ", arr[idx].inspectionID);
    if (!fgets(in, sizeof(in), stdin)) return;
    // trim newline
    size_t L = strlen(in);
    if (L > 0 && (in[L - 1] == '\n' || in[L - 1] == '\r')) in[--L] = 0;
    if (strcmp(in, "0") == 0) return;
    if (strlen(in) > 0) {
        if (!is_valid_id_or_reg(in)) {
            printf("Invalid format; update aborted.\n");
            return;
        }
        // ensure unique (except itself, case-insensitive)
        int conflict = find_by_id_or_reg(arr, n, in);
        if (conflict != -1 && conflict != idx) {
            printf("ID conflict; update aborted.\n");
            return;
        }
        strncpy(arr[idx].inspectionID, in, sizeof(arr[idx].inspectionID) - 1);
        arr[idx].inspectionID[sizeof(arr[idx].inspectionID) - 1] = 0;
    }

    // CarReg
    printf("New CarReg (enter to keep current '%s', or 0 to back): ", arr[idx].carReg);
    if (!fgets(in, sizeof(in), stdin)) return;
    L = strlen(in);
    if (L > 0 && (in[L - 1] == '\n' || in[L - 1] == '\r')) in[--L] = 0;
    if (strcmp(in, "0") == 0) return;
    if (strlen(in) > 0) {
        if (!is_valid_id_or_reg(in)) {
            printf("Invalid format; update aborted.\n");
            return;
        }
        int conflict = find_by_id_or_reg(arr, n, in);
        if (conflict != -1 && conflict != idx) {
            printf("CarReg conflict; update aborted.\n");
            return;
        }
        strncpy(arr[idx].carReg, in, sizeof(arr[idx].carReg) - 1);
        arr[idx].carReg[sizeof(arr[idx].carReg) - 1] = 0;
    }

    // Owner
    printf("New Owner (enter to keep current '%s', or 0 to back): ", arr[idx].owner);
    if (!fgets(in, sizeof(in), stdin)) return;
    L = strlen(in);
    if (L > 0 && (in[L - 1] == '\n' || in[L - 1] == '\r')) in[--L] = 0;
    if (strcmp(in, "0") == 0) return;
    if (strlen(in) > 0) {
        if (!is_valid_owner_name(in)) {
            printf("Invalid owner name; update aborted.\n");
            return;
        }
        strncpy(arr[idx].owner, in, sizeof(arr[idx].owner) - 1);
        arr[idx].owner[sizeof(arr[idx].owner) - 1] = 0;
    }

    // Date
    printf("New Date (DD/MM/YYYY) (enter to keep current '%s', or 0 to back): ", arr[idx].date);
    if (!fgets(in, sizeof(in), stdin)) return;
    L = strlen(in);
    if (L > 0 && (in[L - 1] == '\n' || in[L - 1] == '\r')) in[--L] = 0;
    if (strcmp(in, "0") == 0) return;
    if (strlen(in) > 0) {
        if (!is_valid_date(in)) {
            printf("Invalid date; update aborted.\n");
            return;
        }
        strncpy(arr[idx].date, in, sizeof(arr[idx].date) - 1);
        arr[idx].date[sizeof(arr[idx].date) - 1] = 0;
    }

    if (save_all(arr, n)) printf("Record updated and saved.\n");
    else printf("Save failed.\n");
}

void delete_record() {
    Record arr[MAX_RECORDS];
    int n = load_all(arr, MAX_RECORDS);
    if (n == 0) {
        printf("No records.\n");
        return;
    }

    char buf[INPUT_BUFFER_SIZE];
    printf("--- Delete (search by InspectionID). Type 0 to go back ---\n");
    if (!input_line("Enter InspectionID to delete: ", buf, sizeof(buf))) return;

    int idx = -1;
    // Need a case-insensitive search for deletion as well
    char lower_buf[ID_REG_BUFFER_LEN];
    strncpy(lower_buf, buf, sizeof(lower_buf) - 1);
    lower_buf[sizeof(lower_buf) - 1] = '\0';
    to_lower_str(lower_buf);

    char lower_id[ID_REG_BUFFER_LEN];

    for (int i = 0; i < n; ++i) {
        strncpy(lower_id, arr[i].inspectionID, sizeof(lower_id) - 1);
        lower_id[sizeof(lower_id) - 1] = '\0';
        to_lower_str(lower_id);
        if (strcmp(lower_id, lower_buf) == 0) {
            idx = i;
            break;
        }
    }
    if (idx == -1) {
        printf("No exact match found for '%s'.\n", buf);
        return;
    }

    printf("Found: %s | %s | %s | %s\n", arr[idx].inspectionID, arr[idx].carReg, arr[idx].owner, arr[idx].date);
    printf("Confirm delete? (Y/N): ");
    char c = getchar();
    while (getchar() != '\n'); // clear
    if (c != 'Y' && c != 'y') {
        printf("Delete cancelled.\n");
        return;
    }
    // remove by shifting
    for (int i = idx; i < n - 1; ++i) arr[i] = arr[i + 1];
    n--;
    if (save_all(arr, n)) printf("Deleted and saved.\n");
    else printf("Save failed.\n");
}

/* ---------- Unit tests (2 functions): add & delete ---------- */

void unit_test_add() {
    printf("\n[Unit Test] add_record\n");

    // Arrange
    Record arr[MAX_RECORDS];
    int n = load_all(arr, MAX_RECORDS);
    // int before_count = n; // Not strictly needed for the assertion below

    Record t = {"UT001", "UTREG1", "Unit Tester", "01/10/2025"};

    // Act
    arr[n++] = t;
    save_all(arr, n);

    // Assert
    Record arr2[MAX_RECORDS];
    int m = load_all(arr2, MAX_RECORDS);
    int found = find_by_id_or_reg(arr2, m, "UT001");

    assert(found != -1);
    printf(" -> Add test passed (record UT001 found)\n");

    // Cleanup: remove the added record
    if (found != -1) {
        for (int i = found; i < m - 1; ++i) arr2[i] = arr2[i + 1];
        save_all(arr2, m - 1);
    }
}

void unit_test_delete() {
    printf("\n[Unit Test] delete_record\n");

    // Arrange
    Record arr[MAX_RECORDS];
    int n = load_all(arr, MAX_RECORDS);

    Record t = {"UTDEL1", "UTREG2", "Delete Tester", "02/10/2025"};
    arr[n++] = t;
    save_all(arr, n);

    int idx = find_by_id_or_reg(arr, n, "UTDEL1");
    assert(idx != -1); // Ensure it was added

    // Act
    for (int i = idx; i < n - 1; ++i) arr[i] = arr[i + 1];
    save_all(arr, n - 1);

    // Assert
    Record arr2[MAX_RECORDS];
    int m = load_all(arr2, MAX_RECORDS);
    int found = find_by_id_or_reg(arr2, m, "UTDEL1");
    assert(found == -1);
    printf(" -> Delete test passed (record UTDEL1 not found)\n");
}

/* ---------- E2E Test: create a temp record -> save -> reload -> verify ---------- */

void e2e_test() {
    printf("E2E Test: create a temp record -> save -> reload -> verify\n");

    // create new record
    Record test = {"E2E01", "E2EREG1", "Tester One", "10/10/2025"};

    // Load existing records
    Record arr[MAX_RECORDS];
    int n = load_all(arr, MAX_RECORDS);

    // Add new data
    arr[n++] = test;
    save_all(arr, n);
    printf(" -> added test record E2E01\n");

    // Reload records
    Record arr2[MAX_RECORDS];
    int m = load_all(arr2, MAX_RECORDS);

    // Verify
    int idx = find_by_id_or_reg(arr2, m, "E2E01");
    if (idx >= 0) {
        printf(" -> found record after reload: ID=%s, REG=%s, Owner=%s, Date=%s\n",
               arr2[idx].inspectionID, arr2[idx].carReg, arr2[idx].owner, arr2[idx].date);
        printf(" -> E2E Test PASSED\n");

        // Cleanup: remove the added record
        for (int i = idx; i < m - 1; ++i) arr2[i] = arr2[i + 1];
        save_all(arr2, m - 1);
    } else {
        printf(" -> could not find test record after reload!\n");
        printf(" -> E2E Test FAILED\n");
    }

    printf("E2E test finished\n");
}

/* ---------- Menu and main ---------- */

void unit_test_menu() {
    while (1) {
        printf("\n===== Unit Test Menu =====\n");
        printf("1) Test add_record\n");
        printf("2) Test delete_record\n");
        printf("0) Back\n");
        printf("==========================\n");
        printf("Choose option: ");
        char buf[16];
        if (!fgets(buf, sizeof(buf), stdin)) return;
        int opt = atoi(buf);
        switch (opt) {
            case 1: unit_test_add(); break;
            case 2: unit_test_delete(); break;
            case MENU_BACK: return;
            default: printf("Invalid choice.\n"); break;
        }
    }
}

void display_menu_top() {
    printf("\n===== Inspection Manager =====\n");
    printf("%d) Add record\n", MENU_ADD_RECORD);
    printf("%d) Search record\n", MENU_SEARCH_RECORD);
    printf("%d) Update record\n", MENU_UPDATE_RECORD);
    printf("%d) Delete record\n", MENU_DELETE_RECORD);
    printf("%d) Display all records\n", MENU_DISPLAY_ALL);
    printf("%d) Unit tests\n", MENU_UNIT_TESTS);
    printf("%d) E2E test\n", MENU_E2E_TEST);
    printf("%d) Exit\n", MENU_EXIT);
    printf("==============================\n");
}
int main() {
    // ensure sample CSV exists on first run
    ensure_csv_has_sample();

    while (1) {
        display_menu_top();
        printf("Choose option: ");
        char optbuf[16];
        if (!fgets(optbuf, sizeof(optbuf), stdin)) break; // Exit on EOF
        
        int opt = atoi(optbuf);
        
        // This check handles cases where user just presses Enter
        if (opt == 0 && strcmp(optbuf, "0\n") != 0 && strcmp(optbuf, "0\r\n") != 0) {
            continue;
        }

        switch (opt) {
            case MENU_ADD_RECORD: 
                add_record(); 
                break;
            case MENU_SEARCH_RECORD: 
                search_record(); 
                break;
            case MENU_UPDATE_RECORD: 
                update_record(); 
                break;
            case MENU_DELETE_RECORD: 
                delete_record(); 
                break;
            case MENU_DISPLAY_ALL: 
                display_all(); 
                break;
            case MENU_UNIT_TESTS: 
                unit_test_menu(); 
                break;
            case MENU_E2E_TEST:
                e2e_test();
                break;
            case MENU_EXIT: 
                printf("Goodbye.\n"); 
                return 0;
            default: 
                printf("Invalid choice. Enter a number from the menu.\n"); 
                break;
        }
    }
    return 0;
}

จากโค้ดนี่คุณช่วยถามคำถามอะไรก็ได้ สมมุติว่าคุณเป็นคนช่างสงสัย
