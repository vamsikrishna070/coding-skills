#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STUD_FILE "students.dat"
#define TKT_FILE  "tickets.dat"
#define CSV_FILE  "students.csv"
#define TXT_FILE  "students.txt"
#define MAX_STUDENTS 2000

typedef struct {
    char id[20];
    char name[50];
    char branch[20];
    char section[10];
    float cgpa;
    char phone[15];
    char password[20];
} Student;

typedef struct {
    int ticketId;
    char studentId[20];
    char message[200];
    char status[10];
} Ticket;

/* ---------- INPUT HELPERS (NO scanf MIXING) ---------- */

void readLine(char *buf, size_t size) {
    if (fgets(buf, (int)size, stdin)) {
        buf[strcspn(buf, "\n")] = '\0';
    } else {
        if (size > 0) buf[0] = '\0';
    }
}

int readInt() {
    char line[64];
    int x;
    while (1) {
        if (!fgets(line, sizeof(line), stdin))
            return 0;
        if (sscanf(line, "%d", &x) == 1)
            return x;
        printf("Enter a valid number: ");
    }
}

float readFloat() {
    char line[64];
    float x;
    while (1) {
        if (!fgets(line, sizeof(line), stdin))
            return 0.0f;
        if (sscanf(line, "%f", &x) == 1)
            return x;
        printf("Enter a valid float value: ");
    }
}

void pauseScreen() {
    printf("\nPress ENTER to continue...");
    char dummy[8];
    readLine(dummy, sizeof(dummy));
}

/* ---------- AUTH ---------- */

int managementLogin() {
    char u[32], p[32];

    printf("----- MANAGEMENT LOGIN -----\n");
    printf("Username: ");
    readLine(u, sizeof(u));

    printf("Password: ");
    readLine(p, sizeof(p));

    if (strcmp(u, "admin") == 0 && strcmp(p, "admin123") == 0) {
        return 1;
    }
    return 0;
}

int studentLogin(char outId[]) {
    char id[20];
    char pass[20];
    Student s;
    FILE *fp = fopen(STUD_FILE, "rb");
    if (!fp) {
        printf("No student database found. Please add students first via Management Login.\n");
        return 0;
    }

    printf("----- STUDENT LOGIN -----\n");
    printf("Student ID: ");
    readLine(id, sizeof(id));
    
    printf("Password: ");
    readLine(pass, sizeof(pass));

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (strcmp(s.id, id) == 0 && strcmp(s.password, pass) == 0) {
            fclose(fp);
            strcpy(outId, id);
            return 1;
        }
    }

    fclose(fp);
    printf("\nInvalid credentials or student not found.\n");
    return 0;
}

/* ---------- STUDENT FILE HELPERS ---------- */

int loadAllStudents(Student arr[], int maxCount) {
    FILE *fp = fopen(STUD_FILE, "rb");
    if (!fp) return 0;
    int n = 0;
    while (n < maxCount && fread(&arr[n], sizeof(Student), 1, fp) == 1) {
        n++;
    }
    fclose(fp);
    return n;
}

void saveAllStudents(Student arr[], int count) {
    FILE *fp = fopen(STUD_FILE, "wb");
    if (!fp) {
        perror("Error writing students file");
        return;
    }
    fwrite(arr, sizeof(Student), count, fp);
    fclose(fp);
}

/* ---------- CSV EXPORT ---------- */

void autoExportCSV() {
    Student arr[MAX_STUDENTS];
    int n = loadAllStudents(arr, MAX_STUDENTS);
    if (n == 0) return;
    
    FILE *fp = fopen(CSV_FILE, "w");
    if (!fp) return;
    
    fprintf(fp, "ID,Name,Branch,Section,CGPA,Phone\n");
    for (int i = 0; i < n; i++) {
        fprintf(fp, "%s,%s,%s,%s,%.2f,%s\n",
                arr[i].id, arr[i].name, arr[i].branch, arr[i].section,
                arr[i].cgpa, arr[i].phone);
    }
    fclose(fp);
}

