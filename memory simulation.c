//  CITS2002 Project 2 2024
//  Student1:   24214099   Lucan McDonald
//  Student2:   24502509   Anthony Stewart
//  Platform:   Apple

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>

#define RAMSIZE 16
#define VMEMSIZE 32
#define FRAMES 8
#define FRAMESIZE 2
#define PAGESIZE 2
#define PROCESSES 4
#define PAGEPERPROCESS 4
#define DLOC 99



// structure used to represent a memory page
typedef struct {
    int process_id;
    int page_num;
    int last_accessed;
} memory;

 // Intialises virtualmemory array to store 32 pointers to the memory struct
struct memory *vmem[VMEMSIZE];

// Intialises RAM array to store 16 pointers to memory struct
struct memory *RAM[RAMSIZE];



void buildVirtualMemory () {
    // this will just load everyting up and set the clock etc...

    for (int id=0; id < PROCESSES; id++) {
        for (int page=0; page < PAGEPERPROCESS; page++) {
            for (int bit=0; bit < PAGESIZE; bit++) {
                
                // Use malloc to dynamiclally allocate memory for each entry in the memory struct
                // Entry is simply a temporary pointer to memory that will later be assigned to the vmem array
                memory* entry = (memory*) malloc(sizeof(memory));

                // Checks if malloc failed
                if (entry==NULL) {
                    fprintf(stderr, "malloc failed\n");
                    return EXIT_FAILURE;
                }

                // Each entry stores the processid, pagenum and last_accessed value for each bit in the virtual memory
                entry->process_id = id;
                entry->page_num = page;
                entry->last_accessed = 0;

                 // The index is calculated to assign 32 entires for the 32 bits inside virtual memory
                int index = id*PAGEPERPROCESS*PAGESIZE + page*PAGESIZE + bit;

                // Now we make the vmem array at the current bits index points to the bits entry
                vmem[index] = entry;
            }
        }
    }
}

void processNewMemory (int processID) {
    
    // bool value that becomes true if the allocation was succesful
    bool succesful_alloc = false;

    // Initialises empty RAM index at -1 to indicate that the RAM is full
    int empty_RAM_index = -1;

    // First we check if we have enough space to store it in RAM
    for (int page=0; page < FRAMES; page+=2) {
 
        // Checks if there are any empty pages in RAM and keeps the index in a variable if it does
        if (RAM[page] == NULL) {
            empty_RAM_index = page;
            break;
        }
    }

    // Check the first entry of every page in the process to see which ones have already been allocated
    // Only does this if the RAM also had free space
    if (empty_RAM_index != -1) {
        for (int process=0; process < PROCESSES && !succesful_alloc; process++) {
            for (int page=0; page < PAGEPERPROCESS && !succesful_alloc; page++) {
                int index = process*PAGEPERPROCESS*PAGESIZE + page*PAGESIZE;
                // We use entry again as a temporary pointer to the memory struct
                    memory *entry = vmem[index];

                // Checks for empty vmem for the process
                if (entry->last_accessed == 0 && entry->process_id == processID) {
                    RAM[empty_RAM_index] = entry;

                    succesful_alloc = true;
                }
            }
        }
    }
}

int lru () {

    // does LRU shit idk
}



// runSim runs processes on the simulated memory pertaining to the provided input file, writing results to out.txt
void runSim (const char *inFile, const char *outFile) {

    FILE* in_fp = fopen(inFile, "r");
    FILE* out_fp = fopen(outFile, "w");

    // see how deep the rabbit hole goes
    buildVirtualMemory();
}



// Main
int main(int argc, char *argv[]) {

    // checks that number of args is correct
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    runSim(argv[1], argv[2]);

    return 0;
}
