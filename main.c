#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_NUM_TOKENS 1000
#define MAX_LINES 100
#define MAX_SPACE 40
#define MAX_BLOCKS MAX_SPACE / 4
#define tokenSeparators ", "

typedef struct Entry {
    int file;
    int start;
    int end;
    struct Entry* next; // to link to the next directory entry
} Entry;


/* FUNCTION PROTOTYPES */
int tokenise(char line[], char* token[]);
int checkSpace(int space, int array[]);
void printSys();
void printDir(Entry* head);

/* Soz I need this to be global cos checkSpace() needs to access it */
int fileSystem[MAX_SPACE] = { 0 };          // memory


int main()
{   

    /* DIRECTORY AND MEMORY OBJECTS */
    /* I know the array is unnecessary, but at 2am my brain isnt
    entirely working very well to optimise shit so i just did 
    whatever worked */
    struct Entry directory[MAX_SPACE];      // array of directory entries
    Entry* head = NULL;                     // head of linked list
    int entries = 0;                        // keeps track of number of entries
    int blkCount;                           // number of blocks needed 
    int tokCount;                           // keeps track of file data
    int arrCount;                           // keeps track of index

    /* INPUT OBJECTS */ 
    char file[30];                          // for input string
    int i = 0, itemCount = 0;               // number of items currently in list out of MAX_SIZE
    char *input[MAX_LINES];                 // maximum number of instructions to be processed
    char *copy[MAX_LINES];                  // to copy the input line (otherwise menu gets messed up)
    char line[1024];                        // fgets buffer
    char *tok[MAX_NUM_TOKENS];              // maximum number of tokens for each line of instructions
    int len;                                // length for each tokenised line
    int exe;                                // its definitely used somewhere in this code trust me
    

    // input file 
    do {
        printf("\nEnter input file name (with .csv): ");
        scanf("%s", &file);
    } while (access(file, F_OK) != 0);

    FILE* stream = fopen(file, "r");

    // add items from buffer into input and increment itemCount for each item
    while (fgets(line, 1024, stream)) {
        input[i] = strdup(line);
        copy[i] = strdup(line);
        i++;
        itemCount++;
        char* tmp = strdup(line);
        free(tmp);
    }

    printf("File loaded!");

    do {
        printf("\n\nType index of line to execute: \n");
        for (int j = 1; j < itemCount; j++)
            printf("%d: %s", j, input[j]);
        printf("%d: view directory\n%d: view system\n%d: exit\n>> ", itemCount, itemCount + 1, itemCount + 2);
        scanf("%d", &exe);
        
        if (exe == itemCount)
            printDir(head);
        else if (exe == itemCount + 1)
            printSys();
        else if (exe < itemCount)
        {
            if (strcmp(input[exe], "NULL\n") == 0)
                continue;

            strcpy(copy[exe], input[exe]);  // duplicate string
            len = tokenise(copy[exe], tok); // tokenise string
            
            // ADDING
            if (strcmp(tok[0], "add") == 0)
            {
                blkCount = ((len - 1) / 3) + ((len - 1) % 3);   // determine number of blocks needed
                int blkArray[blkCount];                         // array of available blocks

                // check if there is enough space
                if (checkSpace(blkCount, blkArray) == 0)        
                {
                    tokCount = 1; // start from the second elent in tok[]

                    for (int i = 0; i < blkCount; i++)
                    {
                        arrCount = 0; // counter for index inside block array

                        // while not all the tokens are loaded
                        while (tokCount != len && arrCount < 3)
                        {
                            // loading into file system!!! yay
                            // printf("Putting %d in %d\n", atoi(tok[tokCount]), (blkArray[i] * 4) + arrCount);
                            fileSystem[(blkArray[i] * 4) + arrCount] = atoi(tok[tokCount]);
                            tokCount++;
                            arrCount++;
                        }

                        // if all data is loaded
                        if (tokCount == len)
                            printf("");
                        else
                        {
                            // linking!!!! woooo
                            // printf("Linking %d to %d\n", blkArray[i] + 3, blkArray[i + 1]);
                            fileSystem[(blkArray[i] * 4) + 3] = blkArray[i + 1]; // links to the next block
                        }
                    }

                    // updating directory
                    Entry entry = { atoi(tok[1]), blkArray[0], blkArray[blkCount - 1], NULL };
                    directory[entries] = entry;

                    // if the entry is the first entry, head = that entry
                    if (entries == 0)
                        head = &directory[entries];

                    // link prev entry to current entry 
                    if ((entries - 1) >= 0)
                        directory[entries - 1].next = &directory[entries]; 

                    entries++;

                    printf("\nEntry %d loaded!\n", exe);
                    strcpy(input[exe], "NULL\n");
                }
                else
                    printf("There isn't enough space in memory.\n");
            }

            // READING
            else if (strcmp(tok[0], "read") == 0)
            {
                // run time is like more than O(n) sorry
                Entry* curr = head;
                while (curr != NULL)
                {
                    if (atoi(tok[1]) - curr->file < 0)
                        curr = curr->next;
                    else
                    {
                        int block = curr->start;

                        while (1) // while true la ok everything also while true
                        {
                            for (arrCount = 0; arrCount < 3; arrCount++)
                            {
                                if (fileSystem[(block * 4) + arrCount] == atoi(tok[1]))
                                {
                                    printf("\nFile: %d is located at block %d, index %d.\n", atoi(tok[1]), block, (block * 4) + arrCount);
                                    break;
                                }
                            }   

                            if (block == curr->end)
                                break;
                            block = fileSystem[(block * 4) + 3]; // move on to the next block

                        }
                    }

                    break;
                }
            }

            // DELETING
            else if (strcmp(tok[0], "delete") == 0)
            {
                // run time is also O(n) sorry
                Entry* curr = head;
                Entry* prev = NULL;
                while (curr != NULL)
                {
                    if (curr->file != atoi(tok[1]))
                    {
                        prev = curr;
                        curr = curr->next;
                    }
                    else
                    {
                        int block = curr->start;

                        while (1)
                        {
                            // deleting
                            for (arrCount = 0; arrCount < 3; arrCount++)
                                fileSystem[(block * 4) + arrCount] = 0;

                            // if all data is deleted
                            if (block == curr->end)
                                break;
                            else
                                block = fileSystem[(block * 4) + 3]; // move on to the next block
                        }
                        
                        // updating directory
                        if (entries == 1)               // if entry is the only entry in directory
                            head = NULL;

                        else if (entries > 1)
                            prev->next = curr->next;    // link the entry behind to the entry ahead

                        entries--;

                        printf("\nEntry %d deleted!\n", curr->file);
                        break;
                    }
                }
            }
        }
    } while (exe != itemCount + 2);
    
    printf("\nBye!\n");
}


