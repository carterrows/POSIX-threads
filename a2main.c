// Assignment 2 - CIS*3110
// Carter Rows - 1170615
// DUE DATE: March 27th @23:59

#include "cartersUtilities.h"

// hash table for all words found by all threads
TableWithMutex globalTable;

// count of completed threads
int completedThreads = 0;
pthread_mutex_t completedThreadsMutex;

// list of delimiters
char delimiters[] = ";.,/:;'()?!";

int main(int argc, char **argv) {
    // variables for operation
    int cmdOption = SUMMARY_TO_STDOUT;

    // check for invalid command line arguments
    if(argc > 2) {
        // incorrect program usage
        fprintf(stderr, "Invalid cmd args.\nUSAGE:   ./A2checker [-l]\n\n");
        exit(1);
    }

    // check for cmd line option -l
    if(argc == 2) {
        if(argv[1][0] == '-' && argv[1][1] == 'l') {
            // valid cmd arg, update cmdOption variable
            cmdOption = SUMMARY_TO_FILE;
        } else {
            fprintf(stderr, "Unrecognized option: '%s'\n", argv[1]);
            exit(1);
        }
    }

    // create the array for worker ids and arguments
    pthread_t tid[MAX_WORKER_THREADS];
    ThreadArgs args[MAX_WORKER_THREADS];

    // create the message queue
    MessageQueue *q = create_message_queue();

    // create some attributes for the threads
    pthread_attr_t attributes;

    // initialize thread attributes. 
    pthread_attr_init(&attributes);

    // make thread detached in the thread attributes
    pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);

    // initialize our global table
    globalTable.table = init_hash_table(HASH_TABLE_SIZE);
    if(globalTable.table == NULL) {
        fprintf(stderr, "Failed to allocate hash table for global.\n");

        // clean up before exitting
        free(q);
        pthread_attr_destroy(&attributes);

        exit(1);
    }

    // init mutex
    pthread_mutex_init(&globalTable.mutex, NULL);
    pthread_mutex_init(&completedThreadsMutex, NULL);

    // now loop until user chooses exit
    int userOption;
    int threadCount = 0;
    char dictionaryBuffer[MAX_FILENAME];
    char inputFileBuffer[MAX_FILENAME];
    while(1) {
        // check for max threads
        if(threadCount >= MAX_WORKER_THREADS) {
            fprintf(stderr, "Max threads reached. Exitting.\n");
            break;
        }

        // print main menu
        display_menu();
        scanf("%d", &userOption);

        // check if we are starting new task or exitting
        if(userOption == 1) {
            // ask user for dictionary file
            printf("\nEnter the dictionary file name, or type 'return' to go back to main menu:\t");
            scanf("%64s", dictionaryBuffer);
            if(!strncmp(dictionaryBuffer, "return", MAX_WORD_LEN)) {
                // just skip the loop to go back to main menu
                continue;
            }

            // do the same for input file name
            printf("Enter the input file name, or type 'return' to go back to main menu:\t\t");
            scanf("%64s", inputFileBuffer);
            if(!strncmp(inputFileBuffer, "return", MAX_WORD_LEN)) {
                // just skip the loop to go back to main menu
                continue;
            }

            // check if we have valid file names
            if(!is_valid_file(dictionaryBuffer)) {
                fprintf(stderr, "Invalid file name:   '%s'\n", dictionaryBuffer);
                continue;
            }
            if(!is_valid_file(inputFileBuffer)) {
                fprintf(stderr, "Invalid file name:   '%s'\n", inputFileBuffer);
                continue;
            }

            // if we are here, both input and dictionary file names are valid
            // create a process to spellcheck
            strncpy(args[threadCount].dictionaryFileName, dictionaryBuffer, MAX_WORD_LEN);
            strncpy(args[threadCount].inputFileName, inputFileBuffer, MAX_WORD_LEN);
            args[threadCount].q = q;
            pthread_create(&tid[threadCount], &attributes, spell_checker_thread, &args[threadCount]);

            // keep track of how many threads we have spawned
            threadCount++;
        }
        else if(userOption == 2) {
            // get out of the loop
            break;
        }
        else {
            fprintf(stderr, "Invalid choice. Please select either 1 or 2.\n");
        }
    }

    // exitting logic here
    int messagesRecieved = 0;
    int totalMisspellings = 0;

    // open the file pointer
    FILE *outputFp = fopen("crows_A2.out", "w");
    if(outputFp == NULL) {
        fprintf(stderr, "Failed to open output file.\n");

        // clean up before exitting
        free(q);
        delete_table(globalTable.table, HASH_TABLE_SIZE);
        pthread_attr_destroy(&attributes);

        exit(1);
    }

    // lock all mutexes now that we have exitted
    pthread_mutex_lock(&completedThreadsMutex);
    pthread_mutex_lock(&globalTable.mutex);

    // check how many threads are finished / still running
    int unfinishedThreads = threadCount - completedThreads;
    if(unfinishedThreads > 0) {
        printf("\n%d threads are still running.\n", unfinishedThreads);
    } else {
        printf("\nAll threads have completed their tasks.\n");
    }

    // get messages from threads
    while(messagesRecieved < completedThreads) {
        Message msg;
        if(!get_message(q, &msg)) {
            fprintf(stderr, "Failed to recieve message #%d\n", messagesRecieved);
        }

        // write the summary to output file
        write_to_file(outputFp, msg.fileName, msg.totalIncorrect, &(msg.found));
        totalMisspellings += msg.totalIncorrect;
        
        messagesRecieved++;
    }

    // get stuff from global table
    TopThree total;
    find_top_three(globalTable.table, HASH_TABLE_SIZE, &total);

    // now check the cmdOption to see if we put this info in the file or screen
    if(cmdOption == SUMMARY_TO_STDOUT) {
        print_summary(stdout, completedThreads, totalMisspellings, &total);
    } else if(cmdOption == SUMMARY_TO_FILE) {
        print_summary(outputFp, completedThreads, totalMisspellings, &total);
    }

    // free everything we dont need anymore
    free(q);
    delete_table(globalTable.table, HASH_TABLE_SIZE);
    fclose(outputFp);
    pthread_attr_destroy(&attributes);

    exit(0);
}
