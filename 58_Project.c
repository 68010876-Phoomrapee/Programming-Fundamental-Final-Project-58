#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>

// Named Constants for limits and file
#define MAX_RECORDS 1000
#define MAX_LINE 512
#define CSV_FILE "users_data.csv"

// Named Constants for field lengths (including null terminator)
#define ID_REG_MAX_LEN 16
#define ID_REG_BUFFER_LEN (ID_REG_MAX_LEN + 1) // For strings InspectionID
#define CAR_REG_MAX_LEN 13  
#define CAR_REG_BUFFER_LEN (CAR_REG_MAX_LEN + 1) // For strings CarRegNumber
#define OWNER_MAX_LEN 40
#define OWNER_BUFFER_LEN (OWNER_MAX_LEN + 1) // For OwnerName
#define DATE_MAX_LEN 14 // DD/MM/YYYY
#define DATE_BUFFER_LEN (DATE_MAX_LEN + 1) // For InspectionDate (DD/MM/YYYY)
#define INPUT_BUFFER_SIZE 128 // General buffer for user input
#define TABLE_SEPARATOR "--------------------------------------------------------------------------------------------"

// Date validation constants
#define MIN_YEAR 1990
#define MAX_YEAR 2026
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

#if defined(_WIN32) || defined(_WIN64)
    #define strcasecmp _stricmp
#else
    #include <strings.h> // for macOS/Linux
#endif
// clear screen
void clear_screen() {
#ifdef _WIN32
    system("cls");      // Windows
#else
    system("clear");    // Linux / macOS / Unix
#endif
}

// Record structure (4 columns)
typedef struct {
    char inspectionID[ID_REG_BUFFER_LEN];
    char carReg[CAR_REG_BUFFER_LEN];
    char owner[OWNER_BUFFER_LEN];
    char date[DATE_BUFFER_LEN]; // DD/MM/YYYY
} Record;

/* ---------- Validation helpers ---------- */

int is_alnum_char(char c) {
    return isalpha((unsigned char)c) || isdigit((unsigned char)c);
}

