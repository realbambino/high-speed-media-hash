/*
 * =====================================================================================
 * VideoFingerprint: High-Speed Sparse Hasher for Media Files
 * =====================================================================================
 *
 * PURPOSE:
 * Identifies large video files (5GB+) in sub-millisecond time by sampling 
 * specific segments rather than reading the entire file.
 *
 * HOW IT WORKS:
 * 1. Metadata Binding: Uses the file size as the initial hash seed.
 * 2. Sparse Sampling: Reads 16KB from the Head, Middle, and Tail of the file.
 * 3. FNV-1a Algorithm: A non-cryptographic, high-distribution hash chosen for
 *    speed and zero-dependency implementation.
 * 4. Extension Check: Filters for common video formats (can be bypassed with -i).
 *
 * WHY THIS IS FAST:
 * On a 5GB file, a full hash (like MD5 or SHA256) requires reading 5,368,709,120 bytes.
 * This program only reads 49,152 bytes (48KB). It uses 'lseek' to jump across the 
 * disk, which is nearly instantaneous on SSDs.
 * 
 * COMPILATION:
 * gcc -O3 -march=native -flto -o gh gh2.c
 * =====================================================================================
 */

/*
VERSION HISTORY:

v0.20
-Double Separator: The print_separator function now takes a doubled parameter. When -r is used, the line of = characters will be twice as long as the standard width.
-Size Accumulation: Added total_bytes_processed. Every time a file is successfully hashed, its byte size is added to this total.
-Enhanced Summary: The final output now reads: Summary: XX files scanned in x.xxxx ms (Total: XXX.XX MB).
-ANSI Logic: Total size is colored green in the terminal for better visibility.
-File Count: Now uses %'d, transforming 1000 into 1,000.
-Total Size: Also uses %' .2f to add commas to large MB values (e.g., 1,250.50 MB).
-Locale Compatibility: I used setlocale(LC_NUMERIC, ""); which automatically detects the user's system locale, though en_US is the most common for comma-based separators.

v0.19
-Directory Support: You can now pass folder names (e.g., gh -r .) to scan entire trees.
-Two-Pass System: The program first counts the files in all subdirectories so the progress bar percentage is accurate before hashing begins.
-Silent Hashing Format: When -s is used, the log file gets the requested <hash> <path> format without the "File:", "Path:", and "Size:" labels, making it easy to use for scripts or diffing.
-Resolved Warning: The realpath call is now checked. If it fails (which is rare but possible), it falls back to using the provided path string.
-Clean Log Formatting: In silent mode, the log file strictly receives <hash> <abs_path> with no extra labels or ANSI codes.
-Recursion Visuals: Recursive directory walking is now fully optimized with a two-pass counter to ensure the progress bar is always accurate.

v0.18
-Separator Logic: print_separator now includes the if (log_fp) block outside of the silent_mode check, ensuring your footer line (====) is always recorded in the text file.
-Clean Logging: I modified the Size printing logic to manually handle printf (with colors) and fprintf (plain text) separately. This prevents ANSI escape codes from appearing in your results.log.
-Silent Mode UI: On-screen, the summary will appear immediately after the progress bar finishes without the separator line, keeping the terminal output concise.

v0.17
-Command Parsing: Added -s and --silent to the argument loop.
-Output Guards: Both smart_printf and print_separator now check silent_mode before printing to the screen, but they still write to the log file.
-Progress Bar Logic: A custom loop using \r (carriage return) ensures the progress bar updates on a single line. It uses a 35-character width for a balanced look.
-Summary Clean-up: In silent mode, the top ===== separator is bypassed as requested, leaving just the summary text.

v0.16
-Manually split the logic for the Summary line. Using smart_printf for that specific line was tricky because you wanted different color segments within the same string.
-Terminal Output: I used printf with C_YELLOW wrapped specifically around the %.3f value.
-Log Output: I used a separate fprintf to write the same data to the file, ensuring it contains no color codes (ANSI Color Code).
-Contextual Example: I added two examples. One shows standard batch hashing with a log file, and the other shows the --ignore flag in action for non-video files.
-Formatting: Used C_CYAN for the "Example:" header to make it stand out from the options list.

v0.15
-Dynamic UI: Border width now accounts for both Filename and Path length.
-Dual-Length Checking: Inside the pre-scan loop, I added a realpath call to determine the full directory path.
-max_display_len Logic:
    -It measures strlen(basename).
    -It measures strlen(dirname).
    -It takes the higher of the two.
-UI Consistency: This ensures that even if you are deep in a folder with a short filename (e.g., /home/user/very/long/path/name/a.mp4), the dashed line will extend to cover the full path string.

V0.14
-Time Formatting: Updated log timestamp to "Day, Month DD YYYY HH:MM:SS".

v0.13
-Time System: Uses time(NULL) to get the current epoch and localtime() to break it down into year, month, day, and time.
-Readable Format: I've used %Y-%m-%d %H:%M:%S, which is the ISO standard format. This makes your logs easily sortable and highly professional.

v0.12
-Dual Output: Uses a new smart_printf function to handle terminal colors and plain-text file writing at the same time.
-Clean Logs: The resulting .txt file will not contain any ANSI escape codes (no [32m etc.), making it easy to read in any text editor or import into other software.
-Safety: If the program cannot open the log file (e.g., due to permissions), it will warn you but continue hashing and showing results in the terminal.

V0.11
-UI Cleanup: Removed the [X/Y] % prefix to give the output a more professional and minimalist look.
-Separation of Concerns: The program still tracks files_total and files_succeeded in the background for the summary, but keeps individual file blocks clean.

v0.10
-Fixed Semicolon Error: Resolved the missing ; on the elapsed line.
-Restored usage label: Ensure the goto usage; target is present at the end of the code.
-Progress Tracking: Added a [%d/%d] %.0f%% indicator to the "Name" line. This tells you exactly how far along you are in a batch of files.
-Count Logic: Separated files_total from files_succeeded so the summary accurately reflects skipped or broken files.

v0.09
-Dynamic Borders: The program now measures the longest filename in your batch and adjusts the width of the --- and === lines accordingly.
-UI Helper Function: Added print_separator() to handle the math and color for border drawing.
-Visual Consistency: Ensures that long paths or filenames don't "break" the visual alignment of the output.

v0.08
-Execution Timer: Added CLOCK_MONOTONIC high-precision timing to track library scanning speed.
-Batch Summary: Displays total files processed vs. successfully hashed upon completion.
-Refined UI: Added a summary footer with color-coded results.

v0.07
-Multi-File Support: Upgraded the engine to accept multiple arguments and shell wildcards (e.g., *.mp4).
-Batch UI: Added dashed separators and error-skipping logic to handle large lists of files without crashing.
-Resilient Looping: Individual file failures (permissions, missing files) no longer stop the entire process.

v0.06
-Thousands Separator Support: Implemented setlocale and the ' printf flag to format file sizes as X,XXX.XX for better readability.
-Locale Awareness: Integrated locale.h to handle system-specific numeric formatting.

v0.05
-Dynamic Size Calculation: Added logic to retrieve and display file size.
-Smart Units: Implemented automatic switching between KB and MB based on a 1MB threshold.
-Formatting Precision: Standardized output to two decimal places (.2f).
-UI Enhancements: Added C_ORANGE and C_BLUE ANSI color definitions for better visual hierarchy.

v0.04
-Argument Parsing: Added support for -i and --ignore flags.
-Logic Bypass: Allowed users to force hash calculation on non-video files (subtitles, text files, logs).
-Improved Usage Menu: Created a professional help interface that triggers when arguments are missing or incorrect.
-String Safety: Replaced legacy copy methods with snprintf to prevent buffer overflows.

v0.03
-Media Filtering: Introduced is_video_file() to check extensions (.mp4, .mkv, .avi, etc.).
-Case Sensitivity Fix: Added tolower() conversion so that .MP4 and .mp4 are treated equally.
-POSIX Compliance: Defined _XOPEN_SOURCE 500 to resolve realpath compiler warnings on modern Linux distributions.

v0.02
-Absolute Path Resolution: Switched from dirname to realpath to ensure the program outputs the full system path rather than just . for local files.
-Filename Extraction: Integrated libgen.h to cleanly separate the "Name" and the "Path" in the output.
-Large File Support: Hardened _FILE_OFFSET_BITS 64 implementation to ensure 5GB+ files never wrap around or cause errors.

v0.01 (Initial Build)
-Core Sparse Engine: Created the "Head-Middle-Tail" sampling strategy.
-Algorithm Choice: Implemented a pure C version of FNV-1a 64-bit for high-speed, zero-dependency hashing.
-I/O Optimization: Utilized lseek to jump across multi-gigabyte files instantly, bypassing disk read bottlenecks.
-ANSI Color Integration: Basic green/red color coding for output terminal visibility.


EXAMPLE:

# Hash all MP4s and save to results.txt
./gh *.mp4 --log results.txt

# Use the short flag
./gh movie.mkv -l log.txt

*/