/* ---------- MANAGEMENT: CRUD ---------- */

void addStudent() {
    Student s;
    memset(&s, 0, sizeof(Student));
    FILE *fp = fopen(STUD_FILE, "ab");
    if (!fp) {
        perror("Cannot open student file");
        pauseScreen();
        return;
    }

    printf("----- ADD STUDENT -----\n");

    printf("ID: ");
    readLine(s.id, sizeof(s.id));

    printf("Name: ");
    readLine(s.name, sizeof(s.name));

    printf("Branch: ");
    readLine(s.branch, sizeof(s.branch));

    printf("Section: ");
    readLine(s.section, sizeof(s.section));

    printf("CGPA: ");
    s.cgpa = readFloat();

    printf("Phone: ");
    readLine(s.phone, sizeof(s.phone));

    snprintf(s.password, sizeof(s.password), "%s@pass", s.id);

    fwrite(&s, sizeof(Student), 1, fp);
    fclose(fp);

    printf("\nStudent added.\n");
    printf("Default password: %s\n", s.password);
    
    autoExportCSV();
    printf("CSV updated automatically.\n");
    
    pauseScreen();
}

void displayAll() {
    Student s;
    FILE *fp = fopen(STUD_FILE, "rb");
    if (!fp) {
        printf("No data.\n");
        pauseScreen();
        return;
    }

    FILE *txt = fopen(TXT_FILE, "w");
    if (!txt) {
        printf("Warning: Could not create %s (txt export will be skipped).\n", TXT_FILE);
    }

    printf("\n----- ALL STUDENTS -----\n");
    if (txt) fprintf(txt, "----- ALL STUDENTS -----\n");

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (s.id[0] == '\0') continue;  

        printf("\nID: %s\nName: %s\nBranch: %s\nSection: %s\nCGPA: %.2f\nPhone: %s\n",
               s.id, s.name, s.branch, s.section, s.cgpa, s.phone);

        if (txt) {
            fprintf(txt,
                    "\nID: %s\nName: %s\nBranch: %s\nSection: %s\nCGPA: %.2f\nPhone: %s\n",
                    s.id, s.name, s.branch, s.section, s.cgpa, s.phone);
        }
    }

    fclose(fp);
    if (txt) {
        fclose(txt);
        printf("\nData also written to %s\n", TXT_FILE);
    }

    pauseScreen();
}

void updateStudent() {
    char id[20];
    int found = 0;
    Student s;
    FILE *fp = fopen(STUD_FILE, "rb+");
    if (!fp) {
        printf("No student file.\n");
        pauseScreen();
        return;
    }

    printf("\nEnter ID to update: ");
    readLine(id, sizeof(id));

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (strcmp(s.id, id) == 0) {
            found = 1;

            printf("New Name: ");
            readLine(s.name, sizeof(s.name));

            printf("New Branch: ");
            readLine(s.branch, sizeof(s.branch));

            printf("New Section: ");
            readLine(s.section, sizeof(s.section));

            printf("New CGPA: ");
            s.cgpa = readFloat();

            printf("New Phone: ");
            readLine(s.phone, sizeof(s.phone));

            fseek(fp, -(long)sizeof(Student), SEEK_CUR);
            fwrite(&s, sizeof(Student), 1, fp);
            break;
        }
    }

    fclose(fp);

    if (found) {
        printf("\nUpdated.\n");
        autoExportCSV();
        printf("CSV updated automatically.\n");
    } else {
        printf("\nID not found.\n");
    }

    pauseScreen();
}

