#include "create.h"

#include <stdio.h>
#include <string.h>

/* Checks that a new name fits FAT's 11-character short-name limit and
 * is not reserved. Prints an error and returns -1 if it is invalid. */
static int validate_new_name(const char *name)
{
    size_t len = strlen(name);

    if (len == 0 || len > 11) {
        printf("Error: name must be 1 to 11 characters\n");
        return -1;
    }
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0 ||
        strchr(name, '/') != NULL) {
        printf("Error: '%s' is not a valid name\n", name);
        return -1;
    }
    return 0;
}

/* Fills in a directory entry with a name, attribute, and first cluster. */
static void build_entry(DirEntry *entry, const uint8_t name[11],
                        uint8_t attr, uint32_t cluster)
{
    memset(entry, 0, sizeof(*entry));
    memcpy(entry->DIR_Name, name, 11);
    entry->DIR_Attr = attr;
    entry->DIR_FstClusHI = (uint16_t)(cluster >> 16);
    entry->DIR_FstClusLO = (uint16_t)(cluster & 0xFFFF);
}

/* Creates a new, empty directory in the current working directory. */
void cmd_mkdir(FileSystem *fs, const char *dirname)
{
    DirEntry existing;

    if (validate_new_name(dirname) != 0)
        return;
    if (find_entry(fs, fs->cwd_cluster, dirname, &existing, NULL) == 0) {
        printf("Error: '%s' already exists\n", dirname);
        return;
    }

    uint32_t cluster = alloc_cluster(fs);
    if (cluster == 0) {
        printf("Error: no free space on the image\n");
        return;
    }

    /* Every directory except root starts with "." and "..".
     * ".." stores cluster 0 when the parent is the root. */
    uint8_t dot[11], dotdot[11];
    to_short_name(".", dot);
    to_short_name("..", dotdot);

    uint32_t parent = fs->cwd_cluster;
    if (parent == fs->bs.BPB_RootClus)
        parent = 0;

    DirEntry entries[2];
    build_entry(&entries[0], dot, ATTR_DIRECTORY, cluster);
    build_entry(&entries[1], dotdot, ATTR_DIRECTORY, parent);
    fseek(fs->fp, cluster_to_offset(fs, cluster), SEEK_SET);
    fwrite(entries, sizeof(DirEntry), 2, fs->fp);
    fflush(fs->fp);

    uint8_t name[11];
    DirEntry new_dir;
    to_short_name(dirname, name);
    build_entry(&new_dir, name, ATTR_DIRECTORY, cluster);

    if (add_dir_entry(fs, fs->cwd_cluster, &new_dir) != 0) {
        set_fat_entry(fs, cluster, 0);      /* give the cluster back */
        printf("Error: no free space on the image\n");
    }
}

/* Creates a new 0-byte file in the current working directory. An empty
 * file has no clusters, so its first cluster is 0. */
void cmd_creat(FileSystem *fs, const char *filename)
{
    DirEntry existing;

    if (validate_new_name(filename) != 0)
        return;
    if (find_entry(fs, fs->cwd_cluster, filename, &existing, NULL) == 0) {
        printf("Error: '%s' already exists\n", filename);
        return;
    }

    uint8_t name[11];
    DirEntry new_file;
    to_short_name(filename, name);
    build_entry(&new_file, name, ATTR_ARCHIVE, 0);

    if (add_dir_entry(fs, fs->cwd_cluster, &new_file) != 0)
        printf("Error: no free space on the image\n");
}
