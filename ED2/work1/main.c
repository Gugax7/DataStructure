#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DATA_FILE "images.dat"
#define INDEX_FILE "index.idx"

#define MAX_NAME_LEN 256

typedef struct {
    char name[MAX_NAME_LEN];
    long offset;
    int width;
    int height;
    int max_val;
    char magic[3];
    int removed;
    long compressed_size;
    unsigned char start_pixel;
} IndexRecord;

unsigned char* read_pgm_pixels(const char* pgm_path, int* width, int* height, int* max_val, char magic[3], size_t* data_size) {
    FILE *pgm_fp = fopen(pgm_path, "rb");
    if (!pgm_fp) {
        perror("Error opening PGM file for reading");
        return NULL;
    }

    if (fscanf(pgm_fp, "%2s", magic) != 1) { 
        fprintf(stderr, "Error: Could not read PGM magic number from '%s'.\n", pgm_path);
        fclose(pgm_fp); 
        return NULL; 
    }
    
    int ch;
    while ((ch = fgetc(pgm_fp)) != EOF) {
        if (isspace(ch)) {
            continue;
        }
        if (ch == '#') {
            while ((ch = fgetc(pgm_fp)) != EOF && ch != '\n');
            continue;
        }
        ungetc(ch, pgm_fp);
        break;
    }

    if (fscanf(pgm_fp, "%d %d", width, height) != 2) { 
        fprintf(stderr, "Error: Could not read PGM width and height from '%s'.\n", pgm_path);
        fclose(pgm_fp); 
        return NULL; 
    }
    
    while ((ch = fgetc(pgm_fp)) != EOF) {
        if (isspace(ch)) continue;
        if (ch == '#') {
            while ((ch = fgetc(pgm_fp)) != EOF && ch != '\n');
            continue;
        }
        ungetc(ch, pgm_fp);
        break;
    }

    if (fscanf(pgm_fp, "%d", max_val) != 1) { 
        fprintf(stderr, "Error: Could not read PGM max value from '%s'.\n", pgm_path);
        fclose(pgm_fp); 
        return NULL; 
    }

    fgetc(pgm_fp);

    if (strcmp(magic, "P5") != 0 && strcmp(magic, "P2") != 0) {
        fprintf(stderr, "Error: Unsupported PGM format (only P2 or P5).\n");
        fclose(pgm_fp);
        return NULL;
    }

    *data_size = (*width) * (*height);
    if (*data_size == 0) { // Handle case of 0-dimension image
        fclose(pgm_fp);
        return (unsigned char*)malloc(0); // Return valid, empty buffer
    }
    unsigned char* pixels = (unsigned char*)malloc(*data_size);
    if (!pixels) {
        fprintf(stderr, "Memory allocation error\n");
        fclose(pgm_fp);
        return NULL;
    }
    
    if (strcmp(magic, "P5") == 0) {
        if (fread(pixels, 1, *data_size, pgm_fp) != *data_size) {
            fprintf(stderr, "Error reading binary pixel data.\n");
            free(pixels); fclose(pgm_fp); return NULL;
        }
    } else {
        unsigned int val;
        for (size_t i = 0; i < *data_size; i++) {
            if (fscanf(pgm_fp, "%u", &val) != 1) {
                fprintf(stderr, "Error reading ASCII pixel data at pixel %zu.\n", i);
                free(pixels); fclose(pgm_fp); return NULL;
            }
            pixels[i] = (unsigned char)val;
        }
    }
    fclose(pgm_fp);
    return pixels;
}

void write_pgm_pixels(const char* pgm_path, const unsigned char* pixels, int width, int height, int max_val, const char magic[3]) {
    FILE *output_fp = fopen(pgm_path, "wb");
    if (!output_fp) {
        perror("Error creating output PGM file");
        return;
    }
    
    fprintf(output_fp, "%s\n%d %d\n%d\n", magic, width, height, max_val);
    
    size_t data_size = width * height;
    if (strcmp(magic, "P5") == 0) {
        fwrite(pixels, 1, data_size, output_fp);
    } else {
        for (size_t i = 0; i < data_size; i++) {
            fprintf(output_fp, "%d ", pixels[i]);
            if ((i + 1) % 17 == 0) fprintf(output_fp, "\n");
        }
    }
    fclose(output_fp);
}

void applyThreshold(unsigned char* pixels, size_t data_size, int threshold, int max_val) {
    for (size_t i = 0; i < data_size; i++) {
        if (pixels[i] < threshold) {
            pixels[i] = 0;
        } else {
            pixels[i] = max_val;
        }
    }
}

