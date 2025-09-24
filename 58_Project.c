#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // For mkdir on POSIX systems
#include <direct.h>   // For _mkdir on Windows

#define MAX_LINE_LENGTH 256
#define MAX_USERS 100
#define FILENAME "users_data.csv"
#define DATA_PATH "C:\\C_Project\\data\\" FILENAME // Using concatenation

typedef struct {
    char inspectionID[MAX_LINE_LENGTH];
    char carRegNum[MAX_LINE_LENGTH];
    char ownerName[MAX_LINE_LENGTH];
    char inspectionDate[MAX_LINE_LENGTH];
} User;

// Function
void create_directory_and_file();
void add_user();
void edit_user();
void delete_user();
void search_user();
void display_menu();
int load_users_from_file(User users[], int *num_users);
void save_users_to_file(User users[], int num_users);

int main() {
    int choice;
    create_directory_and_file();

    do {
        display_menu();
        printf("Enter your choice: ");
        scanf("%d", &choice);
        while (getchar() != '\n');

        switch (choice) {
            case 1:
                add_user();
                break;
            case 2:
                edit_user();
                break;
            case 3:
                delete_user();
                break;
            case 4:
                search_user();
                break;
            case 5:
                printf("Exiting program. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
        printf("\n");
    } while (choice != 5);

    return 0;
}

// Function to create directory and file
void create_directory_and_file() {
    char dir_path[] = "C:\\C_Project\\data";
    
    #ifdef _WIN32
        _mkdir(dir_path);
    #else
        mkdir(dir_path, 0777);
    #endif

    FILE *fp = fopen(DATA_PATH, "a+");
    if (fp == NULL) {
        perror("Error creating or opening file");
        exit(EXIT_FAILURE);
    }
    fclose(fp);
}

// Main menu
void display_menu() {
    printf("--- User Management System ---\n");
    printf("1. Add Inspection Record\n");
    printf("2. Edit Inspection Record\n");
    printf("3. Delete Inspection Record\n");
    printf("4. Search Inspection Record\n");
    printf("5. Exit\n");
    printf("------------------------------\n");
}

// Loads user data from the CSV file
int load_users_from_file(User users[], int *num_users) {
    FILE *fp = fopen(DATA_PATH, "r");
    if (fp == NULL) {
        *num_users = 0;
        return 0;
    }

    *num_users = 0;
    char line[MAX_LINE_LENGTH * 4 + 4];

    char header_line[MAX_LINE_LENGTH];
    fgets(header_line, sizeof(header_line), fp);
    
    while (fgets(line, sizeof(line), fp) != NULL && *num_users < MAX_USERS) {
       
        if (strlen(line) <= 1) continue;

        char *token;
      
        if (*num_users == 0 && (unsigned char)line[0] == 0xEF && (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF) {
            memmove(line, line + 3, strlen(line) - 3 + 1);
        }

        // InspectionID
        token = strtok(line, ",");
        if (token != NULL) {
            strcpy(users[*num_users].inspectionID, token);
        } else {
            continue;
        }

        // CarRegNum
        token = strtok(NULL, ",");
        if (token != NULL) {
            strcpy(users[*num_users].carRegNum, token);
        } else {
            continue;
        }

        // OwnerName
        token = strtok(NULL, ",");
        if (token != NULL) {
            strcpy(users[*num_users].ownerName, token);
        } else {
            continue;
        }

        // InspectionDate
        token = strtok(NULL, "\n");
        if (token != NULL) {
            strcpy(users[*num_users].inspectionDate, token);
        } else {
            continue;
        }
        
        (*num_users)++;
    }

    fclose(fp);
    return 1;
}

// Saves user data to the CSV file
void save_users_to_file(User users[], int num_users) {
    FILE *fp = fopen(DATA_PATH, "w");
    if (fp == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }
    
    fprintf(fp, "InspectionID,CarRegNum,OwnerName,InspectionDate\n");

    for (int i = 0; i < num_users; i++) {
        fprintf(fp, "%s,%s,%s,%s\n", 
                users[i].inspectionID, 
                users[i].carRegNum, 
                users[i].ownerName, 
                users[i].inspectionDate);
    }
    fclose(fp);
}

// Adds a new user record
void add_user() {
    User users[MAX_USERS];
    int num_users = 0;
    load_users_from_file(users, &num_users);

    if (num_users >= MAX_USERS) {
        printf("Maximum number of users reached.\n");
        return;
    }

    printf("--- Add New Inspection Record ---\n");
    printf("Enter Inspection ID: ");
    fgets(users[num_users].inspectionID, sizeof(users[num_users].inspectionID), stdin);
    users[num_users].inspectionID[strcspn(users[num_users].inspectionID, "\n")] = 0;

    printf("Enter Car Registration Number: ");
    fgets(users[num_users].carRegNum, sizeof(users[num_users].carRegNum), stdin);
    users[num_users].carRegNum[strcspn(users[num_users].carRegNum, "\n")] = 0;

    printf("Enter Owner Name: ");
    fgets(users[num_users].ownerName, sizeof(users[num_users].ownerName), stdin);
    users[num_users].ownerName[strcspn(users[num_users].ownerName, "\n")] = 0;

    printf("Enter Inspection Date: ");
    fgets(users[num_users].inspectionDate, sizeof(users[num_users].inspectionDate), stdin);
    users[num_users].inspectionDate[strcspn(users[num_users].inspectionDate, "\n")] = 0;

    num_users++;
    save_users_to_file(users, num_users);
    printf("Record added successfully!\n");
}

// Edits an existing user record
void edit_user() {
    User users[MAX_USERS];
    int num_users = 0;
    load_users_from_file(users, &num_users);

    if (num_users == 0) {
        printf("No records to edit.\n");
        return;
    }

    char search_term[MAX_LINE_LENGTH];
    printf("--- Edit Inspection Record ---\n");
    printf("Enter Inspection ID or Car Registration Number to edit: ");
    fgets(search_term, sizeof(search_term), stdin);
    search_term[strcspn(search_term, "\n")] = 0;

    int found_index = -1;
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].inspectionID, search_term) == 0 || strcmp(users[i].carRegNum, search_term) == 0) {
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        printf("Record found:\n");
        printf("Inspection ID: %s, Car Reg: %s, Owner: %s, Date: %s\n", 
               users[found_index].inspectionID, users[found_index].carRegNum, 
               users[found_index].ownerName, users[found_index].inspectionDate);

        printf("Enter New Inspection ID (leave blank to keep current: %s): ", users[found_index].inspectionID);
        char new_id[MAX_LINE_LENGTH];
        fgets(new_id, sizeof(new_id), stdin);
        new_id[strcspn(new_id, "\n")] = 0;
        if (strlen(new_id) > 0) {
            strcpy(users[found_index].inspectionID, new_id);
        }

        printf("Enter New Car Registration Number (leave blank to keep current: %s): ", users[found_index].carRegNum);
        char new_car_reg[MAX_LINE_LENGTH];
        fgets(new_car_reg, sizeof(new_car_reg), stdin);
        new_car_reg[strcspn(new_car_reg, "\n")] = 0;
        if (strlen(new_car_reg) > 0) {
            strcpy(users[found_index].carRegNum, new_car_reg);
        }

        printf("Enter New Owner Name (leave blank to keep current: %s): ", users[found_index].ownerName);
        char new_owner[MAX_LINE_LENGTH];
        fgets(new_owner, sizeof(new_owner), stdin);
        new_owner[strcspn(new_owner, "\n")] = 0;
        if (strlen(new_owner) > 0) {
            strcpy(users[found_index].ownerName, new_owner);
        }

        printf("Enter New Inspection Date (leave blank to keep current: %s): ", users[found_index].inspectionDate);
        char new_date[MAX_LINE_LENGTH];
        fgets(new_date, sizeof(new_date), stdin);
        new_date[strcspn(new_date, "\n")] = 0;
        if (strlen(new_date) > 0) {
            strcpy(users[found_index].inspectionDate, new_date);
        }

        save_users_to_file(users, num_users);
        printf("Record updated successfully!\n");
    } else {
        printf("Record not found.\n");
    }
}

