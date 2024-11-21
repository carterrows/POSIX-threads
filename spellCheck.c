// Assignment 2 - CIS*3110
// Carter Rows - 1170615

#include "cartersUtilities.h"

int is_valid_file(char *fileName) {
    // check for valid input
    if(fileName == NULL) {
        fprintf(stderr, "NULL filename in 'is_valid_file()'\n");
        return 0;
    }
    
    // try to open the file
    FILE *testfp = fopen(fileName, "r");
    if(testfp == NULL) {
        fprintf(stderr, "Failed to open file:   '%s'\n", fileName);
        return 0;
    }

    // if we are here we have a valid file name
    fclose(testfp);
    return 1;
}

void *spell_checker_thread(void *arg) {
    // cast input data
    ThreadArgs *args = (ThreadArgs *)arg;

    // create our hash table to store all incorrectly spelt words
    WordCountPair **pardonMySpelling = init_hash_table(HASH_TABLE_SIZE);
    if(pardonMySpelling == NULL) {
        fprintf(stderr, "Failed to create the hash table.\n");
        return NULL;
    }

    // complete spell checking
    int incorrectCount = 0;
    char inputWordBuff[MAX_WORD_LEN];
    FILE *inputFp = fopen(args->inputFileName, "r");

    // loop through all input words
    while(fscanf(inputFp, "%63s", inputWordBuff) != EOF) {
        if(check_word_spelling(inputWordBuff, args->dictionaryFileName) == 0) {
            // update this word in our hash table
            if(update_word(pardonMySpelling, HASH_TABLE_SIZE, inputWordBuff, LOCAL_TABLE) == 0) {
                fprintf(stderr, "Failed to keep track of '%s' in local table.\n", inputWordBuff);
            }

            if(update_word(globalTable.table, HASH_TABLE_SIZE, inputWordBuff, GLOBAL_TABLE) == 0) {
                fprintf(stderr, "Failed to keep track of '%s' in global table.\n", inputWordBuff);
            }
            
            // keep track of how many incorrectly spelt words we find
            incorrectCount++;
        }
    }

    // done with the file pointer
    fclose(inputFp);

    // find 3 most common words and total spelling mistakes
    TopThree toFind;
    find_top_three(pardonMySpelling, HASH_TABLE_SIZE, &toFind);

    // now we have the top 3 words in toFind
    // send this data back to the main process with the message queue
    send_message(args->q, &toFind, args->inputFileName, incorrectCount);

    // clean up
    delete_table(pardonMySpelling, HASH_TABLE_SIZE);

    // increment total completed threads
    pthread_mutex_lock(&completedThreadsMutex);
    completedThreads++;
    pthread_mutex_unlock(&completedThreadsMutex);

    // create the string to print the return message
    char returnMsg[MAX_RETURN_MSG_LEN];
    sprintf(returnMsg, "\nThread ID '%lu' has finished processing file '%s'\n", pthread_self(), args->inputFileName);
    
    // print the message to signal completion
    ubWrite(returnMsg);

    return NULL;
}

int check_word_spelling(char *check, char *dictionaryFileName) {
    // open the dictionary
    // this should always work since we check for valid file name in main
    FILE *fp = fopen(dictionaryFileName, "r");

    string_to_lower(check);
    strip_delimiter(check);

    // buffer for checking
    char buffer[MAX_WORD_LEN];

    while(fscanf(fp, "%63s", buffer) != EOF) {
        string_to_lower(buffer);
        if(!strncmp(check, buffer, MAX_WORD_LEN)) {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

void find_top_three(WordCountPair **table, int size, TopThree *found) {
    // place to store words and counts
    char word1[MAX_WORD_LEN];
    char word2[MAX_WORD_LEN];
    char word3[MAX_WORD_LEN];
    int top1 = 0;
    int top2 = 0;
    int top3 = 0;
    word1[0] = word2[0] = word3[0] = '\0';
    
    // search through the entier table and update top 3 counts
    for(int i = 0; i < size; i++) {
        // check if we have a NULL index first
        if(table[i] == NULL) {
            continue;
        }
        
        if(table[i]->count > top1) {
            // move #2 -> #3, move #1 -> #2, update #1
            // move #2 -> #3
            strncpy(word3, word2, MAX_WORD_LEN);
            top3 = top2;

            // move #1 -> #2
            strncpy(word2, word1, MAX_WORD_LEN);
            top2 = top1;

            // update #1
            strncpy(word1, table[i]->word, MAX_WORD_LEN);
            top1 = table[i]->count;
        } 
        else if(table[i]->count > top2) {
            // move #2 -> #3, update #2
            // move #2 -> #3
            strncpy(word3, word2, MAX_WORD_LEN);
            top3 = top2;

            // update #2
            strncpy(word2, table[i]->word, MAX_WORD_LEN);
            top2 = table[i]->count;
        } 
        else if(table[i]->count > top3) {
            // update #3
            strncpy(word3, table[i]->word, MAX_WORD_LEN);
            top3 = table[i]->count;
        }
    }

    // copy what we found into the given struct
    strncpy(found->word1, word1, MAX_WORD_LEN);
    strncpy(found->word2, word2, MAX_WORD_LEN);
    strncpy(found->word3, word3, MAX_WORD_LEN);
    found->count1 = top1;
    found->count2 = top2;
    found->count3 = top3;
}

void string_to_lower(char *string) {
    for(int i = 0; i < strlen(string); i++) {
        string[i] = tolower(string[i]);
    }
}

void strip_delimiter(char *string) {
    // remove delimiter at the end of the string
    int lastCharIndex = strlen(string) - 1;
    if(is_delimiter(string[lastCharIndex])) {
        string[lastCharIndex] = '\0';
    }

    // remove delimiter at the start of the string
    if(is_delimiter(string[0])) {
        for(int i = 0; i < strlen(string) - 1; i++) {
            string[i] = string[i+1];
        }
        string[strlen(string) - 1] = '\0';
    }
}

int is_delimiter(char check) {
    // compare the character to our delimiter list
    for(int i = 0; i < strlen(delimiters); i++) {
        if(check == delimiters[i]) {
            return 1;
        }
    }
    return 0;
}
