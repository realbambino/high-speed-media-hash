# GetHash (gh) 🚀
**High-Speed Sparse Media Hasher for Linux**

`GetHash` is a lightweight, high-performance C utility designed to generate unique fingerprints for video files in milliseconds. Instead of reading a multi-gigabyte file in its entirety, it uses **Sparse Hashing** to sample key points (Head, Mid, Tail), making it ideal for large media libraries, deduplication scripts, and media players.



## ✨ Features
* **Blazing Fast**: Hashes files of any size (even 100GB+) in under 5ms.
* **Sparse Sampling**: Samples the file structure (Head/Mid/Tail) + File Size to ensure uniqueness.
* **Silent Mode**: Optimized for background tasks with a clean terminal progress bar.
* **Smart Logging**: Generates clean, ANSI-free text logs for audit trails.
* **Auto-Filtering**: Automatically recognizes common video extensions (MP4, MKV, AVI, etc.).

## 🛠 Installation

### Prerequisites
* A Linux-based OS
* `gcc` (GNU Compiler Collection)

### Compilation
Clone the repository and compile using `gcc`:

```bash
gcc -O3 -march=native -flto -o gh gh3.c
```
`-O3` enables high-level optimizations, and `-flto` enables Link Time Optimization for maximum performance.

## 📖 Usage
```
gh [options] <file1> [file2...]
```
### Options
| Flag | Long Flag | Description |
| :--- | :--- | :--- |
| `-i` | `--ignore` | Process files regardless of their extension. |
| `-l` | `--log <file>` | Save results to a specified text file. |
| `-s` | `--silent` | Hide detailed output; show only a progress bar (requires `-l`). |
| `-r` | `--resursive` | Perform the hash on other directories recursively. |

### Examples
**Standard Batch Processing:**
```
gh -l results.log movie1.mp4 movie2.mkv
```
**Background/Silent Processing:**
```
gh -s -l scan_results.txt /path/to/media/*.mp4
```
**Recursively process all videos in the directory:**
```
gh -r -l scan.txt ./movies
```
### Output Examples
**Example 1:**
Create hashes for all files in the current directory & any other directories.
```
gh -r -i *
```
The resulting output will be:
```
fe07d8e5b6c11795  [redacted].7z
a8c7e5a3c37eefca  [redacted].7z
86087c71bccffd07  [redacted].7z
18d138b49b753263  [redacted].7z
c63511e25cefe3ab  covers/[redacted].jpg
.
.
[TRUNCATED]
.
.
eb623860eaa30cb2  pcsx2/BIOS/[redacted].MEC
56a5b272f7a35229  pcsx2/BIOS/[redacted].NVM
2b3a8027e2154901  pcsx2/memcards/[redacted].ps2
e8494a17efd25f8d  pcsx2/memcards/[redacted].ps2
0e70a41d233bf919  scan.txt

Summary: 52 files hashed in 11.118 ms (Total:  22,448.93 MB).
```
**Example 2:**
Create hashes for all 7z-files in the current directory, ignoring the default media file extension.
```
gh -i *.7z
```
The resulting output will be:
```
------------------------------------------------------------------------------
File: Ace Combat 04 - Shattered Skies (US).7z
Path: /media/[redacted]/Backup/Games/PS2
Size: 2,333.90 MB
Hash: fe07d8e5b6c11795
------------------------------------------------------------------------------
File: Final Fantasy XII International - Zodiac Job System (English v0.23).7z
Path: /media/[redacted]/Backup/Games/PS2
Size: 2,934.28 MB
Hash: c8006bbbdd87fe55
------------------------------------------------------------------------------
.
.
[TRUNCATED]
.
.
------------------------------------------------------------------------------
File: Gran Turismo 4 (US).7z
Path: /media/[redacted]/Backup/Games/PS2
Size: 4,107.26 MB
Hash: 75ac1d4ceceea530
------------------------------------------------------------------------------
File: Metal Slug Anthology (EU).7z
Path: /media/[redacted]/Backup/Games/PS2
Size: 626.52 MB
Hash: 9b392ea29a8bd0b2
==============================================================================
Summary: 8 of 8 files hashed in 1.755 ms
```
**Example 3:**
Create hashes for all files in the `/dev/shm/` directory and sub-directories, ignoring the default media file extension.
```
gh -i -r /dev/shm/*
```
The resulting output will be:
```
.
.
[TRUNCATED
.
.
af338dc10fa7fc46  /dev/shm/vivaldi-profile/[redacted]/AdBlockRules/AdBlocking/Index
6c9fa99f225d1f5e  /dev/shm/vivaldi-profile/[redacted]/AdBlockRules/AdBlocking/4283905709
aca5856b73c93df3  /dev/shm/vivaldi-profile/[redacted]/AdBlockRules/AdBlocking/249440746
af5b09a29a7206d9  /dev/shm/vivaldi-profile/[redacted]/AdBlockRules/AdBlocking/1616781723
b3bb66eefa42a4e5  /dev/shm/vivaldi-profile/[redacted]/AdBlockRules/AdBlocking/1303319603

Summary: 14,213 files hashed in 425.042 ms (Total:  3,245.28 MB).
```


## 🧠 How it Works

GetHash uses the **FNV-1a (Fowler–Noll–Vo)** 64-bit hashing algorithm. Unlike standard hashing tools that read every byte of a file, `gh` employs a **Sparse Hashing** strategy. This allows it to generate a unique fingerprint for multi-gigabyte files in constant time ($O(1)$ complexity relative to file size).



To maintain speed without sacrificing accuracy, it samples four specific data points:

1. **The File Size**: Recorded in bytes. This acts as the hash basis to differentiate between files that might have identical headers but different lengths.
2. **The Head**: The first **16KB** of the file. This typically contains the container metadata (e.g., MP4 atoms, MKV headers, or AVI indexes).
3. **The Midpoint**: A **16KB** chunk taken from the exact center of the file. This captures unique bitstream data from the middle of the media.
4. **The Tail**: The last **16KB** of the file. This often contains crucial footer metadata, index chunks, or end-of-file markers.

By combining these four samples, `gh` creates a 64-bit fingerprint that is highly resistant to collisions while requiring only **48KB** of disk I/O per file.

## 📄 License
Copyright © 2025-2026 Ino Jacob. This project is provided "as-is" for media management and performance-critical hashing tasks.