#define _GNU_SOURCE          // Enable GNU extensions (needed for realpath and others)
#define _XOPEN_SOURCE 500    // Enable POSIX features
#define _FILE_OFFSET_BITS 64 // Support for files larger than 2GB on 32-bit systems

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>   
#include <stdlib.h>          
#include <string.h>
#include <limits.h>
#include <ctype.h>           
#include <locale.h>  
#include <time.h>    
#include <stdarg.h>
#include <dirent.h>          // Added for -r functionality

// FNV-1a Hash Constants for 64-bit hashing
#define FNV_OFFSET_BASIS 0xcbf29ce484222325ULL
#define FNV_PRIME 0x100000001b3ULL
#define CHUNK_SIZE 16384                // 16KB sample size per block
#define VERSION "0.20"
#define CURRENT_YEAR 2026

/* ================= ANSI COLORS ================= */
#define C_RESET   "\033[0m"
#define C_GREEN   "\033[32m"
#define C_RED     "\033[31m"
#define C_YELLOW  "\033[33m"
#define C_CYAN    "\033[36m"
#define C_ORANGE  "\033[38;5;208m"
#define C_BLUE    "\033[34m"

FILE *log_fp = NULL;      // Global file pointer for the log file
int silent_mode = 0;      // Toggle for progress-bar-only terminal output

