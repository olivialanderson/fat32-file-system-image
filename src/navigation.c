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
