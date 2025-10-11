#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Global counters for comparisons and swaps
unsigned long long comparisons = 0;
unsigned long long swaps = 0;

// Function to swap two elements
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
    swaps++;
}

// Heapify a subtree rooted with node i which is an index in arr[]
// n is size of heap
void heapify(int arr[], int n, int i) {
    int largest = i;      // Initialize largest as root
    int left = 2 * i + 1; // Left child
    int right = 2 * i + 2; // Right child

    // If left child is larger than root
    if (left < n) {
        comparisons++;
        if (arr[left] > arr[largest])
            largest = left;
    }

    // If right child is larger than largest so far
    if (right < n) {
        comparisons++;
        if (arr[right] > arr[largest])
            largest = right;
    }

    // If largest is not root
    if (largest != i) {
        swap(&arr[i], &arr[largest]);
        // Recursively heapify the affected sub-tree
        heapify(arr, n, largest);
    }
}

// Main function to do heap sort
void heapSort(int arr[], int n) {
    // Build heap (rearrange array)
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(arr, n, i);

    // One by one extract an element from heap
    for (int i = n - 1; i > 0; i--) {
        // Move current root to end
        swap(&arr[0], &arr[i]);

        // Call max heapify on the reduced heap
        heapify(arr, i, 0);
    }
}

// Count the number of integers in the file
int countIntegers(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }

    int count = 0;
    int num;
    while (fscanf(file, "%d", &num) == 1) {
        count++;
    }

    fclose(file);
    return count;
}

// Read integers from file into array
void readIntegers(const char* filename, int arr[], int n) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }

    for (int i = 0; i < n; i++) {
        if (fscanf(file, "%d", &arr[i]) != 1) {
            printf("Error reading integer %d from file\n", i);
            exit(1);
        }
    }

    fclose(file);
}

int main() {
    const char* filename = "input.txt";
    
    // Count integers in file
    int n = countIntegers(filename);
    
    // Allocate memory for array
    int* arr = (int*)malloc(n * sizeof(int));
    if (arr == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    // Read integers from file
    readIntegers(filename, arr, n);
    
    // Reset counters
    comparisons = 0;
    swaps = 0;
    
    // Measure time
    clock_t start = clock();
    
    // Sort the array
    heapSort(arr, n);
    
    clock_t end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    // Print results
    printf("Sorted array: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
    
    printf("Execution time: %f seconds\n", cpu_time_used);
    printf("Number of comparisons: %llu\n", comparisons);
    printf("Number of swaps: %llu\n", swaps);
    
    // Free allocated memory
    free(arr);
    
    return 0;
}