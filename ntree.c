#include <stdio.h>      // For printf, fprintf, perror
#include <stdlib.h>     // For malloc, realloc, free, qsort
#include <string.h>     // For strcmp, strcpy, strcat, strdup
#include <dirent.h>     // For opendir, readdir, closedir, struct dirent
#include <sys/stat.h>   // For stat, S_ISDIR
#include <limits.h>     // For PATH_MAX (if available, otherwise fallback)

// Define PATH_MAX if it's not available on the system
#ifndef PATH_MAX
#define PATH_MAX 4096 // A common maximum path length on Linux
#endif

// Structure to hold directory entry information for sorting
typedef struct {
    char *name;   // Name of the file or directory
    int is_dir;   // 1 if it's a directory, 0 if it's a file
} DirEntry;

// Comparison function for qsort.
// It sorts entries alphabetically, with directories coming before files.
int compareDirEntries(const void *a, const void *b) {
    const DirEntry *entryA = (const DirEntry *)a;
    const DirEntry *entryB = (const DirEntry *)b;

    // Prioritize directories:
    // If A is a directory and B is not, A comes first (-1)
    if (entryA->is_dir && !entryB->is_dir) {
        return -1;
    }
    // If B is a directory and A is not, B comes first (1)
    if (!entryA->is_dir && entryB->is_dir) {
        return 1;
    }

    // If both are directories or both are files, sort alphabetically by name
    return strcmp(entryA->name, entryB->name);
}

/**
 * @brief Recursively lists the contents of a directory in a tree-like structure.
 *
 * @param path The current directory path to list.
 * @param indent_level The current indentation level (for internal use, not directly used for printing here).
 * @param prefix The string prefix to use for indentation (e.g., "│   ", "    ").
 */
void list_directory_recursive(const char *path, int indent_level, const char *prefix) {
    DIR *dir;               // Directory stream pointer
    struct dirent *entry;   // Pointer to directory entry
    struct stat statbuf;    // Structure for file status information
    char full_path[PATH_MAX]; // Buffer to store the full path of each entry

    // Try to open the directory
    if (!(dir = opendir(path))) {
        fprintf(stderr, "Error: Cannot open directory '%s'\n", path);
        return;
    }

    // --- Phase 1: Read all entries into a dynamic array ---
    DirEntry *entries = NULL; // Pointer to the dynamic array of DirEntry structs
    int num_entries = 0;      // Current number of entries
    int capacity = 10;        // Initial capacity for the dynamic array

    // Allocate initial memory for entries
    entries = (DirEntry *)malloc(capacity * sizeof(DirEntry));
    if (!entries) {
        perror("Error: Memory allocation failed for entries array");
        closedir(dir);
        return;
    }

    // Read directory entries one by one
    while ((entry = readdir(dir)) != NULL) {
        // Skip current directory "." and parent directory ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construct the full path of the current entry
        // snprintf is safer than sprintf as it prevents buffer overflows
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        // Get file status to determine if it's a directory or a regular file.
        // stat() is used to get information about the file/directory.
        if (stat(full_path, &statbuf) == -1) {
            perror("Error getting file status");
            continue; // Skip this entry if stat fails
        }

        // Check if the dynamic array needs to be resized
        if (num_entries >= capacity) {
            capacity *= 2; // Double the capacity
            DirEntry *new_entries = (DirEntry *)realloc(entries, capacity * sizeof(DirEntry));
            if (!new_entries) {
                perror("Error: Memory reallocation failed for entries array");
                // Clean up already allocated names before exiting
                for (int i = 0; i < num_entries; i++) {
                    free(entries[i].name);
                }
                free(entries);
                closedir(dir);
                return;
            }
            entries = new_entries; // Update pointer to the new, larger array
        }

        // Store the entry's name and type
        entries[num_entries].name = strdup(entry->d_name); // Duplicate string to own memory
        if (!entries[num_entries].name) {
            perror("Error: Memory allocation failed for entry name");
            // Clean up already allocated names and the array itself
            for (int i = 0; i < num_entries; i++) {
                free(entries[i].name);
            }
            free(entries);
            closedir(dir);
            return;
        }
        entries[num_entries].is_dir = S_ISDIR(statbuf.st_mode); // Check if it's a directory
        num_entries++;
    }
    closedir(dir); // Close the directory stream after reading all entries

    // --- Phase 2: Sort the collected entries ---
    qsort(entries, num_entries, sizeof(DirEntry), compareDirEntries);

    // --- Phase 3: Print entries and recurse for subdirectories ---
    for (int i = 0; i < num_entries; i++) {
        DirEntry current_entry = entries[i];
        int is_last_entry = (i == num_entries - 1); // Check if this is the last entry in the current directory

        // Print the current indentation prefix
        printf("%s", prefix);

        // Print the appropriate branch connector
        if (is_last_entry) {
            printf("└── "); // Last entry in the list
        } else {
            printf("├── "); // Not the last entry
        }

        // Print the name of the file or directory
        printf("%s\n", current_entry.name);

        // If the current entry is a directory, recurse into it
        if (current_entry.is_dir) {
            char new_prefix[PATH_MAX]; // Buffer for the prefix for the next level of recursion

            // Construct the new prefix:
            // If it's the last entry, add 4 spaces ("    ") to the prefix.
            // If it's not the last, add a vertical line and 3 spaces ("│   ").
            snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_last_entry ? "    " : "│   ");

            // Construct the full path for the recursive call
            snprintf(full_path, sizeof(full_path), "%s/%s", path, current_entry.name);

            // Make the recursive call
            list_directory_recursive(full_path, indent_level + 1, new_prefix);
        }
        free(current_entry.name); // Free the memory allocated by strdup for this entry's name
    }

    free(entries); // Free the dynamic array itself
}

int main(int argc, char *argv[]) {
    const char *start_path = "."; // Default starting path is the current directory

    // Check for command-line arguments
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [directory_path]\n", argv[0]);
        return 1; // Indicate error
    } else if (argc == 2) {
        start_path = argv[1]; // Use the provided directory path
    }

    // Print the starting directory itself
    printf("%s\n", start_path);

    // Start the recursive listing process
    list_directory_recursive(start_path, 0, ""); // Initial call with no indentation prefix

    return 0; // Indicate success
}