// grabbed from 1007 lab 4 question 14
int tokenise(char line[], char* token[])
{
    char *tk;
    int i = 0;
    
    tk = strtok(line, tokenSeparators);
    token[i] = tk;
    while (tk != NULL) 
    {
        i++;
        if (i >= MAX_NUM_TOKENS)
        {
            i = -1;
            break;
        }
        
        tk = strtok(NULL, tokenSeparators);
        token[i] = tk;
    }
    
    return i;
}


int checkSpace(int space, int array[])
{
    int counter = 0;

    for (int i = 0; i < MAX_BLOCKS; i++)
    {
        // printf("%d space: fileSystem[%d] is %d\n", space, i, fileSystem[i * 4]);
        if (fileSystem[i * 4] == 0)
        {   
            array[counter] = i; // record block number
            counter++;
            space--;
        }
        if (space == 0)
            return 0;
    }
    return -1;
}


void printSys()
{
    // display simulated filesystem
    printf("\n\n**** SYSTEM MEMORY ****\n");
    printf(" Index | Block | Data\n");
    for (int j = 0; j < MAX_SPACE; j++)
        printf("%6d |%5d  |%5d\n", j, j/4, fileSystem[j]);
}


void printDir(Entry* head)
{
    // display directory
    printf("\n\n****** DIRECTORY ******\n");
    printf("  File | Start | End\n");
    Entry* curr = head;
    while (curr != NULL)
    {
        printf("%6d |%5d  |%3d\n", curr->file, curr->start, curr->end);
        curr = curr->next;
    }
}