// validate InspectionID: allowed letters A-Z (1 letter), digits 0-9 only (3 digits) and don't allow letters + 000,
int is_valid_id(const char *id) {
   if (strlen(id) != 4) return 0; 
    if (!isupper(id[0])) return 0;
    for (int i = 1; i < 4; i++) {
        if (!isdigit(id[i])) return 0;
    }
    if (id[1] == '0' && id[2] == '0' && id[3] == '0') return 0;
    return 1;
}
// validate CarRegNumber: allowed letters A-Z (3 letters) and digits 0-9 only (4 digits),
int is_valid_car_reg(const char *reg) {
    if (strlen(reg) != 7) return 0;
    for (int i = 0; i < 3; i++) {
        if (!isupper(reg[i])) return 0;
    }
    for (int i = 3; i < 7; i++) {
        if (!isdigit(reg[i])) return 0;
    }
    if (reg[3] == '0' && reg[4] == '0' && reg[5] == '0' && reg[6] == '0') return 0;
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
int is_valid_date(const char *s, char *normalized) {
    if (!s || !normalized) return 0;

    int d, m, y;
    if (sscanf(s, "%d/%d/%d", &d, &m, &y) != 3) return 0;

    if (y < MIN_YEAR || y > MAX_YEAR) return 0;
    if (m < 1 || m > MONTHS_IN_YEAR) return 0;

    int mdays;
    if (m == 2) {
        mdays = is_leap_year(y) ? DAYS_IN_FEBRUARY_LEAP : DAYS_IN_FEBRUARY_COMMON;
    } else if (m == 4 || m == 6 || m == 9 || m == 11) {
        mdays = DAYS_IN_MONTH_30;
    } else {
        mdays = DAYS_IN_MONTH_31;
    }

    if (d < 1 || d > mdays) return 0;
    for (const char *p = s; *p; ++p) {
        if (!isdigit((unsigned char)*p) && *p != '/' && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') return 0;
    }

    snprintf(normalized, DATE_BUFFER_LEN, "%02d/%02d/%04d", d, m, y);
    return 1;
}

int confirmAction(const char *message) {
    char buf[16];

    while (1) {
        printf("%s (Y/n): ", message);

        if (!fgets(buf, sizeof(buf), stdin)) {
            clearerr(stdin);
            continue;
        }

        buf[strcspn(buf, "\n")] = '\0';

        if (strlen(buf) == 1) {
            if (buf[0] == 'Y') return 1;
            if (buf[0] == 'n') return 0;
        }

        printf("\nInvalid input. Please enter Y or n.\n");
    }
}

void trim_whitespace(char *str) {
    if (!str) return;
    // Trim leading
    char *start = str;
    while (isspace((unsigned char)*start)) start++;
    memmove(str, start, strlen(start) + 1);

    // Trim trailing
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) *end-- = '\0';
}

void assert_equal_int(int actual, int expected, const char *msg) {
    if (actual == expected)
        printf("[PASS] %s\n", msg);
    else
        printf("[FAIL] %s: expected %d, got %d\n", msg, expected, actual);
}
void assert_equal_string(const char *actual, const char *expected, const char *msg) {
    if (strcmp(actual, expected) == 0)
        printf("[PASS] %s\n", msg);
    else
        printf("[FAIL] %s: expected '%s', got '%s'\n", msg, expected, actual);
}
int find_case_insensitive(Record *arr, int n, const char *key) {
    for (int i = 0; i < n; i++) {
        if (strcasecmp(arr[i].inspectionID, key) == 0 ||
            strcasecmp(arr[i].carReg, key) == 0) {
            return i;
        }
    }
    return -1;
}
void fake_save_all(Record recs[], int n) {
    printf("[FAKE SAVE] Would save %d records.\n", n);
}

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
        trim_whitespace(recs[count].inspectionID);

        tk = strtok(NULL, ",");
        if (!tk) continue;
        strncpy(recs[count].carReg, tk, sizeof(recs[count].carReg) - 1);
        recs[count].carReg[sizeof(recs[count].carReg) - 1] = 0;
        trim_whitespace(recs[count].carReg);

        tk = strtok(NULL, ",");
        if (!tk) continue;
        strncpy(recs[count].owner, tk, sizeof(recs[count].owner) - 1);
        recs[count].owner[sizeof(recs[count].owner) - 1] = 0;
        trim_whitespace(recs[count].owner);

        tk = strtok(NULL, ",");
        if (!tk) continue;
        strncpy(recs[count].date, tk, sizeof(recs[count].date) - 1);
        recs[count].date[sizeof(recs[count].date) - 1] = 0;
        trim_whitespace(recs[count].date);

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

int input_line(char *prompt, char *buf, int bufsize) {
    while (1) {
        printf("%s", prompt);
        if (!fgets(buf, bufsize, stdin)) return 0; 
        size_t L = strlen(buf);
        if (L > 0 && (buf[L - 1] == '\n' || buf[L - 1] == '\r')) {
            buf[L - 1] = 0;
            --L;
            if (L > 0 && buf[L - 1] == '\r') {
                buf[L - 1] = 0;
                --L;
            }
        }
        if (strcmp(buf, "0") == 0) return 0;
        return 1;
    }
}

/* ---------- CRUD operations ---------- */

void display_records(Record arr[], int n, const char *title) {
    if (title && title[0] != '\0') {
        printf("\n---- %s (%d) ----\n", title, n);
    } else {
        printf("\n---- Records (%d) ----\n", n);
    }
    printf("%-*s | %-*s | %-*s | %-*s\n",
           ID_REG_MAX_LEN, "InspectionID",
           CAR_REG_MAX_LEN, "CarRegNumber",
           OWNER_MAX_LEN, "OwnerName",
           DATE_MAX_LEN, "InspectionDate");
    printf("---------------------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < n; ++i) {
        printf("%-*s | %-*s | %-*s | %-*s\n",
               ID_REG_MAX_LEN, arr[i].inspectionID,
               CAR_REG_MAX_LEN, arr[i].carReg,
               OWNER_MAX_LEN, arr[i].owner,
               DATE_MAX_LEN, arr[i].date);
    }
    printf("---------------------------------------------------------------------------------------------------------------------\n");
}

void display_all() {
    Record arr[MAX_RECORDS];
    int n = load_all(arr, MAX_RECORDS);
    printf("\n---- All inspections (%d) ----\n", n);
    printf("%-*s | %-*s | %-*s | %-*s\n",
           ID_REG_MAX_LEN, "InspectionID",
           CAR_REG_MAX_LEN, "CarRegNumber",
           OWNER_MAX_LEN, "OwnerName",
           DATE_MAX_LEN, "InspectionDate");
    printf("---------------------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < n; ++i) {
        printf("%-*s | %-*s | %-*s | %-*s\n",
               ID_REG_MAX_LEN, arr[i].inspectionID,
               CAR_REG_MAX_LEN, arr[i].carReg,
               OWNER_MAX_LEN, arr[i].owner,
               DATE_MAX_LEN, arr[i].date);
    }
    printf("---------------------------------------------------------------------------------------------------------------------\n");
}

// helper: find index by inspectionID or carReg (case-insensitive exact match); return -1 if not found
int find_by_id_or_reg(Record arr[], int n, const char *key) {
    for (int i = 0; i < n; ++i) {
        if (_stricmp(arr[i].inspectionID, key) == 0) return i; // _stricmp for Windows , strcasecmp() for Linux/macOS
        if (_stricmp(arr[i].carReg, key) == 0) return i;
    }
    return -1;
}

void add_record() {
    Record arr[MAX_RECORDS];
    int n = load_all(arr, MAX_RECORDS);

    if (n >= MAX_RECORDS) {
        printf("\nMax records reached.\n");
        return;
    }

    char buf[INPUT_BUFFER_SIZE];
    char normalized_date_temp[DATE_BUFFER_LEN];

    clear_screen();

    printf("-----------------------------------------------------\n");
    printf("           ADD NEW INSPECTION RECORD\n");
    printf("   (Type 0 at any prompt to go back to menu)\n");
    printf("-----------------------------------------------------\n");
    
    display_records(arr, n, "Current Records");

    Record r;

    // InspectionID
    while (1) {
        if (!input_line("\nInspectionID (UPPERCASE letters and digits only): ", buf, sizeof(buf))) return;
        if (!is_valid_id(buf)) {
            printf("\nInvalid InspectionID format. Use UPPERCASE letters (A-Z) and digits (0-9) only.\nExample: A001, I009, B123 (1 uppercase letter + 3 digits)\n", ID_REG_MAX_LEN);
            continue;
        }
        if (find_by_id_or_reg(arr, n, buf) != -1) {
            printf("\nThis InspectionID or CarReg already exists.\n");
            continue;
        }
        strncpy(r.inspectionID, buf, sizeof(r.inspectionID) - 1);
        r.inspectionID[sizeof(r.inspectionID) - 1] = '\0';
        break;
    }

    // CarRegNumber
  while (1) {
    if (!input_line("\nCarRegNumber (UPPERCASE letters and digits only): ", buf, sizeof(buf))) return;
    if (!is_valid_car_reg(buf)) {
        printf("\nInvalid CarRegNumber format. Use UPPERCASE letters (A-Z) and digits (0-9) only.\nExample: ABC0001, XYZ2025 (3 uppercase letters + 4 digits)\n", CAR_REG_MAX_LEN);
        continue;
    }
    if (find_by_id_or_reg(arr, n, buf) != -1) {
        printf("\nThis InspectionID or CarRegNumber already exists.\n");
        continue;
    }
    strncpy(r.carReg, buf, sizeof(r.carReg) - 1);
    r.carReg[sizeof(r.carReg) - 1] = '\0';
    break;
}

    // OwnerName
    while (1) {
        if (!input_line("\nOwnerName (letters and spaces only): ", buf, sizeof(buf))) return;
        if (!is_valid_owner_name(buf)) {
            printf("\nInvalid OwnerName. Use letters and spaces only (1-%d chars).\n", OWNER_MAX_LEN);
            continue;
        }
        strncpy(r.owner, buf, sizeof(r.owner) - 1);
        r.owner[sizeof(r.owner) - 1] = '\0';
        break;
    }

    // Date
    while (1) {
        if (!input_line("\nInspectionDate DD/MM/YYYY (AD year): ", buf, sizeof(buf))) return;
        if (!is_valid_date(buf, normalized_date_temp)) {
            printf("\nInvalid InspectionDate. Please use a valid calendar date.\n");
            continue;
        }
        strncpy(r.date, normalized_date_temp, sizeof(r.date) - 1);
        r.date[sizeof(r.date) - 1] = '\0';
        break;
    }

    printf("\nPreview of new record:\n");
    printf("%-*s | %-*s | %-*s | %-*s\n",
       ID_REG_MAX_LEN, "InspectionID",
       CAR_REG_MAX_LEN, "CarRegNumber",
       OWNER_MAX_LEN, "OwnerName",
       DATE_MAX_LEN, "InspectionDate");
    printf("%s\n", TABLE_SEPARATOR);
    printf("%-*s | %-*s | %-*s | %-*s\n",
       ID_REG_MAX_LEN, r.inspectionID,
       CAR_REG_MAX_LEN, r.carReg,
       OWNER_MAX_LEN, r.owner,
       DATE_MAX_LEN, r.date);
    printf("%s\n", TABLE_SEPARATOR);

    int confirmed = confirmAction("\nConfirm to add this record?");
    if (!confirmed) {
        printf("\nAdd cancelled.\n");
        printf("\nPress Enter to return to menu...");
        while (getchar() != '\n' && !feof(stdin) && !ferror(stdin));
        return;
    }


    arr[n++] = r;
    if (save_all(arr, n)) {
    printf("\n------------------------------------------\n");
    printf("\nRecord added and saved successfully.\n");
} else {
    printf("\nError saving file.\n");
}

    printf("\nLatest Records:\n");
    display_records(arr, n, "All Records After Addition");

    printf("\nPress Enter to return to menu...");
    while (getchar() != '\n');
}

void search_record() {
    clear_screen();

    Record arr[MAX_RECORDS];
    int n = load_all(arr, MAX_RECORDS);
    if (n == 0) {
        printf("\nNo records.\n");
        printf("\nPress Enter to return to menu...");
        getchar();
        return;
    }

    char buf[INPUT_BUFFER_SIZE];
    printf("-----------------------------------------------------\n");
    printf("                SEARCH INSPECTION RECORD\n");
    printf("   (Search by InspectionID or CarRegNumber - type 0 to go back)\n");
    printf("-----------------------------------------------------\n");

    display_records(arr, n, "Current Records");

    if (!input_line("\nEnter key (InspectionID or CarRegNumber): ", buf, sizeof(buf)))
        return;

    trim_whitespace(buf);
    if (strcmp(buf, "0") == 0)
        return;

    int found = 0;

    printf("\n---- Search Results ----\n");
    printf("%-*s | %-*s | %-*s | %-*s\n",
           ID_REG_MAX_LEN, "InspectionID",
           CAR_REG_MAX_LEN, "CarRegNumber",
           OWNER_MAX_LEN, "OwnerName",
           DATE_MAX_LEN, "InspectionDate");
    printf("---------------------------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < n; ++i) {
#if defined(_WIN32) || defined(_WIN64)
        if (_stricmp(arr[i].inspectionID, buf) == 0 || _stricmp(arr[i].carReg, buf) == 0)
#else
        if (strcasecmp(arr[i].inspectionID, buf) == 0 || strcasecmp(arr[i].carReg, buf) == 0)
#endif
        {
            printf("%-*s | %-*s | %-*s | %-*s\n",
                   ID_REG_MAX_LEN, arr[i].inspectionID,
                   CAR_REG_MAX_LEN, arr[i].carReg,
                   OWNER_MAX_LEN, arr[i].owner,
                   DATE_MAX_LEN, arr[i].date);
            found++;
        }
    }

    printf("---------------------------------------------------------------------------------------------------------------------\n");

    if (!found)
        printf("\nNo matches found.\n");
    else
        printf("\n%d match(es) found.\n", found);

    printf("\nPress Enter to return to menu...");
    getchar();
}

void update_record() {
    clear_screen();
    Record arr[MAX_RECORDS];
    int n = load_all(arr, MAX_RECORDS);

    if (n == 0) {
        printf("No records available to update.\n");
        printf("\nPress Enter to return to menu...");
        while (getchar() != '\n' && !feof(stdin) && !ferror(stdin));
        getchar();
        return;
    }

    printf("-----------------------------------------------------\n");
    printf("             UPDATE INSPECTION RECORD\n");
    printf("   (Type 0 at any prompt to go back to menu)\n");
    printf("-----------------------------------------------------\n");

    display_all();

    char key[INPUT_BUFFER_SIZE];
    int idx = -1;

    while (idx == -1) {
        if (!input_line("\nEnter InspectionID or CarRegNumber to edit: ", key, sizeof(key))) {
        printf("\nReturning to main menu.\n");
    return;
    }

        trim_whitespace(key);

        if (strcmp(key, "0") == 0) return;

#if defined(_WIN32) || defined(_WIN64)
        for (int i = 0; i < n; i++) {
            if (_stricmp(arr[i].inspectionID, key) == 0 || _stricmp(arr[i].carReg, key) == 0) {
                idx = i;
                break;
            }
        }
#else
        for (int i = 0; i < n; i++) {
            if (strcasecmp(arr[i].inspectionID, key) == 0 || strcasecmp(arr[i].carReg, key) == 0) {
                idx = i;
                break;
            }
        }
#endif
        if (idx == -1) {
            printf("\nNo record found for '%s'. Please try again.\n", key);
        }
    }

    printf("\nFound record:\n");
    printf("%-*s | %-*s | %-*s | %-*s\n",
       ID_REG_MAX_LEN, "InspectionID",
       CAR_REG_MAX_LEN, "CarRegNumber",
       OWNER_MAX_LEN, "OwnerName",
       DATE_MAX_LEN, "InspectionDate");
    printf("%s\n", TABLE_SEPARATOR);
    printf("%-*s | %-*s | %-*s | %-*s\n",
       ID_REG_MAX_LEN, arr[idx].inspectionID,
       CAR_REG_MAX_LEN, arr[idx].carReg,
       OWNER_MAX_LEN, arr[idx].owner,
       DATE_MAX_LEN, arr[idx].date);
    printf("%s\n", TABLE_SEPARATOR);

   if (!confirmAction("\nConfirm to edit this record?")) {
    printf("\nUpdate cancelled.\n");
    printf("\nPress Enter to return to menu...");
    while (getchar() != '\n' && !feof(stdin) && !ferror(stdin));
    return;
}

    char buf[INPUT_BUFFER_SIZE];
    char normalized_date_temp[DATE_BUFFER_LEN];
    Record newRec = arr[idx];

    printf("\nPress Enter to keep the current data.\n");

    // --- Update InspectionID ---
    while (1) {
        printf("\nNew InspectionID (current: %s): ", newRec.inspectionID);
        if (!input_line("", buf, sizeof(buf))) return;
        if (strlen(buf) == 0) break;

        if (!is_valid_id(buf)) {
            printf("\nInvalid InspectionID format. Use UPPERCASE letters (A-Z) and digits (0-9) only.\nExample: A001, I009, B123 (1 uppercase letter + 3 digits)\n", ID_REG_MAX_LEN);
            continue;
        }

        int conflict = find_by_id_or_reg(arr, n, buf);
        if (conflict != -1 && conflict != idx) {
            printf("\nThis InspectionID already exists in another record.\n");
            continue;
        }

        strncpy(newRec.inspectionID, buf, sizeof(newRec.inspectionID) - 1);
        newRec.inspectionID[sizeof(newRec.inspectionID) - 1] = '\0';
        break;
    }

    // --- Update CarReg ---
    while (1) {
        printf("\nNew CarRegNumber (current: %s): ", newRec.carReg);
        if (!input_line("", buf, sizeof(buf))) return;
        if (strlen(buf) == 0) break;

        if (!is_valid_car_reg(buf)) {
            printf("\nInvalid CarRegNumber format. Use UPPERCASE letters (A-Z) and digits (0-9) only.\nExample: ABC0001, XYZ2025 (3 uppercase letters + 4 digits)\n", CAR_REG_MAX_LEN);
            continue;
        }

        int conflict = find_by_id_or_reg(arr, n, buf);
        if (conflict != -1 && conflict != idx) {
            printf("\nThis CarRegNumber already exists in another record.\n");
            continue;
        }

        strncpy(newRec.carReg, buf, sizeof(newRec.carReg) - 1);
        newRec.carReg[sizeof(newRec.carReg) - 1] = '\0';
        break;
    }

    // --- Update Owner ---
    while (1) {
        printf("\nNew OwnerName (current: %s): ", newRec.owner);
        if (!input_line("", buf, sizeof(buf))) return;
        if (strlen(buf) == 0) break;

        if (!is_valid_owner_name(buf)) {
            printf("\nInvalid OwnerName. Use letters and spaces only.\n");
            continue;
        }

        strncpy(newRec.owner, buf, sizeof(newRec.owner) - 1);
        newRec.owner[sizeof(newRec.owner) - 1] = '\0';
        break;
    }

    // --- Update Date ---
    while (1) {
        printf("\nNew InspectionDate (DD/MM/YYYY, current: %s): ", newRec.date);
        if (!input_line("", buf, sizeof(buf))) return;
        if (strlen(buf) == 0) break;

        if (!is_valid_date(buf, normalized_date_temp)) {
            printf("\nInvalid InspectionDate. Please use a valid calendar date.\n");
            continue;
        }

        strncpy(newRec.date, normalized_date_temp, sizeof(newRec.date) - 1);
        newRec.date[sizeof(newRec.date) - 1] = '\0';
        break;
    }
    printf("\nPreview of updated record:\n");
    printf("%-*s | %-*s | %-*s | %-*s\n",
       ID_REG_MAX_LEN, "InspectionID",
       CAR_REG_MAX_LEN, "CarRegNumber",
       OWNER_MAX_LEN, "OwnerName",
       DATE_MAX_LEN, "InspectionDate");
    printf("%s\n", TABLE_SEPARATOR);
    printf("%-*s | %-*s | %-*s | %-*s\n",
       ID_REG_MAX_LEN, newRec.inspectionID,
       CAR_REG_MAX_LEN, newRec.carReg,
       OWNER_MAX_LEN, newRec.owner,
       DATE_MAX_LEN, newRec.date);
    printf("%s\n", TABLE_SEPARATOR);

    if (!confirmAction("\nSave these changes?")) {
        printf("\nUpdate cancelled.\n");
        printf("\nPress Enter to return to menu...");
        while (getchar() != '\n' && !feof(stdin) && !ferror(stdin));
        return;
    }
    // Save updated record
    arr[idx] = newRec;

    if (save_all(arr, n)) {
        printf("\nRecord successfully updated!\n");
    } else {
        printf("\nError saving file. Changes might be lost.\n");
    }

    printf("\nLatest Records:\n");
    display_all();

    printf("\nPress Enter to return to menu...");
    while (getchar() != '\n' && !feof(stdin) && !ferror(stdin));
}

void delete_record() {
    clear_screen();
    Record arr[MAX_RECORDS];
    int n = load_all(arr, MAX_RECORDS);

    if (n == 0) {
        printf("\nNo records to delete.\n");
        printf("\nPress Enter to return to menu...");
        getchar();
        return;
    }

    printf("-----------------------------------------------------\n");
    printf("             DELETE INSPECTION RECORD\n");
    printf("   (Type 0 at any prompt to go back to menu)\n");
    printf("-----------------------------------------------------\n\n");

    display_all();

    char key[INPUT_BUFFER_SIZE];
    if (!input_line("\nEnter InspectionID or CarRegNumber to delete: ", key, sizeof(key))) return;
    if (strcmp(key, "0") == 0) return;

    int idx = find_by_id_or_reg(arr, n, key); 
    if (idx == -1) {
        printf("\nNo record found for '%s'.\n", key);
        printf("\nPress Enter to return to menu...");
        getchar();
        return;
    }

    printf("\nFound record:\n");

     printf("\nFound record:\n");
    printf("%-*s | %-*s | %-*s | %-*s\n",
           ID_REG_MAX_LEN, "InspectionID",
           CAR_REG_MAX_LEN, "CarRegNumber",
           OWNER_MAX_LEN, "OwnerName",
           DATE_MAX_LEN, "InspectionDate");
    printf("---------------------------------------------------------------------------------------------------------------------\n");
    printf("%-*s | %-*s | %-*s | %-*s\n",
           ID_REG_MAX_LEN, arr[idx].inspectionID,
           CAR_REG_MAX_LEN, arr[idx].carReg,
           OWNER_MAX_LEN, arr[idx].owner,
           DATE_MAX_LEN, arr[idx].date);
    printf("---------------------------------------------------------------------------------------------------------------------\n");


    if (!confirmAction("\nAre you sure you want to delete this record?")) {
        printf("\nDelete cancelled.\n");
        printf("\nPress Enter to return to menu...");
        getchar();
        return;
    }

    for (int i = idx; i < n - 1; ++i) {
        arr[i] = arr[i + 1];
    }
    n--;
    
    if (save_all(arr, n)) {
        printf("\n------------------------------------------\n");
        printf("\nSuccessfully deleted and saved.\n");
    } else {
        printf("\nError: Save failed.\n");
        printf("\nPress Enter to return to menu...");
        getchar();
        return;
    }

    printf("\nLatest Records:\n");
    display_all();

    printf("\nPress Enter to return to menu...");
    while (getchar() != '\n');
}

/* ---------- Unit tests (2 functions): search & delete ---------- */

void unit_test_search() {
    printf("\n[Unit Test] search_record\n");

    Record arr[MAX_RECORDS];
    int n = load_all(arr, MAX_RECORDS);

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

    auto void delete_record_test(const char* key, int confirm) {
        int n = load_all(arr, MAX_RECORDS);
        int idx = find_by_id_or_reg(arr, n, key);
        if (idx == -1) return;
        if (!confirm) return;
        for (int i = idx; i < n - 1; i++) arr[i] = arr[i + 1];
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

/* ---------- E2E Test ---------- */
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

/* ---------- Menu and main ---------- */

void unit_test_menu() {
    char buf[INPUT_BUFFER_SIZE];
    int opt;

    while (1) {
        clear_screen();
        printf("-----------------------------------------------------\n");
        printf("                 UNIT TEST MENU\n");
        printf("   (Type 0 at any prompt to go back to main menu)\n");
        printf("-----------------------------------------------------\n\n");
        
        printf("1) Run Search Record Unit Tests\n");
        printf("2) Run Delete Record Unit Tests\n");
        printf("0) Back to Main Menu\n");
        printf("=====================================================\n");
        printf("Enter your choice: ");

        if (!fgets(buf, sizeof(buf), stdin)) {
            clearerr(stdin);
            continue;
        }
        buf[strcspn(buf, "\n")] = 0;

        int valid = 1;
        for (int i = 0; buf[i] != '\0'; i++) {
            if (!isdigit((unsigned char)buf[i])) {
                valid = 0;
                break;
            }
        }

        if (!valid || strlen(buf) == 0) {
            printf("\nInvalid input. Please enter a number.\n");
            printf("Press Enter to continue...");
            while (getchar() != '\n' && !feof(stdin) && !ferror(stdin));
            continue;
        }

        opt = atoi(buf);

        switch (opt) {
            case 1: 
                clear_screen();
                unit_test_search(); 
                break;
            case 2: 
                clear_screen();
                unit_test_delete(); 
                break;
            case 0: 
                return;
            default: 
                printf("\nInvalid choice. Please select an option from the menu.\n"); 
                break;
        }
        printf("\nPress Enter to return to Unit Test Menu...");
        while (getchar() != '\n' && !feof(stdin) && !ferror(stdin));
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
    int choice;
    char input[INPUT_BUFFER_SIZE];

    while (1) {
        clear_screen();

        Record arr_initial[MAX_RECORDS];
        int n_initial = load_all(arr_initial, MAX_RECORDS);

        display_records(arr_initial, n_initial, "Current Records");

        printf("\n==== MENU ====\n");
        printf("1. Add Record\n");     
        printf("2. Search Record\n");  
        printf("3. Update Record\n");  
        printf("4. Delete Record\n");  
        printf("5. Unit Test\n");  
        printf("6. E2E Test\n");  
        printf("0. Exit\n");
        printf("\nEnter your choice: ");

        if (!fgets(input, sizeof(input), stdin)) {
            printf("Input error. Exiting.\n");
            return 1;
        }

        char *newline_pos = strchr(input, '\n');
        if (newline_pos) *newline_pos = '\0';
        else while (getchar() != '\n' && !feof(stdin));

        int valid = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isdigit((unsigned char)input[i])) valid = 0;
        }

        if (!valid || strlen(input) == 0) {
            printf("\nInvalid choice. Please enter a number from the menu.\n\nPress Enter to try again...");
            getchar();
            continue;
        }

        choice = atoi(input);

        switch (choice) {
            case 1:
                add_record();
                break;
            case 2:
                search_record();
                break;
            case 3:
                update_record();
                break;
            case 4:
                delete_record();
                break;
            case 5:
                unit_test_menu();
                break;
            case 6:
                e2e_test();
                break;
            case 0:
                printf("Exiting program...\n");
                return 0;
            default:
                printf("\nInvalid choice. Enter a number from the menu.\n");
                getchar();
                break;
        }
    }
}
