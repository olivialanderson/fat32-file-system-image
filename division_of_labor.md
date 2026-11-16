# Division of Labor

COP4610 Project 3: FAT32 File System Utility

## Group Members
- **Olivia Anderson**
- **Gabriel Valladares-Ruiz**
- **Gannon Wooley**

## Shared Infrastructure (built first, used by every part)
- **Responsibilities**: Boot sector struct and parsing, FAT entry read/write helpers, cluster-to-
  sector/offset math, cluster chain traversal, directory entry struct, and a shared "find entry by
  name in a directory" lookup function. Set up the repo layout (`src/`, `include/`, `bin/`),
  the Makefile, and integrate the provided lexer for the shell loop.
- **Assigned to**: Olivia Anderson, Gabriel Valladares-Ruiz, Gannon Wooley

## Part 1: Mounting the Image
- **Responsibilities**: Open the image from `argv[1]` and exit with an error if it does not exist.
  Read and store the boot sector. Run the shell loop with the `[IMAGE_NAME]/[PATH]/>` prompt.
  Implement `info` (root cluster, bytes per sector, sectors per cluster, total data-region
  clusters, entries per FAT, image size in bytes) and `exit` (close the image and free all
  resources).
- **Assigned to**: Olivia Anderson

## Part 2: Navigation
- **Responsibilities**: Track the current working directory (its cluster and path string). Implement
  `cd [DIRNAME]`, including `.` and `..`, with errors for names that are missing or not a
  directory. Implement `ls` to print every entry in the current directory, including `.` and `..`,
  while skipping long-name and deleted entries.
- **Assigned to**: Olivia Anderson

## Part 3: Create
- **Responsibilities**: Write a free-cluster allocator (scan the FAT, mark end-of-chain, and keep
  every FAT copy in sync) and a function that writes a new directory entry, extending the parent
  directory's cluster chain when it is full. Implement `mkdir [DIRNAME]` (allocate a cluster and
  write the `.` and `..` entries) and `creat [FILENAME]` (0-byte file, no cluster). Both commands
  print an error if the name already exists.
- **Assigned to**: Gabriel Valladares-Ruiz

## Part 4: Read
- **Responsibilities**: Build the open-file table (max 10 entries, each storing name, mode, offset,
  path, first cluster, and size). Implement `open [FILENAME] [-r|-w|-rw|-wr]`, `close`, `lsof`,
  `lseek [FILENAME] [OFFSET]` (error if the offset is past the file size), and `read [FILENAME]
  [SIZE]` (follow the cluster chain from the offset, stop at EOF, and update the offset). Handle
  every error case listed in the spec.
- **Assigned to**: Gannon Wooley

## Part 5: Update
- **Responsibilities**: Implement `write [FILENAME] "[STRING]"`. It writes at the current offset,
  extends the file's cluster chain using Part 3's allocator when needed, updates the file size in
  its directory entry, and advances the offset. It errors on missing files, directories, and files
  not opened for writing. Implement `mv [SRC] [DEST]` to rename an entry, or to move it into an
  existing directory (updating `..` when a directory is moved). It errors if the source is missing,
  the destination is a file, or the file is open.
- **Assigned to**: Gannon Wooley (`write`), Olivia Anderson (`mv`)

## Part 6: Delete
- **Responsibilities**: Write a function that frees a cluster chain in every FAT copy and marks a
  directory entry as deleted. Implement `rm [FILENAME]`, which errors if the name is missing, is a
  directory, or is open. Implement `rmdir [DIRNAME]`, which works only on empty directories and
  errors if the name is missing, is not a directory, is not empty, or has a file open inside it.
- **Assigned to**: Gabriel Valladares-Ruiz

## Testing & Documentation
- **Responsibilities**: Each member tests their own commands against a fresh copy of `fat32.img` and
  checks the image's integrity by mounting it with a loopback device and with `hexedit`. Everyone
  updates the README together before submission: the final division of labor, file listing, bugs,
  and the compile and run instructions.
- **Assigned to**: Olivia Anderson, Gabriel Valladares-Ruiz, Gannon Wooley
