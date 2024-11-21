// Assignment 2 - CIS*3110
// Carter Rows - 1170615

#include "cartersUtilities.h"

unsigned int hash(char *word, int size) {
    // i am stealing this hash function from an
    // assignment i wrote in data structures :)
    // it is the FNV-1a hash algorithm
    unsigned int hashIndex = OFFSET;
    for(int i = 0; i < strlen(word); i++) {
        hashIndex ^= word[i];
        hashIndex *= PRIME;
    }

    return hashIndex % size;
}

WordCountPair **init_hash_table(int size) {
    // allocate the hash table
    WordCountPair **newTable = (WordCountPair **)malloc(sizeof(WordCountPair *) * size);
    if(newTable == NULL) {
        fprintf(stderr, "Failed to allocate hash table memory.\n");
        return NULL;
    }

    // default all to NULL
    for(int i = 0; i < size; i++) {
        newTable[i] = NULL;
    }

    return newTable;
}

int update_word(WordCountPair **table, int size, char *word, int destination) {
    // lock the mutex if we are accessing global table
    if(destination == GLOBAL_TABLE) pthread_mutex_lock(&globalTable.mutex);
    
    // get the words hash index
    int tableIndex = hash(word, size);

    // hopefully we can simply insert right at the hash index every time
    if(table[tableIndex] == NULL) {
        // this word is brand new
        table[tableIndex] = create_pair(word);

        // unlock the mutex
        if(destination == GLOBAL_TABLE) pthread_mutex_unlock(&globalTable.mutex);
        return 1;
    }
    else if(!strncmp(table[tableIndex]->word, word, MAX_WORD_LEN)) {
        // the word is already here, update the count
        table[tableIndex]->count++;

        // unlock the mutex
        if(destination == GLOBAL_TABLE) pthread_mutex_unlock(&globalTable.mutex);
        return 1;
    }
    else {
        // linear probe until we find the word or an empty location
        for(int i = tableIndex + 1; i < size; i++) {
            // check for empty spot
            if(table[i] == NULL) {
                // insert the pair here
                table[i] = create_pair(word);

                // unlock the mutex
                if(destination == GLOBAL_TABLE) pthread_mutex_unlock(&globalTable.mutex);
                return 1;
            }

            // check if the word is already here
            if(table[i] != NULL && !strncmp(table[i]->word, word, MAX_WORD_LEN)) {
                // found it, update the count
                table[i]->count++;

                // unlock the mutex
                if(destination == GLOBAL_TABLE) pthread_mutex_unlock(&globalTable.mutex);
                return 1;
            }
        }
    }
    // if we are here, somehow we did not update the word in our table
    // unlock the mutex
    if(destination == GLOBAL_TABLE) pthread_mutex_unlock(&globalTable.mutex);
    return 0;
}

WordCountPair *create_pair(char *word) {
    // allocate memory for the pair
    WordCountPair *newPair = (WordCountPair *)malloc(sizeof(WordCountPair));
    if(newPair == NULL) {
        fprintf(stderr, "Failed to allocate word count pair memory.\n");
        return NULL;
    }

    // set the values
    strncpy(newPair->word, word, MAX_WORD_LEN);
    newPair->count = 1;

    return newPair;
}

void delete_table(WordCountPair **table, int size) {
    // free the entire table contents if not NULL
    for(int i = 0; i < size; i++) {
        if(table[i] != NULL) {
            free(table[i]);
        }
    }

    // then finally free the table itself
    free(table);
}