void importImage(char* pgm_path) {
    int width, height, max_val;
    char magic[3];
    size_t data_size;
    unsigned char* pixels = read_pgm_pixels(pgm_path, &width, &height, &max_val, magic, &data_size);
    if (!pixels) return;

    if (data_size == 0) {
        free(pixels);
        printf("Warning: Image has no pixel data, skipping import.\n");
        return;
    }

    for (size_t i = 0; i < data_size; i++) {
        if (pixels[i] > 0) pixels[i] = 1;
    }

    // Allocate enough space for worst case (alternating pixels might double the data size)
    unsigned char* runs = (unsigned char*)malloc(data_size * 2);
    if (!runs) {
        fprintf(stderr, "Memory allocation error for compression buffer.\n");
        free(pixels);
        return;
    }

    size_t run_count = 0;
    unsigned char start_pixel = pixels[0];
    
    size_t i = 0;
    while (i < data_size) {
        unsigned char current_val = pixels[i];
        size_t run_start_index = i;
        while (i < data_size && pixels[i] == current_val) {
            i++;
        }
        size_t total_run_length = i - run_start_index;

        // Correctly encode the run, splitting if longer than 255
        while (total_run_length > 255) {
            runs[run_count++] = 255; // A full chunk of the current color
            runs[run_count++] = 0;   // A zero-length chunk of the opposite color
            total_run_length -= 255;
        }
        runs[run_count++] = (unsigned char)total_run_length;
    }
    free(pixels);

    long compressed_byte_size = run_count * sizeof(unsigned char);
    FILE* data_fp = fopen(DATA_FILE, "ab");
    if (!data_fp) { perror("Error opening data file for writing"); free(runs); return; }

    long offset = ftell(data_fp);
    if (fwrite(runs, 1, compressed_byte_size, data_fp) != compressed_byte_size) {
        perror("Error writing compressed data");
        free(runs); fclose(data_fp); return;
    }
    fclose(data_fp);
    free(runs);

    IndexRecord record;
    strncpy(record.name, pgm_path, MAX_NAME_LEN - 1);
    record.name[MAX_NAME_LEN - 1] = '\0';
    record.offset = offset;
    record.width = width;
    record.height = height;
    record.max_val = max_val;
    strncpy(record.magic, magic, 3);
    record.magic[2] = '\0';
    record.removed = 0;
    record.compressed_size = compressed_byte_size;
    record.start_pixel = start_pixel;

    FILE *index_fp = fopen(INDEX_FILE, "ab");
    if (!index_fp) { perror("Error opening index file for writing"); return; }
    if (fwrite(&record, sizeof(IndexRecord), 1, index_fp) != 1) {
        perror("Error writing index record");
        fclose(index_fp); return;
    }
    fclose(index_fp);

    printf("Image '%s' compressed and imported. (Original %.2f KB -> Compressed %.2f KB)\n",
           pgm_path, (float)data_size / 1024.0, (float)compressed_byte_size / 1024.0);
}

void exportImage(char* image_name, char* output_path) {
    FILE* index_fp = fopen(INDEX_FILE, "rb");
    if (!index_fp) {
        perror("Could not open index file. Have you imported any images?");
        return;
    }

    IndexRecord record;
    int found = 0;
    while (fread(&record, sizeof(IndexRecord), 1, index_fp) == 1) {
        if (strcmp(record.name, image_name) == 0 && record.removed == 0) {
            found = 1;
            break;
        }
    }
    fclose(index_fp);

    if (!found) {
        printf("Image '%s' not found in the database.\n", image_name);
        return;
    }

    FILE* data_fp = fopen(DATA_FILE, "rb");
    if (!data_fp) {
        perror("Could not open data file");
        return;
    }

    fseek(data_fp, record.offset, SEEK_SET);
    unsigned char* runs = (unsigned char*)malloc(record.compressed_size);
    if (!runs) { fprintf(stderr, "Memory allocation error for compressed data.\n"); fclose(data_fp); return; }
    if (fread(runs, 1, record.compressed_size, data_fp) != record.compressed_size) {
        perror("Error reading compressed data");
        free(runs); fclose(data_fp); return;
    }
    fclose(data_fp);

    size_t original_size = (size_t)record.width * (size_t)record.height;
    unsigned char* pixels = (unsigned char*)malloc(original_size);
    if (!pixels) { fprintf(stderr, "Memory allocation error for decompressed pixels.\n"); free(runs); return; }
    
    size_t pixel_index = 0;
    size_t run_count = record.compressed_size / sizeof(unsigned char);
    unsigned char current_pixel_val = record.start_pixel;

    for (size_t i = 0; i < run_count; i++) {
        unsigned char run_length = runs[i];
        for (int j = 0; j < run_length; j++) {
            if (pixel_index < original_size) {
                pixels[pixel_index++] = current_pixel_val;
            } else {
                // This indicates an error, but we'll check properly at the end
            }
        }
        current_pixel_val = (current_pixel_val == 0) ? 1 : 0;
    }
    
    if (pixel_index != original_size) {
        fprintf(stderr, "Decompression failed for '%s'. The image data is corrupt or mismatched.\n", image_name);
        fprintf(stderr, "Total pixels expected: %zu, but decompressed: %zu.\n", original_size, pixel_index);
        fprintf(stderr, "Please remove this image with the 'remove' command and re-import it.\n");
        free(runs);
        free(pixels);
        return;
    }
    
    free(runs);

    for (size_t i = 0; i < original_size; i++) {
        if (pixels[i] == 1) {
            pixels[i] = record.max_val;
        }
    }

    write_pgm_pixels(output_path, pixels, record.width, record.height, record.max_val, record.magic);
    free(pixels);

    printf("Image '%s' exported successfully to '%s'.\n", image_name, output_path);
}