void deleteStudent() {
    char id[20];
    int found = 0;
    Student s;
    FILE *fp = fopen(STUD_FILE, "rb");
    FILE *tmp = fopen("tmp.dat", "wb");

    if (!fp || !tmp) {
        printf("Error opening files.\n");
        if (fp) fclose(fp);
        if (tmp) fclose(tmp);
        pauseScreen();
        return;
    }

    printf("\nEnter ID to delete: ");
    readLine(id, sizeof(id));

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (strcmp(s.id, id) == 0) {
            found = 1;
            continue;
        }
        fwrite(&s, sizeof(Student), 1, tmp);
    }

    fclose(fp);
    fclose(tmp);

    remove(STUD_FILE);
    rename("tmp.dat", STUD_FILE);

    if (found) {
        printf("Deleted.\n");
        autoExportCSV();
        printf("CSV updated automatically.\n");
    } else {
        printf("ID not found.\n");
    }

    pauseScreen();
}

void changeStudentId() {
    char oldId[20], newId[20];
    int found = 0;
    Student s;
    FILE *fp = fopen(STUD_FILE, "rb+");
    if (!fp) {
        printf("No student file.\n");
        pauseScreen();
        return;
    }

    printf("\nEnter old ID: ");
    readLine(oldId, sizeof(oldId));

    printf("Enter new ID: ");
    readLine(newId, sizeof(newId));

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (strcmp(s.id, oldId) == 0) {
            strcpy(s.id, newId);
            snprintf(s.password, sizeof(s.password), "%s@pass", s.id);
            fseek(fp, -(long)sizeof(Student), SEEK_CUR);
            fwrite(&s, sizeof(Student), 1, fp);
            found = 1;
            break;
        }
    }

    fclose(fp);

    if (found) {
        printf("\nID changed.\n");
        autoExportCSV();
        printf("CSV updated automatically.\n");
    } else {
        printf("\nOld ID not found.\n");
    }

    pauseScreen();
}

/* ---------- ANALYTICS & SORTING ---------- */

int cmpCgpaAsc(const void *a, const void *b) {
    const Student *sa = (const Student *)a;
    const Student *sb = (const Student *)b;
    if (sa->cgpa < sb->cgpa) return -1;
    if (sa->cgpa > sb->cgpa) return 1;
    return strcmp(sa->name, sb->name);
}

int cmpCgpaDesc(const void *a, const void *b) {
    return -cmpCgpaAsc(a, b);
}

int cmpNameAsc(const void *a, const void *b) {
    const Student *sa = (const Student *)a;
    const Student *sb = (const Student *)b;
    return strcmp(sa->name, sb->name);
}

void showAnalytics() {
    Student arr[MAX_STUDENTS];
    int n = loadAllStudents(arr, MAX_STUDENTS);
    if (n == 0) {
        printf("\nNo students found.\n");
        pauseScreen();
        return;
    }

    float sum = 0.0f;
    int hi = 0, lo = 0;

    for (int i = 0; i < n; i++) {
        sum += arr[i].cgpa;
        if (arr[i].cgpa > arr[hi].cgpa) hi = i;
        if (arr[i].cgpa < arr[lo].cgpa) lo = i;
    }

    float avg = sum / n;

    printf("\n----- ANALYTICS -----\n");
    printf("Total Students : %d\n", n);
    printf("Average CGPA   : %.2f\n", avg);

    printf("\nHighest CGPA:\n");
    printf("ID: %s | Name: %s | CGPA: %.2f\n",
           arr[hi].id, arr[hi].name, arr[hi].cgpa);

    printf("\nLowest CGPA:\n");
    printf("ID: %s | Name: %s | CGPA: %.2f\n",
           arr[lo].id, arr[lo].name, arr[lo].cgpa);

    pauseScreen();
}

