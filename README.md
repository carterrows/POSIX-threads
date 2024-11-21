# README for Assignment 2 - Carter Rows (1170615)

## State of implementation
Everything is working as expected. The -l command line option works correctly by outputting the final summary to the output file.
The threads successfully find the incorrectly spelt words and update hash tables accordingly.

When running my program, processing 4 input files all against the dictionary file american-english, processing times were found to be:
- 23 seconds for normal execution (no valgrind)
- 70 seconds using valgrind
- 101 seconds using valgrind's DRD tool

This is just to give you an expected timeframe of how long you will be sitting there waiting for my program to finish haha.

## How to run
Use make A2checker to build the executable, and make clean to remove object files and the executable.
I also added an unbuffered print statement at the end of the thread's function to indicate when each thread has completed execution. This way when you are marking you can easily tell when all of the threads have completed execution.

## Thread functionality
The threads get an input file and dictionary file as input, calculate which words found in the input file are misspelt, and then creates a message with this summarized data and adds it to the message queue. The thread also updates a global hash table along with its own local table whenever it finds a misspelt word. This allows me to simply get a summary from the global table in my main thread at the end of execution for the final summary.

Each thread does not do any file printing, because the way I understood the project outline was that the threads would add their results to the queue, and the main thread prints these summaries to the output file once the user exits. If I wanted to have the threads print this summary data to the file themselves, then there would be no need for a message queue.

## Hash table implementation
I decided to use a hash table to keep track of misspelt words, simply because of the speed advantage over a traditional array. I made the table size 1024, which makes collisions very unlikely, but in the case of a collision, I implement linear probing. Look at my update_word function in hashTable.c to see how I did linear probing / updating.

Like I said, each thread gets its own local hash table, and then I allocate one global hash table for all threads to constantly update during runtime. This of course is protected by a mutex because updating the global table is obviously a critical section.

## Memory management
Since I dynamically allocate my hash tables and the message queue, I have also correctly freed this memory in the thread function and main thread.
I believe I have tested all scenarios with valgrind, so hopefully none appear when you test my code.