void listImages() {
    FILE* index_fp = fopen(INDEX_FILE, "rb");
    if (!index_fp) {
        printf("No images in database or index file not found.\n");
        return;
    }

    IndexRecord record;
    int count = 0;
    printf("--- Stored Images ---\n");
    while (fread(&record, sizeof(IndexRecord), 1, index_fp) == 1) {
        if (record.removed == 0) {
            printf("- Name: %s (Format: %s, Size: %dx%d, Compressed Size: %.2f KB)\n", 
                   record.name, record.magic, record.width, record.height, (float)record.compressed_size / 1024.0);
            count++;
        }
    }
    fclose(index_fp);
    if (count == 0) {
        printf("No active images found.\n");
    }
    printf("---------------------\n");
}

void removeImage(char* image_name) {
    FILE* index_fp = fopen(INDEX_FILE, "r+b"); 
    if (!index_fp) {
        perror("Could not open index file");
        return;
    }

    IndexRecord record;
    int found = 0;
    long record_pos = 0;

    while (fread(&record, sizeof(IndexRecord), 1, index_fp) == 1) {
        if (strcmp(record.name, image_name) == 0 && record.removed == 0) {
            found = 1;
            record_pos = ftell(index_fp) - sizeof(IndexRecord);
            break;
        }
    }

    if (found) {
        record.removed = 1;
        fseek(index_fp, record_pos, SEEK_SET);
        fwrite(&record, sizeof(IndexRecord), 1, index_fp);
        printf("Image '%s' removed successfully.\n", image_name);
    } else {
        printf("Image '%s' not found in the database or already removed.\n", image_name);
    }

    fclose(index_fp);
}

void thresholdImage(const char* input_path, const char* output_path, int threshold) {
    int width, height, max_val;
    char magic[3];
    size_t data_size;

    printf("Applying threshold %d to '%s'...\n", threshold, input_path);

    unsigned char* pixels = read_pgm_pixels(input_path, &width, &height, &max_val, magic, &data_size);
    if (!pixels) {
        fprintf(stderr, "Failed to read input image '%s'.\n", input_path);
        return;
    }
    
    applyThreshold(pixels, data_size, threshold, max_val);

    write_pgm_pixels(output_path, pixels, width, height, max_val, magic);
    free(pixels);

    printf("Thresholded image saved to '%s'.\n", output_path);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  To import:   %s import <image_path.pgm>\n", argv[0]);
        fprintf(stderr, "  To export:   %s export <image_name_in_db> <output_path.pgm>\n", argv[0]);
        fprintf(stderr, "  To list:     %s list\n", argv[0]);
        fprintf(stderr, "  To remove:   %s remove <image_name_in_db>\n", argv[0]);
        fprintf(stderr, "  To threshold: %s threshold <input_path.pgm> <output_path.pgm> <threshold_value>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "import") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Incorrect usage. Try: %s import <image_path.pgm>\n", argv[0]);
            return 1;
        }
        importImage(argv[2]);
    } else if (strcmp(argv[1], "export") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Incorrect usage. Try: %s export <image_name_in_db> <output_path.pgm>\n", argv[0]);
            return 1;
        }
        exportImage(argv[2], argv[3]);
    } else if (strcmp(argv[1], "list") == 0) {
        if (argc != 2) {
             fprintf(stderr, "Incorrect usage. Try: %s list\n", argv[0]);
             return 1;
        }
        listImages();
    } else if (strcmp(argv[1], "remove") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Incorrect usage. Try: %s remove <image_name_in_db>\n", argv[0]);
            return 1;
        }
        removeImage(argv[2]);
    } else if (strcmp(argv[1], "threshold") == 0) {
        if (argc != 5) {
            fprintf(stderr, "Incorrect usage. Try: %s threshold <input_path.pgm> <output_path.pgm> <threshold_value>\n", argv[0]);
            return 1;
        }
        int threshold_val = atoi(argv[4]);
        if (threshold_val < 0 || threshold_val > 255) {
            fprintf(stderr, "Threshold value must be between 0 and 255.\n");
            return 1;
        }
        thresholdImage(argv[2], argv[3], threshold_val);
    }
    else {
        fprintf(stderr, "Unknown command: '%s'.\n", argv[1]);
        return 1;
    }

    return 0;
}