// Deletes an existing user record
void delete_user() {
    User users[MAX_USERS];
    int num_users = 0;
    load_users_from_file(users, &num_users);

    if (num_users == 0) {
        printf("No records to delete.\n");
        return;
    }

    char search_term[MAX_LINE_LENGTH];
    printf("--- Delete Inspection Record ---\n");
    printf("Enter Inspection ID or Car Registration Number to delete: ");
    fgets(search_term, sizeof(search_term), stdin);
    search_term[strcspn(search_term, "\n")] = 0;

    int found_index = -1;
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].inspectionID, search_term) == 0 || strcmp(users[i].carRegNum, search_term) == 0) {
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        printf("Record found:\n");
        printf("Inspection ID: %s, Car Reg: %s, Owner: %s, Date: %s\n", 
               users[found_index].inspectionID, users[found_index].carRegNum, 
               users[found_index].ownerName, users[found_index].inspectionDate);
        
        printf("Are you sure you want to delete this record? (y/n): ");
        char confirm;
        scanf(" %c", &confirm); 
        while (getchar() != '\n'); 

        if (confirm == 'y' || confirm == 'Y') {
            for (int i = found_index; i < num_users - 1; i++) {
                users[i] = users[i + 1];
            }
            num_users--;
            save_users_to_file(users, num_users);
            printf("Record deleted successfully!\n");
        } else {
            printf("Deletion cancelled.\n");
        }
    } else {
        printf("Record not found.\n");
    }
}

// Searches for a user record
void search_user() {
    User users[MAX_USERS];
    int num_users = 0;
    load_users_from_file(users, &num_users);

    if (num_users == 0) {
        printf("No records to search.\n");
        return;
    }

    char search_term[MAX_LINE_LENGTH];
    printf("--- Search Inspection Record ---\n");
    printf("Enter Owner Name or Car Registration Number to search: ");
    fgets(search_term, sizeof(search_term), stdin);
    search_term[strcspn(search_term, "\n")] = 0;

    int found_count = 0;
    for (int i = 0; i < num_users; i++) {
        if (strstr(users[i].ownerName, search_term) != NULL || strstr(users[i].carRegNum, search_term) != NULL) {
            printf("Found Record:\n");
            printf("Inspection ID: %s, Car Reg: %s, Owner: %s, Date: %s\n", 
                   users[i].inspectionID, users[i].carRegNum, users[i].ownerName, users[i].inspectionDate);
            found_count++;
        }
    }

    if (found_count == 0) {
        printf("No records found matching '%s'.\n", search_term);
    } else {
        printf("%d record(s) found.\n", found_count);
    }
}