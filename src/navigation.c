#include "navigation.h"

#include <stdio.h>
#include <string.h>

/* Turns an 11-byte space-padded name into a normal C string. */
static void format_name(const uint8_t raw[11], char out[13])
{
    int len = 0;

    for (int i = 0; i < 8 && raw[i] != ' '; i++)
        out[len++] = raw[i];

    if (raw[8] != ' ') {
        out[len++] = '.';
        for (int i = 8; i < 11 && raw[i] != ' '; i++)
            out[len++] = raw[i];
    }
    out[len] = '\0';
}

/* Prints every entry in the current directory, following its
 * cluster chain until the end-of-directory marker. */
void cmd_ls(FileSystem *fs)
{
    uint32_t cluster_size = fs->bs.BPB_BytsPerSec * fs->bs.BPB_SecPerClus;
    uint32_t entries_per_cluster = cluster_size / sizeof(DirEntry);
    uint32_t cluster = fs->cwd_cluster;

    while (cluster >= 2 && cluster < END_OF_CHAIN) {
        uint32_t base = cluster_to_offset(fs, cluster);

        for (uint32_t i = 0; i < entries_per_cluster; i++) {
            DirEntry entry;
            fseek(fs->fp, base + i * sizeof(DirEntry), SEEK_SET);
            fread(&entry, sizeof(DirEntry), 1, fs->fp);

            if (entry.DIR_Name[0] == ENTRY_END)
                return;
            if (entry.DIR_Name[0] == ENTRY_DELETED)
                continue;
            if ((entry.DIR_Attr & ATTR_LONG_NAME) == ATTR_LONG_NAME)
                continue;
            if (entry.DIR_Attr & ATTR_VOLUME_ID)
                continue;

            char name[13];
            format_name(entry.DIR_Name, name);
            printf("%s\n", name);
        }
        cluster = get_fat_entry(fs, cluster);
    }
}

/* Removes the last folder from a path: "/A/B/" becomes "/A/". */
static void path_go_up(char *path)
{
    size_t len = strlen(path);

    if (len <= 1)               /* already at "/" */
        return;
    path[len - 1] = '\0';       /* drop the trailing '/' */
    char *last = strrchr(path, '/');
    last[1] = '\0';
}

/* Changes the current working directory to dirname. */
void cmd_cd(FileSystem *fs, const char *dirname)
{
    int at_root = (fs->cwd_cluster == fs->bs.BPB_RootClus);

    /* The FAT32 root has no "." or ".." entries, so stay put. */
    if (at_root && (strcmp(dirname, ".") == 0 || strcmp(dirname, "..") == 0))
        return;

    DirEntry entry;
    if (find_entry(fs, fs->cwd_cluster, dirname, &entry, NULL) != 0) {
        printf("Error: '%s' does not exist\n", dirname);
        return;
    }
    if (!(entry.DIR_Attr & ATTR_DIRECTORY)) {
        printf("Error: '%s' is not a directory\n", dirname);
        return;
    }

    /* A ".." entry that points to the root stores cluster 0. */
    uint32_t cluster = entry_cluster(&entry);
    if (cluster == 0)
        cluster = fs->bs.BPB_RootClus;

    if (strcmp(dirname, "..") == 0) {
        path_go_up(fs->cwd_path);
    } else if (strcmp(dirname, ".") != 0) {
        char name[13];
        format_name(entry.DIR_Name, name);
        if (strlen(fs->cwd_path) + strlen(name) + 2 > sizeof(fs->cwd_path)) {
            printf("Error: path too long\n");
            return;
        }
        strcat(fs->cwd_path, name);
        strcat(fs->cwd_path, "/");
    }
    fs->cwd_cluster = cluster;
}