void sortStudentsMenu() {
    Student arr[MAX_STUDENTS];
    int n = loadAllStudents(arr, MAX_STUDENTS);
    if (n == 0) {
        printf("\nNo students to sort.\n");
        pauseScreen();
        return;
    }

    int choice;
    printf("\n----- SORTING OPTIONS -----\n");
    printf("1. By CGPA (Ascending)\n");
    printf("2. By CGPA (Descending)\n");
    printf("3. By Name (A-Z)\n");
    printf("Choose: ");
    choice = readInt();

    switch (choice) {
        case 1:
            qsort(arr, n, sizeof(Student), cmpCgpaAsc);
            printf("\nSorted by CGPA (Ascending).\n");
            break;
        case 2:
            qsort(arr, n, sizeof(Student), cmpCgpaDesc);
            printf("\nSorted by CGPA (Descending).\n");
            break;
        case 3:
            qsort(arr, n, sizeof(Student), cmpNameAsc);
            printf("\nSorted by Name (A-Z).\n");
            break;
        default:
            printf("Invalid choice.\n");
            pauseScreen();
            return;
    }

    saveAllStudents(arr, n);
    autoExportCSV();
    printf("CSV updated automatically.\n");

    printf("\n----- STUDENTS AFTER SORTING -----\n");
    for (int i = 0; i < n; i++) {
        printf("\nID: %s\nName: %s\nBranch: %s\nSection: %s\nCGPA: %.2f\nPhone: %s\n",
               arr[i].id, arr[i].name, arr[i].branch, arr[i].section, arr[i].cgpa, arr[i].phone);
    }

    pauseScreen();
}

void exportToCSV() {
    autoExportCSV();
    Student arr[MAX_STUDENTS];
    int n = loadAllStudents(arr, MAX_STUDENTS);
    printf("\nExported %d students to %s\n", n, CSV_FILE);
    pauseScreen();
}

/* ---------- TICKETS ---------- */

int nextTicketId() {
    Ticket t;
    int max = 0;
    FILE *fp = fopen(TKT_FILE, "rb");
    if (!fp) return 1;

    while (fread(&t, sizeof(Ticket), 1, fp) == 1) {
        if (t.ticketId > max) max = t.ticketId;
    }

    fclose(fp);
    return max + 1;
}

void raiseTicket(char sid[]) {
    Ticket t;
    memset(&t, 0, sizeof(Ticket));
    FILE *fp = fopen(TKT_FILE, "ab");
    if (!fp) {
        printf("Cannot open ticket file.\n");
        pauseScreen();
        return;
    }

    t.ticketId = nextTicketId();
    strcpy(t.studentId, sid);
    strcpy(t.status, "OPEN");

    printf("\nEnter your concern: ");
    readLine(t.message, sizeof(t.message));

    fwrite(&t, sizeof(Ticket), 1, fp);
    fclose(fp);

    printf("\nTicket raised. ID: %d\n", t.ticketId);
    pauseScreen();
}

void viewTickets() {
    Ticket t;
    FILE *fp = fopen(TKT_FILE, "rb");
    if (!fp) {
        printf("No tickets.\n");
        pauseScreen();
        return;
    }

    printf("\n----- ALL TICKETS -----\n");

    while (fread(&t, sizeof(Ticket), 1, fp) == 1) {
        printf("\nTicket ID: %d\nStudent ID: %s\nIssue: %s\nStatus: %s\n",
               t.ticketId, t.studentId, t.message, t.status);
    }

    fclose(fp);
    pauseScreen();
}

void closeTicket() {
    int tid, found = 0;
    Ticket t;
    FILE *fp = fopen(TKT_FILE, "rb");
    FILE *tmp = fopen("tmp_tkt.dat", "wb");

    if (!fp || !tmp) {
        printf("Error opening ticket files.\n");
        if (fp) fclose(fp);
        if (tmp) fclose(tmp);
        pauseScreen();
        return;
    }

    printf("\nEnter Ticket ID to close: ");
    tid = readInt();

    while (fread(&t, sizeof(Ticket), 1, fp) == 1) {
        if (t.ticketId == tid) {
            strcpy(t.status, "CLOSED");
            found = 1;
        }
        fwrite(&t, sizeof(Ticket), 1, tmp);
    }

    fclose(fp);
    fclose(tmp);

    remove(TKT_FILE);
    rename("tmp_tkt.dat", TKT_FILE);

    if (found) printf("\nTicket closed.\n");
    else printf("\nTicket not found.\n");

    pauseScreen();
}

