// Assignment 2 - CIS*3110
// Carter Rows - 1170615

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>

#ifndef USEFUL_FUNCTIONS_HEADER
#define USEFUL_FUNCTIONS_HEADER

// DEFINITIONS

#define MAX_WORKER_THREADS 256
#define SUMMARY_TO_FILE 1
#define SUMMARY_TO_STDOUT 0
#define MAX_FILENAME 64
#define MAX_WORD_LEN 64
#define LOCAL_TABLE 0
#define GLOBAL_TABLE 1
#define MAX_RETURN_MSG_LEN 256

// for hash function
#define PRIME 0x01000193
#define OFFSET 0x811c9dc5
#define HASH_TABLE_SIZE 1024

// STRUCTURES

// struct for top three words and their counts
typedef struct {
    char word1[MAX_WORD_LEN];
    char word2[MAX_WORD_LEN];
    char word3[MAX_WORD_LEN];
    int count1;
    int count2;
    int count3;
} TopThree;

////////////////////////////////////////////////////

typedef struct Message {
    char fileName[MAX_FILENAME];
    TopThree found;
    int totalIncorrect;
} Message;

// Message node
typedef struct message_node {
    Message msg;
    struct message_node *next;
} MessageNode;

// Message queue - a singly linked list
// Remove from head, add to tail
typedef struct {
    MessageNode *head;
    MessageNode *tail;
    pthread_mutex_t mutex;
    
    // Add a condition variable
    pthread_cond_t cond;
} MessageQueue;

////////////////////////////////////////////////////

// // struct for the output file pointer
// typedef struct {
//     FILE *outputFp;
//     pthread_mutex_t mutex;
// } FilePointer;

// Arguments for a worker thread
typedef struct {
    MessageQueue *q;
    char dictionaryFileName[MAX_FILENAME];
    char inputFileName[MAX_FILENAME];
} ThreadArgs;

// struct for hash table
typedef struct {
    char word[MAX_WORD_LEN];
    int count;
} WordCountPair;

// hash table and mutex encapsulated
typedef struct {
    WordCountPair **table;
    pthread_mutex_t mutex;
} TableWithMutex;

// EXTERNAL VARIABLES
extern TableWithMutex globalTable;
extern pthread_mutex_t completedThreadsMutex;
extern int completedThreads;
extern char delimiters[];

// PROTOTYPES

// MESSAGE QUEUE

/**
 * @brief Creates the message queue
 * @return Message queue pointer on success, NULL on failure
*/
MessageQueue *create_message_queue();

/**
 * @brief Send a message to the message queue
 * @param q Message queue
 * @param found Pointer to a TopThree structure
 * @param fileName File name processed by the thread
 * @param totalIncorrect Count of spelling errors found by thread
*/
void send_message(MessageQueue *q, TopThree *found, char *fileName, int totalIncorrect);

/**
 * @brief Get a message from the queue
 * @param q Message queue
 * @param msg_out Pointer to address the message is to be copied
 * @return 1 on success, 0 on failure
*/
int get_message(MessageQueue *q, Message *msg_out);

// SPELL CHECK

/**
 * @brief Function for the worker threads
 * @param arg Thread arguments
*/
void *spell_checker_thread(void *arg);

/**
 * @brief Check if a given file name is valid
 * @param fileName File name to be checked
 * @return 1 on valid file, 0 on bad file name
*/
int is_valid_file(char *fileName);

/**
 * @brief Check if a given word is spelt correctly based on a given dictionary file
 * @param check Word to be checked
 * @param dictionaryFileName File name of the dictionary file
 * @return 1 on correctly spelt, 0 on incorrectly spelt
*/
int check_word_spelling(char *check, char *dictionaryFileName);

/**
 * @brief Find the top three misspelt words in a given hash table
 * @param table The hash table of incorrectly spelt words
 * @param size Size of the hash table
 * @param found Pointer of TopThree struct to store top three incorrectly spelt words
*/
void find_top_three(WordCountPair **table, int size, TopThree *found);

/**
 * @brief Convert a given string to lowercase
 * @param string String to convert
*/
void string_to_lower(char *string);

/**
 * @brief Strip delimiters from the start and end of a string
 * @param string String to be stripped
*/
void strip_delimiter(char *string);

/**
 * @brief Check if a given string is a delimiter
 * @param check Character to be checked
 * @return 1 on delimiter, 0 otherwise
*/
int is_delimiter(char check);

// HASH TABLE

/**
 * @brief Hash function to get an index for the hash table
 * @param word Word to hash
 * @param size Size of the hash table
 * @return Hash table index
*/
unsigned int hash(char *word, int size);

/**
 * @brief Initialize / allocate a hash table
 * @param size Size of hash table to be allocated
 * @return Pointer to the hash table on success, NULL on failure
*/
WordCountPair **init_hash_table(int size);

/**
 * @brief Update the hash table with a word
 * @param table Hash table
 * @param size Size of hash table
 * @param word Word to be added or updated in the table
 * @param destination This determines if we are updating the local or global table
 * @return 1 on success, 0 on failure
*/
int update_word(WordCountPair **table, int size, char *word, int destination);

/**
 * @brief Create a pair with a given word
 * @param word The word to be made into a pair
 * @return Pointer to the pair on success, NULL on failure
*/
WordCountPair *create_pair(char *word);

/**
 * @brief Free all memory of a given hash table
 * @param table Hash table to be unallocated
 * @param size Hash table size
*/
void delete_table(WordCountPair **table, int size);

// PRINTING

/**
 * @brief Simply display the menu to stdout
*/
void display_menu();

/**
 * @brief Write the summary to the output file
 * @param fp File pointer
 * @param checkedFile File name of the threads checked file
 * @param count Number of incorrectly spelt words found in the thread
 * @param summary Pointer to a TopThree struct with info of top three misspelt words
*/
void write_to_file(FILE *fp, char *checkedFile, int count, TopThree *summary);

/**
 * @brief Print the final summary to given file stream
 * @param stream File stream to print to
 * @param nFiles Total files processed by the program
 * @param count Number of total inccorectly spelt words 
 * @param summary Pointer to a TopThree struct with info of top three misspelt words
*/
void print_summary(FILE *stream, int nFiles, int count, TopThree *summary);

/**
 * @brief Unbuffered printing to STDOUT
 * @param string String to be printed to STDOUT
*/
void ubWrite(char *string);

#endif // USEFUL_FUNCTIONS_HEADER