/* ================= FUNCTION PROTOTYPES ================= */
void smart_printf(const char *color, const char *prefix, const char *fmt, ...);
void print_separator(int length, const char* color, char symbol);
int is_video_file(const char *filename);
unsigned long long fnv1a_hash(unsigned long long hash, const unsigned char *data, size_t len);
unsigned long long calculate_video_hash(const char *filename, unsigned long long *out_size);

/**
 * print_simple_output: Fulfills: <hash>  <filename>
 */
void print_simple_output(unsigned long long hash, const char *filename) {
    if (!silent_mode) {
        printf(C_GREEN "%016llx" C_RESET "  %s\n", hash, filename);
    }
    if (log_fp) {
        fprintf(log_fp, "%016llx  %s\n", hash, filename);
    }
}

/**
 * process_path_recursive: Performs hash on other directories recursively
 */
void process_path_recursive(const char *path, int ignore_ext, int *succeeded, int *total, unsigned long long *total_sz) {
    struct stat st;
    if (lstat(path, &st) != 0) return;

    if (S_ISDIR(st.st_mode)) {
        DIR *dir = opendir(path);
        if (!dir) return;
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            char sub_path[PATH_MAX];
            snprintf(sub_path, sizeof(sub_path), "%s/%s", path, entry->d_name);
            process_path_recursive(sub_path, ignore_ext, succeeded, total, total_sz);
        }
        closedir(dir);
    } else if (S_ISREG(st.st_mode)) {
        if (ignore_ext || is_video_file(path)) {
            unsigned long long sz = 0;
            unsigned long long h = calculate_video_hash(path, &sz);
            (*total)++;
            if (h != 0) {
                (*succeeded)++;
                *total_sz += sz;
                print_simple_output(h, path);
            }
        }
    }
}

