#include <stdio.h>     // For printf, fprintf, perror, fopen, fclose, fread
#include <stdlib.h>    // For EXIT_SUCCESS, EXIT_FAILURE, malloc, free
#include <ctype.h>     // For isprint
#include <sys/ioctl.h> // For ioctl and TIOCGWINSZ
#include <termios.h>   // For struct winsize (contains terminal dimensions)
#include <unistd.h>    // For STDOUT_FILENO (file descriptor for standard output), isatty

// Default bytes per line if terminal width cannot be determined or is too small
#define DEFAULT_BYTES_PER_LINE 16
// Minimum bytes per line to avoid extremely narrow and unreadable output
#define MIN_BYTES_PER_LINE 4 // Must be at least 1, 4 is a decent minimum
// Maximum bytes per line to prevent excessively wide lines even on huge monitors
#define MAX_BYTES_PER_LINE 64

int main(int argc, char *argv[]) {
    // 1. Handle command-line arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        fprintf(stderr, "Displays the binary content of a file in hexadecimal format.\n");
        return EXIT_FAILURE;
    }

    const char *file_path = argv[1];

    int bytes_per_line = DEFAULT_BYTES_PER_LINE; // Initialize with default

    // 2. Determine terminal width to calculate optimal bytes_per_line
    struct winsize ws;
    // Check if standard output is a terminal (tty) AND if ioctl succeeds
    if (isatty(STDOUT_FILENO) && ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        int terminal_width = ws.ws_col; // Get columns (width)

        // Calculate maximum bytes per line that fits within the terminal width.
        // The output format is: "XXXXXXXX: HH HH HH HH HH HH HH HH  |................|\n"
        // Let N be BYTES_PER_LINE.
        // - Offset part: "XXXXXXXX: " is 10 characters.
        // - Hex part: Each byte takes "XX " (3 chars). Plus one extra space in the middle.
        //   So, N * 3 + 1 (for middle space if N >= 2).
        // - Separator: " |" is 2 characters.
        // - ASCII part: Each byte takes 1 char. Plus the closing "|" (N + 1 chars).
        //
        // Total_width = 10 (offset) + (N * 3 + 1) (hex) + 2 (sep) + (N + 1) (ASCII)
        // Total_width = 10 + 3N + 1 + 2 + N + 1 = 4N + 14
        //
        // We want: 4N + 14 <= terminal_width
        // 4N <= terminal_width - 14
        // N <= (terminal_width - 14) / 4

        int calculated_n = (terminal_width - 14) / 4;

        // Apply limits: ensure it's not too small or too large
        if (calculated_n < MIN_BYTES_PER_LINE) {
            bytes_per_line = MIN_BYTES_PER_LINE;
        } else if (calculated_n > MAX_BYTES_PER_LINE) {
            bytes_per_line = MAX_BYTES_PER_LINE;
        } else {
            bytes_per_line = calculated_n;
        }

        // Optional: Ensure bytes_per_line is an even number for cleaner middle spacing.
        // If it's odd and > 1, make it even.
        if (bytes_per_line % 2 != 0 && bytes_per_line > 1) {
            bytes_per_line--;
        }

    }
    // If not a TTY (e.g., piped to a file) or ioctl fails, bytes_per_line remains DEFAULT_BYTES_PER_LINE

    // 3. Open the specified file in binary read mode
    FILE *fp = fopen(file_path, "rb"); // "rb" means "read binary"
    if (fp == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    // 4. Allocate buffer dynamically based on calculated bytes_per_line
    unsigned char *buffer = (unsigned char *)malloc(bytes_per_line);
    if (buffer == NULL) {
        perror("Error allocating buffer");
        fclose(fp); // Close file before exiting on error
        return EXIT_FAILURE;
    }

    int bytes_read;      // Number of bytes read in current chunk
    long offset = 0;     // Current file offset (address)

    // Loop until no more bytes are read (end of file)
    while ((bytes_read = fread(buffer, 1, bytes_per_line, fp)) > 0) {
        // Print the file offset (address) in hexadecimal
        printf("%08lX: ", offset);

        // Print hexadecimal representation of each byte
        for (int i = 0; i < bytes_per_line; i++) { // Loop up to calculated bytes_per_line
            if (i < bytes_read) {
                printf("%02X ", buffer[i]);
            } else {
                printf("   "); // Pad if the last line is not full
            }
            // Add an extra space in the middle of the hex block for readability
            if (bytes_per_line >= 2 && i == (bytes_per_line / 2) - 1) {
                printf(" ");
            }
        }

        // Print ASCII representation of each byte
        printf(" |"); // Separator between hex and ASCII
        for (int i = 0; i < bytes_read; i++) {
            printf("%c", isprint(buffer[i]) ? buffer[i] : '.'); // Print char or '.'
        }
        printf("|\n"); // End of the ASCII block and newline for the next line

        // Increment the offset by the number of bytes read
        offset += bytes_read;
    }

    // 5. Clean up: free dynamically allocated buffer and close file
    free(buffer);
    fclose(fp);

    return EXIT_SUCCESS; // Indicate successful execution
}
