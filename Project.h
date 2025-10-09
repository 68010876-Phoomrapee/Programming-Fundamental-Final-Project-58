#ifndef PROJECT_H
#define PROJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// ==================== Named Constants ====================
#define MAX_RECORDS 1000
#define MAX_LINE 512
#define CSV_FILE "users_data.csv"

#define ID_REG_MAX_LEN 16
#define ID_REG_BUFFER_LEN (ID_REG_MAX_LEN + 1)
#define CAR_REG_MAX_LEN 13  
#define CAR_REG_BUFFER_LEN (CAR_REG_MAX_LEN + 1)
#define OWNER_MAX_LEN 40
#define OWNER_BUFFER_LEN (OWNER_MAX_LEN + 1)
#define DATE_MAX_LEN 14
#define DATE_BUFFER_LEN (DATE_MAX_LEN + 1)
#define INPUT_BUFFER_SIZE 128
#define TABLE_SEPARATOR "--------------------------------------------------------------------------------------------"

#define MIN_YEAR 1990
#define MAX_YEAR 2026
#define MONTHS_IN_YEAR 12
#define DAYS_IN_FEBRUARY_LEAP 29
#define DAYS_IN_FEBRUARY_COMMON 28
#define DAYS_IN_MONTH_30 30
#define DAYS_IN_MONTH_31 31

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
    #include <strings.h>
#endif

// ==================== Structs ====================
typedef struct {
    char inspectionID[ID_REG_BUFFER_LEN];
    char carReg[CAR_REG_BUFFER_LEN];
    char owner[OWNER_BUFFER_LEN];
    char date[DATE_BUFFER_LEN];
} Record;

// ==================== Utility Functions ====================
void clear_screen(void);
void trim_whitespace(char *str);
int input_line(char *prompt, char *buf, int bufsize);

// ==================== Validation ====================
int is_alnum_char(char c);
int is_valid_id(const char *id);
int is_valid_car_reg(const char *reg);
int is_valid_owner_name(const char *s);
int is_leap_year(int y);
int is_valid_date(const char *s, char *normalized);
int confirmAction(const char *message);

// ==================== Assertion ====================
void assert_equal_int(int actual, int expected, const char *msg);
void assert_equal_string(const char *actual, const char *expected, const char *msg);

// ==================== Search / Find ====================
int find_case_insensitive(Record *arr, int n, const char *key);
int find_by_id_or_reg(Record arr[], int n, const char *key);

// ==================== CSV File Operations ====================
void fake_save_all(Record recs[], int n);
int load_all(Record recs[], int max);
int save_all(Record recs[], int n);
void ensure_csv_has_sample(void);

// ==================== Display ====================
void display_records(Record arr[], int n, const char *title);
void display_all(void);

#endif // _58_PROJECT_H
