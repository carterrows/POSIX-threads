// Assignment 2 - CIS*3110
// Carter Rows - 1170615

#include "cartersUtilities.h"

void display_menu() {
    printf("\nMAIN MENU:\n");
    printf("1.  Start a new spellchecking task\n");
    printf("2.  Exit\n");
}

void write_to_file(FILE *fp, char *checkedFile, int count, TopThree *summary) {
    // check the file pointer
    if(fp == NULL) {
        fprintf(stderr, "Failed to open output file.\n");
        return;
    }

    // print summary to the file
    // but first check if we have no spelling mistakes
    if(strlen(summary->word1) == 0 && strlen(summary->word2) == 0 && strlen(summary->word3) == 0) {
        // no misspelt words
        fprintf(fp, "%s %d (no misspelt words)\n", checkedFile, count);
    }
    else {
        // we have at least one misspelt word
        fprintf(fp, "%s %d %s %s %s\n", checkedFile, count, summary->word1, summary->word2, summary->word3);
    }
}

void print_summary(FILE *stream, int nFiles, int count, TopThree *summary) {
    // check for NULL stream
    if(stream == NULL) {
        fprintf(stderr, "NULL stream in print_summary.\n");
        return;
    }

    // write the info to the file stream
    // whether that be stdout or the output file
    // check if we have no misspelt words first
    if(strlen(summary->word1) == 0 && strlen(summary->word2) == 0 && strlen(summary->word3) == 0) {
        // no misspelt words
        fprintf(stream, "\nSPELLCHECK SUMMARY:\n");
        fprintf(stream, "Number of files processed:\t%d\n", nFiles);
        fprintf(stream, "Number of spelling errors:\t%d\n", count);
        fprintf(stream, "Three most common misspellings:\t(no misspelt words)\n");
    }
    else {
        // we have at least one misspelt word
        fprintf(stream, "\nSPELLCHECK SUMMARY:\n");
        fprintf(stream, "Number of files processed:\t%d\n", nFiles);
        fprintf(stream, "Number of spelling errors:\t%d\n", count);
        fprintf(stream, "Three most common misspellings:\t%s (%d times), %s (%d times), %s (%d times)\n", summary->word1, summary->count1, summary->word2, summary->count2, summary->word3, summary->count3);
    }
}

void ubWrite(char *string){
    write(STDOUT_FILENO, string, strlen(string)+1);
}
