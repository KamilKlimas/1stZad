#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

struct MY_STUD {
    int year;
    char* nazwisko;
    unsigned int len;
};

typedef struct MY_STUD MY_STUD;

MY_STUD* CreateArray(size_t no_items) {
    MY_STUD* tab = (MY_STUD*)malloc(no_items * sizeof(MY_STUD));
    if (tab) {
        memset(tab, 0, no_items * sizeof(MY_STUD));
    }
    return tab;
}

MY_STUD* FreeArray(MY_STUD* tab, size_t no_items) {
    if (tab) {
        for (size_t it = 0; it < no_items; ++it) {
            if (tab[it].nazwisko) {
                free(tab[it].nazwisko);
            }
            tab[it].nazwisko = NULL;
        }
        free(tab);
        tab = NULL;
    }
    return NULL;
}

int FillArray(MY_STUD* tab, size_t no_items) {
    if (!tab) return 0; // Check for null pointer

    char str[128];
    int s_begin = 'a', s_end = 'z'; // Use valid ASCII range for letters
    int year = 1000;

    srand((unsigned int)time(NULL));

    for (size_t it = 0; it < no_items; ++it) {
        int len = rand() % 20 + 5; // Generate length from 5 to 25

        for (int i = 0; i < len; ++i) {
            int symb = rand() % (s_end - s_begin + 1) + s_begin;
            str[i] = (char)symb;
        }
        str[len] = '\0'; // Null terminator

        tab[it].nazwisko = (char*)malloc((len + 1) * sizeof(char));
        if (!tab[it].nazwisko) return 0;

        strcpy(tab[it].nazwisko, str);
        tab[it].year = year++;
        tab[it].len = len + 1;
    }

    return 1;
}

void PrintArray(MY_STUD* tab, size_t no_items, const char* title) {
    if (!tab) return; // Check for null pointer

    printf("%s\n", title);
    printf("%-30s %10s\n", "Nazwisko", "Rok");

    for (size_t it = 0; it < no_items; ++it) {
        printf("%-30s %10d\n", tab[it].nazwisko, tab[it].year);
    }

    printf("\n");
}

void MyExit(FILE* pft, MY_STUD* tab, size_t no_items, long long* fdesc) {
    if (pft) fclose(pft);
    FreeArray(tab, no_items);
    if (fdesc) free(fdesc);
    printf("Fatal error\n");
    exit(1);
}

void Save(MY_STUD* tab, size_t no_items, const char* filename) {
    if (!tab) return; // Check for null pointer

    unsigned int no_it = (unsigned int)no_items;
    long long* file_desc = (long long*)malloc((no_items + 1) * sizeof(long long));
    if (!file_desc) MyExit(NULL, tab, no_items, file_desc);

    FILE* pf = fopen(filename, "wb");
    if (!pf) MyExit(pf, tab, no_items, file_desc);

    if (fwrite(&no_it, sizeof(unsigned int), 1, pf) != 1) MyExit(pf, tab, no_items, file_desc);

    fseek(pf, (no_items + 1) * sizeof(long long), SEEK_CUR);

    for (size_t it = 0; it < no_items; ++it) {
        file_desc[it] = ftell(pf);
        if (fwrite(&tab[it], sizeof(MY_STUD), 1, pf) != 1) MyExit(pf, tab, no_items, file_desc);
        if (fwrite(tab[it].nazwisko, sizeof(char), tab[it].len, pf) != tab[it].len) MyExit(pf, tab, no_items, file_desc);
    }
    file_desc[no_items] = ftell(pf);

    fseek(pf, sizeof(unsigned int), SEEK_SET);
    if (fwrite(file_desc, sizeof(long long), no_items + 1, pf) != no_items + 1) MyExit(pf, tab, no_items, file_desc);

    fclose(pf);
    free(file_desc);
}

MY_STUD* Read(MY_STUD* tab, size_t* no_items, const char* filename) {
    if (tab) tab = FreeArray(tab, *no_items);

    unsigned int no_it = 0;
    long long* file_desc = NULL;

    FILE* pf = fopen(filename, "rb");
    if (!pf) MyExit(pf, tab, *no_items, file_desc);

    if (fread(&no_it, sizeof(unsigned int), 1, pf) != 1) MyExit(pf, tab, *no_items, file_desc);

    *no_items = no_it;
    tab = CreateArray(*no_items);
    file_desc = (long long*)malloc((*no_items + 1) * sizeof(long long));
    if (!tab || !file_desc) MyExit(pf, tab, *no_items, file_desc);

    if (fread(file_desc, sizeof(long long), *no_items + 1, pf) != *no_items + 1) MyExit(pf, tab, *no_items, file_desc);

    for (unsigned int it = 0; it < *no_items; ++it) {
        unsigned int rec = *no_items - it - 1;
        fseek(pf, file_desc[rec], SEEK_SET);
        if (fread(&tab[rec], sizeof(MY_STUD), 1, pf) != 1) MyExit(pf, tab, *no_items, file_desc);

        tab[rec].nazwisko = (char*)malloc(tab[rec].len * sizeof(char));
        if (!tab[rec].nazwisko) MyExit(pf, tab, *no_items, file_desc);

        if (fread(tab[rec].nazwisko, sizeof(char), tab[rec].len, pf) != tab[rec].len) MyExit(pf, tab, *no_items, file_desc);
    }

    free(file_desc);
    fclose(pf);

    return tab;
}

int main() {
    const char FileName[] = "MyFile.bin";
    size_t no_items = 5;

    MY_STUD* arr = CreateArray(no_items);
    if (!arr || !FillArray(arr, no_items)) {
        printf("Error filling array\n");
        return 1;
    }

    PrintArray(arr, no_items, "Tablica MY_STUD");
    Save(arr, no_items, FileName);
    arr = FreeArray(arr, no_items);

    MY_STUD* brr = Read(NULL, &no_items, FileName);
    PrintArray(brr, no_items, "Tablica MY_STUD po odczycie");
    brr = FreeArray(brr, no_items);

    return 0;
}
