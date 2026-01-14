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

### Examples
**Standard Batch Processing:**
```
gh -l results.log movie1.mp4 movie2.mkv
```
**Background/Silent Processing:**
```
gh -s -l scan_results.txt /path/to/media/*.mp4
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
