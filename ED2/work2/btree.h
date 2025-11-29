#ifndef BTREE_H
#define BTREE_H

#include <stdio.h>
#include <stdbool.h>

// --- Constants & Configuration ---

// The "Order" (t) of the B-Tree.
// Minimum keys per node = t - 1
// Maximum keys per node = 2*t - 1
// Maximum children per node = 2*t
// NOTE: For production, calculate this based on 4096 byte pages. 
#define BTREE_ORDER 3 

// Max length of the filename (Key). Reduced from 256 to save space.
#define MAX_KEY_SIZE 64 

// Special value to represent "NULL" in a file offset context
#define INVALID_OFFSET -1L


// --- Data Structures ---

typedef struct {
    char recordName[50]; // Reduced from 256 for better B-Tree performance
    int thresholdValue;
    long dataOffset;     // Where the compressed image starts in the .bin file
    long blockSize;      // Size of the compressed image
    int isDeleted;       // Lazy deletion flag
} IndexRecord;

typedef struct {
    int numKeys;                         // How many keys currently in this node?
    int isLeaf;                          // Is this a leaf (1) or internal node (0)?
    long children[2 * BTREE_ORDER];            // "Pointers" (file offsets) to child nodes
    IndexRecord records[2 * BTREE_ORDER - 1];  // Array of your data records
} BTreeNode;

typedef struct {
    long rootNodeOffset; // Where in the file is the root node located?
    long nextFreeOffset; // Where can we write a new node? (End of file)
} BTreeHeader;


// --- Function Prototypes ---

/**
 * Initialization
 * Opens the B-Tree file. If it doesn't exist, creates it and initializes the header/root.
 */
FILE* btree_open(const char* filename);

/**
 * Insertion
 * Adds a new image record to the index. Handles node splitting and root growth.
 * Returns 1 on success, 0 on failure.
 */
int btree_insert(FILE* treeFile, IndexRecord record);

/**
 * Search
 * Looks for a filename in the B-Tree.
 * Returns 1 if found (filling the *result struct), 0 if not found.
 */
int btree_search(FILE* treeFile, const char* key, int threshold, IndexRecord* foundRecord);

/**
 * Debugging / Visualization
 * Prints the tree structure to console (useful to see hierarchy).
 */
void btree_print_all(FILE* treeFile);

// Cleanup
void btree_close(FILE* treeFile);

int compare_records(const char* keyName, int keyThreshold, IndexRecord existingRecord);

int btree_delete(FILE* treeFile, const char* key, int threshold);

// Add these function declarations to your btree.h file

/**
 * Compacts the database by removing deleted records and rebuilding the B-tree
 * @param binaryDataFile Path to the binary data file
 * @param btreeIndexFile Path to the B-tree index file
 * @return Number of records copied, or -1 on error
 */
int btree_compact_database(const char *binaryDataFile, const char *btreeIndexFile);

/**
 * Analyzes fragmentation in the database
 * @param treeFile Open B-tree file handle
 * @return Number of deleted records found, or -1 on error
 */
int btree_analyze_fragmentation(FILE *treeFile);

/**
 * Collects all valid (non-deleted) records from the B-tree
 * @param treeFile Open B-tree file handle
 * @param totalRecords Output parameter for number of records found
 * @return Array of IndexRecord structs (caller must free), or NULL on error
 */
IndexRecord* btree_collect_all_records(FILE *treeFile, int *totalRecords);

#endif