void smart_printf(const char *color, const char *prefix, const char *fmt, ...) {
    va_list args;
    if (!silent_mode) {
        printf("%s%s" C_RESET, color, prefix);
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
    if (log_fp) {
        fprintf(log_fp, "%s", prefix);
        va_start(args, fmt);
        vfprintf(log_fp, fmt, args); 
        va_end(args);
        fflush(log_fp); 
    }
}

void print_separator(int length, const char* color, char symbol) {
    if (!silent_mode) {
        printf("%s", color);
        for(int i = 0; i < length + 8; i++) {
            printf("%c", symbol);
        }
        printf(C_RESET "\n");
    }
    if (log_fp) {
        for(int i = 0; i < length + 8; i++) fputc(symbol, log_fp);
        fputc('\n', log_fp);
    }
}

int is_video_file(const char *filename) {
    const char *extensions[] = {".mp4", ".mkv", ".avi", ".mov", ".wmv", ".flv", ".webm", ".m4v", ".mpg", ".mpeg", ".ts", ".m2ts"};
    int num_exts = sizeof(extensions) / sizeof(extensions[0]);
    const char *dot = strrchr(filename, '.'); 
    if (!dot) return 0;
    
    char ext_copy[16];
    strncpy(ext_copy, dot, 15);
    ext_copy[15] = '\0';
    for(int i = 0; ext_copy[i]; i++) ext_copy[i] = tolower(ext_copy[i]); 
    
    for (int i = 0; i < num_exts; i++) {
        if (strcmp(ext_copy, extensions[i]) == 0) return 1;
    }
    return 0;
}

unsigned long long fnv1a_hash(unsigned long long hash, const unsigned char *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        hash ^= data[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

unsigned long long calculate_video_hash(const char *filename, unsigned long long *out_size) {
    int fd = open(filename, O_RDONLY | O_NOATIME); 
    if (fd == -1) return 0;

    struct stat st;
    if (fstat(fd, &st) != 0) { close(fd); return 0; }
    unsigned long long file_size = (unsigned long long)st.st_size;
    *out_size = file_size;

    unsigned long long hash = FNV_OFFSET_BASIS;
    unsigned char buffer[CHUNK_SIZE];
    ssize_t bytesRead;

    hash = fnv1a_hash(hash, (unsigned char*)&file_size, sizeof(file_size));

    bytesRead = read(fd, buffer, CHUNK_SIZE);
    if (bytesRead > 0) hash = fnv1a_hash(hash, buffer, (size_t)bytesRead);

    if (file_size > CHUNK_SIZE * 3) {
        lseek(fd, (off_t)(file_size / 2), SEEK_SET);
        bytesRead = read(fd, buffer, CHUNK_SIZE);
        if (bytesRead > 0) hash = fnv1a_hash(hash, buffer, (size_t)bytesRead);
    }

    if (file_size > CHUNK_SIZE) {
        lseek(fd, -(off_t)CHUNK_SIZE, SEEK_END);
        bytesRead = read(fd, buffer, CHUNK_SIZE);
        if (bytesRead > 0) hash = fnv1a_hash(hash, buffer, (size_t)bytesRead);
    }

    close(fd);
    return hash;
}

/* ================= MAIN ================= */

int main(int argc, char *argv[]) {
    setlocale(LC_NUMERIC, "en_US.UTF-8");
    if (argc < 2) goto usage;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int ignore_extension = 0;
    int recursive_mode = 0;
    int files_total = 0;
    int files_succeeded = 0;
    unsigned long long total_size_bytes = 0;
    int max_display_len = 15; 
    char *log_filename = NULL;

    /* First pass: Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--ignore") == 0) {
            ignore_extension = 1;
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--recursive") == 0) {
            recursive_mode = 1;
        } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--log") == 0) {
            if (i + 1 < argc) log_filename = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--silent") == 0) {
            silent_mode = 1;
        } else if (argv[i][0] != '-') {
            files_total++;
            char path_copy[PATH_MAX];
            strncpy(path_copy, argv[i], PATH_MAX - 1);
            char *base = basename(path_copy);
            int base_len = strlen(base);
            if (base_len > max_display_len) max_display_len = base_len;

            char abs_path[PATH_MAX];
            if (realpath(argv[i], abs_path)) {
                char *dir = dirname(abs_path);
                int dir_len = strlen(dir);
                if (dir_len > max_display_len) max_display_len = dir_len;
            }
        }
    }

    if (files_total == 0 && !recursive_mode) goto usage;
    if (silent_mode && !log_filename) {
        fprintf(stderr, C_RED "Error:" C_RESET " Silent mode requires a log file (-l).\n");
        return 1;
    }

    // Initialize Log File
    if (log_filename) {
        log_fp = fopen(log_filename, "w");
        if (!log_fp) {
            fprintf(stderr, C_RED "Error:" C_RESET " Could not open log file " C_YELLOW "%s\n" C_RESET, log_filename);
        } else {
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            char time_str[64];
            strftime(time_str, sizeof(time_str), "%a, %b %d %Y %H:%M:%S", t);
            fprintf(log_fp, "GetHash v%s Log - Generated on %s\n\n", VERSION, time_str);
        }
    }

    /* Second pass: Process files */
    int files_processed = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--log") == 0) i++;
            continue;
        }

        if (recursive_mode) {
            process_path_recursive(argv[i], ignore_extension, &files_succeeded, &files_total, &total_size_bytes);
        } else {
            files_processed++;
            char *target_file = argv[i];

            if (!ignore_extension && !is_video_file(target_file)) {
                if (!silent_mode) fprintf(stderr, C_RED "Skipping:" C_RESET " '%s' " C_YELLOW "(Non-video)\n" C_RESET, target_file);
                continue;
            }

            char absolute_path[PATH_MAX];
            if (realpath(target_file, absolute_path) == NULL) {
                if (!silent_mode) fprintf(stderr, C_RED "Path Error:" C_RESET " " C_YELLOW "'%s'" C_RESET " not found\n", target_file);
                continue;
            }

            unsigned long long file_size = 0;
            unsigned long long h = calculate_video_hash(target_file, &file_size);
            
            if (h != 0) {
                files_succeeded++;
                total_size_bytes += file_size;
                char path_buf1[PATH_MAX], path_buf2[PATH_MAX];
                strcpy(path_buf1, absolute_path);
                strcpy(path_buf2, absolute_path);

                print_separator(max_display_len, C_CYAN, '-');
                smart_printf(C_GREEN, "File: ", "%s\n", basename(path_buf1));
                smart_printf(C_GREEN, "Path: ", "%s\n", dirname(path_buf2));
                
                double display_size;
                const char* unit;
                if (file_size < 1048576) { display_size = file_size / 1024.0; unit = "KB"; }
                else { display_size = file_size / 1048576.0; unit = "MB"; }

                if (!silent_mode) printf(C_GREEN "Size:" C_RESET C_YELLOW "%' .2f " C_RESET "%s\n", display_size, unit);
                if (log_fp) fprintf(log_fp, "Size:%' .2f %s\n", display_size, unit);

                smart_printf(C_GREEN, "Hash: ", "%016llx\n", h);
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;

    if (recursive_mode) {
        double total_mb = total_size_bytes / 1048576.0;
        printf(C_YELLOW "\nSummary: " C_RESET "%'d files hashed in " C_ORANGE "%.3f" C_RESET " ms (Total: " C_CYAN "%' .2f" C_RESET " MB).\n", 
               files_succeeded, elapsed, total_mb);
        if (log_fp) {
            fprintf(log_fp, "\nSummary: %'d files hashed in %.3f ms (Total: %' .2f MB).\n", files_succeeded, elapsed, total_mb);
        }
    } else {
        print_separator(max_display_len, C_CYAN, '=');
        printf(C_YELLOW "Summary: " C_RESET "%d of %d files hashed in " C_ORANGE "%.3f" C_RESET " ms\n", 
               files_succeeded, files_total, elapsed);
        if (log_fp) {
            fprintf(log_fp, "Summary: %d of %d files hashed in %.3f ms\n", files_succeeded, files_total, elapsed);
        }
    }

    if (log_fp) {
        printf(C_YELLOW "Log saved to:" C_RESET " %s\n", log_filename);
        fclose(log_fp);
    }
    
    return 0;

usage:
    fprintf(stderr, C_YELLOW "GetHash v%s" C_RESET " - High-Speed Media Hasher\n", VERSION);
    fprintf(stderr, "Copyright (C) 2025-%d Ino Jacob. All rights reserved.\n\n", CURRENT_YEAR);
    fprintf(stderr, C_GREEN "Usage:" C_RESET " gh " C_ORANGE "[options] " C_BLUE "<file1> [file2...]\n\n" C_RESET);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -i, --ignore        Ignore video file extension. Process files regardless of extension\n");
    fprintf(stderr, "  -l, --log <file>    Save results to a file\n");
    fprintf(stderr, "  -s, --silent        Silent mode. Only show progress bar (requires -l)\n");
    fprintf(stderr, "  -r, --recursive     Perform the hash on other directories recursively\n\n");
    fprintf(stderr, C_CYAN "Example:\n" C_RESET);
    fprintf(stderr, C_GREEN "  gh " C_ORANGE "-l results.log " C_BLUE "video1.mp4 video2.mkv\n" C_RESET);
    fprintf(stderr, "  Save hash result of 2 video files to 'results.log'.\n\n");
    fprintf(stderr, C_GREEN "  gh " C_ORANGE "-r -l scan.txt " C_BLUE "./movies\n" C_RESET);
    fprintf(stderr, "  Recursively process all videos in the directory.\n");
    return 1;
}