/* ---------- STUDENT SIDE ---------- */

void viewMyDetails(char sid[]) {
    Student s;
    FILE *fp = fopen(STUD_FILE, "rb");
    if (!fp) {
        printf("No student file.\n");
        pauseScreen();
        return;
    }

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (strcmp(s.id, sid) == 0) {
            printf("\n----- MY DETAILS -----\n");
            printf("ID: %s\nName: %s\nBranch: %s\nSection: %s\nCGPA: %.2f\nPhone: %s\n",
                   s.id, s.name, s.branch, s.section, s.cgpa, s.phone);
            break;
        }
    }

    fclose(fp);
    pauseScreen();
}

void changePassword(char sid[]) {
    Student s;
    FILE *fp = fopen(STUD_FILE, "rb+");
    if (!fp) {
        printf("No student file.\n");
        pauseScreen();
        return;
    }

    char newPass[20];

    printf("\nEnter new password: ");
    readLine(newPass, sizeof(newPass));

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (strcmp(s.id, sid) == 0) {
            strncpy(s.password, newPass, sizeof(s.password) - 1);
            s.password[sizeof(s.password) - 1] = '\0';
            fseek(fp, -(long)sizeof(Student), SEEK_CUR);
            fwrite(&s, sizeof(Student), 1, fp);
            break;
        }
    }

    fclose(fp);

    printf("Password updated.\n");
    pauseScreen();
}

/* ---------- MENUS ---------- */

void managementMenu() {
    int c;
    while (1) {
        printf("\n----- MANAGEMENT MENU -----\n");
        printf("1. Add Student\n");
        printf("2. View All Students\n");
        printf("3. Update Student\n");
        printf("4. Delete Student\n");
        printf("5. Change Student ID\n");
        printf("6. View Tickets\n");
        printf("7. Close Ticket\n");
        printf("8. Analytics (CGPA Stats)\n");
        printf("9. Sorting Options\n");
        printf("10. Export Students to CSV\n");
        printf("0. Logout\n");
        printf("Choose: ");
        c = readInt();

        switch (c) {
            case 1: addStudent(); break;
            case 2: displayAll(); break;
            case 3: updateStudent(); break;
            case 4: deleteStudent(); break;
            case 5: changeStudentId(); break;
            case 6: viewTickets(); break;
            case 7: closeTicket(); break;
            case 8: showAnalytics(); break;
            case 9: sortStudentsMenu(); break;
            case 10: exportToCSV(); break;
            case 0: return;
            default: printf("Invalid.\n"); pauseScreen();
        }
    }
}

void studentMenu(char sid[]) {
    int c;
    while (1) {
        printf("\n----- STUDENT MENU -----\n");
        printf("1. View My Details\n");
        printf("2. Raise Ticket\n");
        printf("3. Change Password\n");
        printf("0. Logout\n");
        printf("Choose: ");
        c = readInt();

        switch (c) {
            case 1: viewMyDetails(sid); break;
            case 2: raiseTicket(sid); break;
            case 3: changePassword(sid); break;
            case 0: return;
            default: printf("Invalid.\n"); pauseScreen();
        }
    }
}

/* ---------- MAIN ---------- */

int main() {
    int ch;
    char sid[20];

    while (1) {
        printf("\n===== STUDENT RECORD MANAGEMENT SYSTEM =====\n");
        printf("1. Management Login\n");
        printf("2. Student Login\n");
        printf("0. Exit\n");
        printf("Choose: ");
        ch = readInt();

        if (ch == 1) {
            if (managementLogin()) managementMenu();
            else { printf("Login failed.\n"); pauseScreen(); }
        }
        else if (ch == 2) {
            if (studentLogin(sid)) studentMenu(sid);
            else { printf("Login failed.\n"); pauseScreen(); }
        }
        else if (ch == 0) {
            printf("\nExiting...\n");
            return 0;
        }
        else {
            printf("Invalid.\n");
            pauseScreen();
        }
    }

    return 0;
}
