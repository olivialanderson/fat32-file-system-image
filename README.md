# FAT32 File System

COP4610 Project 3. This project is a user-space, shell-like utility, written in C, that mounts
a FAT32 image file and runs commands against it without corrupting it. It parses the boot
sector, follows cluster chains through the FAT, and reads and writes directory entries directly
in the image.

## Group Members

- **Olivia Anderson**
- **Gabriel Valladares-Ruiz**
- **Gannon Wooley**

## Division of Labor

### Shared Infrastructure
- **Responsibilities**: Boot sector and directory entry structs, cluster/FAT helpers, the
  Makefile and repo layout, and the shell loop built on the provided lexer.
- **Assigned to**: Olivia Anderson, Gabriel Valladares-Ruiz, Gannon Wooley

### Part 1: Mounting the Image
- **Responsibilities**: Open the image from `argv[1]`, show the `[IMAGE]/[PATH]/>` prompt, and
  implement `info` and `exit`.
- **Assigned to**: Olivia Anderson

### Part 2: Navigation
- **Responsibilities**: Track the current working directory and implement `cd` and `ls`.
- **Assigned to**: Olivia Anderson

### Part 3: Create
- **Responsibilities**: Free-cluster allocation, adding directory entries, and `mkdir` and
  `creat`.
- **Assigned to**: Gabriel Valladares-Ruiz

### Part 4: Read
- **Responsibilities**: The open-file table and `open`, `close`, `lsof`, `lseek`, and `read`.
- **Assigned to**: Gannon Wooley

### Part 5: Update
- **Responsibilities**: `write` (extending files as needed) and `mv`.
- **Assigned to**: Gannon Wooley, Olivia Anderson

### Part 6: Delete
- **Responsibilities**: Freeing cluster chains and implementing `rm` and `rmdir`.
- **Assigned to**: Gabriel Valladares-Ruiz

## File Listing

```text
filesys/
├── include/
│   ├── fat32.h
│   ├── lexer.h
│   └── navigation.h
├── src/
│   ├── fat32.c
│   ├── lexer.c
│   ├── main.c
│   └── navigation.c
├── .gitignore
├── division_of_labor.md
├── Makefile
└── README.md
```

## How to Compile & Execute

### Requirements

- **Compiler**: `gcc` (C99)
- **Tools**: `make`
- **Image**: `fat32.img` from the course files, extracted into the repository root

### Compilation

```bash
make
```
This builds the executable at `bin/filesys`. Run `make clean` to remove `bin/` and `obj/`.

### Execution

```bash
./bin/filesys fat32.img
```
or

```bash
make run
```
The program prints a `fat32.img/>` prompt and accepts the commands below. The program exits
with an error if the image file does not exist.

## Commands

| Command | Status |
|---------|--------|
| `info` | Done |
| `exit` | Done |
| `ls` | Done |
| `cd [DIRNAME]` | Done |
| `mkdir [DIRNAME]` | Not started |
| `creat [FILENAME]` | Not started |
| `open [FILENAME] [FLAGS]` | Not started |
| `close [FILENAME]` | Not started |
| `lsof` | Not started |
| `lseek [FILENAME] [OFFSET]` | Not started |
| `read [FILENAME] [SIZE]` | Not started |
| `write [FILENAME] "[STRING]"` | Not started |
| `mv [SOURCE] [DEST]` | Not started |
| `rm [FILENAME]` | Not started |
| `rmdir [DIRNAME]` | Not started |

## Development Log

Each member records their contributions here.

### Olivia Anderson

| Date       | Work Completed / Notes |
|------------|------------------------|
| 2026-11-18 | Set up the repo layout, Makefile, and `.gitignore`; added the provided lexer |
| 2026-11-18 | Implemented image mounting, the shell prompt, `info`, and `exit` |
| 2026-11-18 | Implemented `ls` (skips deleted, long-name, and volume label entries) |
| 2026-11-24 | Added the shared `find_entry` lookup and implemented `cd` (including `.` and `..`) |

### Gabriel Valladares-Ruiz

| Date       | Work Completed / Notes |
|------------|------------------------|

### Gannon Wooley

| Date       | Work Completed / Notes |
|------------|------------------------|

## Known Limitations

- The root directory has no `.` and `..` entries in FAT32, so `ls` at the root does not list
  them. Other directories list them from their stored entries.
- The Makefile does not track header dependencies. Run `make clean` before `make` after
  changing a file in `include/`.

## Considerations

- Run `make clean` before staging changes. Do not commit `bin/`, `obj/`, or the image file;
  `.gitignore` excludes them.
- Test against a fresh copy of `fat32.img` so a bad write can be undone by re-extracting it.
- AI use: Claude (Anthropic) helped with setup steps and the mounting, `info`, `exit`, `ls`, and
  `cd` code. The team reviewed and tested all output.
