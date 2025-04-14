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
#define MAX_LAST_ACCESSED 1000



// Structure used to represent a memory page
typedef struct {
    int process_id;
    int page_num;
    int last_accessed;
} memory;

 // Intialises virtualmemory array to store 32 pointers to the memory struct
struct memory *vmem[VMEMSIZE];

// Intialises RAM array to store 16 pointers to memory struct
struct memory *ram[RAMSIZE];

// Global time variable for last_accesed variable
int clock_time = 0;

// Page tables for each of the four processes initialised as 99 indicating that they are all in disc stroage
int page_tables [PROCESSES][PAGEPERPROCESS] = {99};
 
// These variables used to track the page_nums of pages to be stored in page_tables
// new_page_num stores the page num for pages that have just been moved into RAM
int new_page_num;
// old_page_num stores the page num for pages that have just been moved into disc
int old_page_num;

void buildVirtualMemory () {
    // this will just load everyting up and set the clock etc...

    for (int id=0; id < PROCESSES; id++) {
        for (int page=0; page < PAGEPERPROCESS; page++) {
            for (int bit=0; bit < PAGESIZE; bit++) {
                
                // Use malloc to dynamiclally allocate memory for each entry in the memory struct
                // Entry is simply a temporary pointer to memory that will later be assigned to the vmem array
                struct memory *entry = (struct memory *)malloc(sizeof(memory));
                // memory* entry = (memory*) malloc(sizeof(memory));

                // Checks if malloc failed
                if (entry==NULL) {
                    fprintf(stderr, "malloc failed\n");
                    exit(EXIT_FAILURE);
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
    bool sufficient_ram = false;
    // vmemfull intialised as true for simplicity
    bool vmem_full = true;

    clock_time++;
    int new_page_num = -1;
    int old_page_num = -1;
    int empty_ram_index;

    // First we check if we have enough space to store it in RAM
    for (int frame=0; frame < FRAMES; frame+=2) {
 
        // Checks if there are any empty pages in RAM and keeps the index in a variable if it does
        if (ram[frame] == NULL) {
            empty_ram_index = frame;
            sufficient_ram = true;
            break;
        }
    }


    memory *empty_vmem_entry;
    int empty_vmem_index;

    // Check the first entry of every page in the process to see which ones have already been allocated
    for (int page=0; page < PAGEPERPROCESS && !sufficient_ram; page++) {
        int index = processID*PAGEPERPROCESS*PAGESIZE + page*PAGESIZE;

        // We use entry again as a temporary pointer to the memory struct
        memory *entry = vmem[index];

        // Checks for empty vmem for the process
        if (entry->last_accessed == 0 && entry->process_id == processID) {
            // The RAM array at the empty index is assigned to a pointer to the location of the vmem in the memory struct
            empty_vmem_entry = entry;
            empty_vmem_index = index;
            entry->last_accessed = clock_time;
            vmem_full = false;

            // Stores page number for later page table updates
            new_page_num = page;
        }
    }
    // Only reallocates if there isn't sufficent memory in RAM and if the virtual memory isn't already full
    if (!sufficient_ram && !vmem_full) {
        empty_vmem_index = deallocate();
    }

    // Allocates both of the bits for the page
    ram[empty_vmem_index] = empty_vmem_entry;
    ram[empty_vmem_index+1] = empty_vmem_entry;

    if (new_page_num != -1) {
        // Updates new memory to indicate what frame its stored in
        page_tables[processID][new_page_num] = empty_vmem_index/FRAMESIZE;
    }

    if (old_page_num != -1) {
        // Updates old memory to indicate that it is now in disk
        page_tables[processID][old_page_num] = 99;
    }
}



int deallocate () {
    // Identifies the index of the least recently accessed page in RAM
    int lru_index =  lru();

    // We then find the page num of the frame we are removing to later update page tables
    memory *entry = ram[lru_index];
    entry->last_accessed = clock_time;
    old_page_num = entry->page_num; 
    
    for (int i=0; i<2; i++) {
        // Resets last_accessed value for the lru page
        memory *entry = ram[lru_index+i];
        entry->last_accessed = clock_time;

         // Frees the two bits of the page in the ram
        ram[lru_index+i] = NULL;
    }
    return lru_index;
}

int lru () {

    // Initialises variable for  the index and the clock time of the lru page in RAM
    int last_accessed_index;
    int min_last_accessed = MAX_LAST_ACCESSED;

    for (int frame=0; frame < FRAMES; frame+=2) {

        // Temporary entry storing memory stuct
        memory *entry = ram[frame];

        // Finds the lru and assigns the lru index
        if (entry->last_accessed < min_last_accessed) {
            min_last_accessed = entry->last_accessed;
            last_accessed_index = frame;
        }
    }

    // Returns the lru page
    return last_accessed_index;
}

// runSim runs processes on the simulated memory pertaining to the provided input file, writing results to out.txt
void runSim (const char *inFile, const char *outFile) {

    FILE* in_fp = fopen(inFile, "r");
    FILE* out_fp = fopen(outFile, "w");

    // see how deep the rabbit hole goes
    buildVirtualMemory();

    int processID;

    while (fscanf(in_fp, "%d", &processID) == 1) {
        processNewMemory(processID);
        printf("%d", processID);
    }
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
