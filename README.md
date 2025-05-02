# FSplitter

**split a large file into multiple smaller files**.

---

## 🧰 Usage

```bash
./main [-h] [-c] [-b <block_size>] [-o <dir_name>] [-f: <FILE>]
```

### ✅ Parameters and Flags

| Flag | Description                                                              |
| ---- | ------------------------------------------------------------------------ |
| `-h` | Show the help menu and exit.                                             |
| `-c` | Compress the output files. Default is **false**.                         |
| `-b` | Specify the block size. Default is **4KB**.                              |
| `-o` | Output directory where the split files will be saved (default: **`.`**). |
| `-f` | Input file to be split (**required**).                                   |

---

## 📦 Block Size Format

The block size must be specified with a unit suffix (no spaces):

| Suffix | Meaning             |
| ------ | ------------------- |
| `B`    | Bytes               |
| `K`    | Kilobytes (1024 B)  |
| `M`    | Megabytes (1024 KB) |
| `G`    | Gigabytes (1024 MB) |
| `T`    | Terabytes (1024 GB) |

### Valid examples:

* `512B` → 512 bytes
* `4K` → 4096 bytes
* `2M` → 2 megabytes
* `1G` → 1 gigabyte

---

## 📂 Example Usages

### 1. Simple split with custom block size

```bash
./main -f myvideo.mp4 -b 10M
```

> Splits `myvideo.mp4` into 10 MB blocks in the current directory.

### 2. Split with compression and custom output folder

```bash
./main -f archive.zip -b 1M -c -o split_output/
```

> Splits `archive.zip` into 1 MB compressed blocks, saved in `split_output/`.

### 3. Display help menu

```bash
./main -h
```


# Installation

```bash
git clone https://github.com/0Lunar/FSplitter.git && \
cd FSplitter && make
```


## 📌 Notes

- **All split blocks have the `.fsp` extension, regardless of compression status.**
- **Compression is performed using the zlib library.**