#include "fat32.h"

#include <ctype.h>
#include <string.h>

/* Opens the image, reads the boot sector, and sets the cwd to root.
 * Returns 0 on success, -1 on failure. */
int mount_image(FileSystem *fs, const char *path)
{
    memset(fs, 0, sizeof(*fs));

    fs->fp = fopen(path, "r+b");
    if (fs->fp == NULL) {
        fprintf(stderr, "Error: could not open image file '%s'\n", path);
        return -1;
    }

    if (fread(&fs->bs, sizeof(BootSector), 1, fs->fp) != 1) {
        fprintf(stderr, "Error: could not read boot sector\n");
        fclose(fs->fp);
        return -1;
    }

    /* Prompt shows just the file name, not the full path. */
    const char *name = strrchr(path, '/');
    name = (name != NULL) ? name + 1 : path;
    snprintf(fs->image_name, sizeof(fs->image_name), "%s", name);

    fs->first_data_sector = fs->bs.BPB_RsvdSecCnt +
                            fs->bs.BPB_NumFATs * fs->bs.BPB_FATSz32;
    fs->cwd_cluster = fs->bs.BPB_RootClus;
    strcpy(fs->cwd_path, "/");
    return 0;
}

void unmount_image(FileSystem *fs)
{
    if (fs->fp != NULL) {
        fclose(fs->fp);
        fs->fp = NULL;
    }
}

/* Byte offset in the image where a given data cluster starts. */
uint32_t cluster_to_offset(const FileSystem *fs, uint32_t cluster)
{
    uint32_t sector = fs->first_data_sector +
                      (cluster - 2) * fs->bs.BPB_SecPerClus;
    return sector * fs->bs.BPB_BytsPerSec;
}

/* Reads the FAT entry for a cluster (the next cluster in its chain).
 * A value >= 0x0FFFFFF8 means end of chain. */
uint32_t get_fat_entry(const FileSystem *fs, uint32_t cluster)
{
    uint32_t offset = fs->bs.BPB_RsvdSecCnt * fs->bs.BPB_BytsPerSec +
                      cluster * 4;
    uint32_t entry = 0;

    fseek(fs->fp, offset, SEEK_SET);
    fread(&entry, sizeof(entry), 1, fs->fp);
    return entry & 0x0FFFFFFF;
}

/* First data cluster of an entry, built from its high and low halves. */
uint32_t entry_cluster(const DirEntry *entry)
{
    return ((uint32_t)entry->DIR_FstClusHI << 16) | entry->DIR_FstClusLO;
}

/* Converts a name like "dir1" into the 11-byte, space-padded, uppercase
 * form stored on disk ("DIR1       "). "." and ".." are kept as-is. */
static void to_short_name(const char *name, uint8_t out[11])
{
    memset(out, ' ', 11);

    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        memcpy(out, name, strlen(name));
        return;
    }

    for (int i = 0; i < 11 && name[i] != '\0'; i++)
        out[i] = (uint8_t)toupper((unsigned char)name[i]);
}

/* Searches a directory for an entry with the given name. Skips deleted,
 * long-name, and volume label entries. On success, copies the entry into
 * *out, stores its byte offset in *offset_out (if not NULL), and returns 0.
 * Returns -1 if no entry has that name. */
int find_entry(FileSystem *fs, uint32_t dir_cluster, const char *name,
               DirEntry *out, uint32_t *offset_out)
{
    uint8_t target[11];
    uint32_t cluster_size = fs->bs.BPB_BytsPerSec * fs->bs.BPB_SecPerClus;
    uint32_t entries_per_cluster = cluster_size / sizeof(DirEntry);
    uint32_t cluster = dir_cluster;

    if (strlen(name) > 11)
        return -1;
    to_short_name(name, target);

    while (cluster >= 2 && cluster < END_OF_CHAIN) {
        uint32_t base = cluster_to_offset(fs, cluster);

        for (uint32_t i = 0; i < entries_per_cluster; i++) {
            DirEntry entry;
            uint32_t offset = base + i * sizeof(DirEntry);

            fseek(fs->fp, offset, SEEK_SET);
            fread(&entry, sizeof(DirEntry), 1, fs->fp);

            if (entry.DIR_Name[0] == ENTRY_END)
                return -1;
            if (entry.DIR_Name[0] == ENTRY_DELETED)
                continue;
            if ((entry.DIR_Attr & ATTR_LONG_NAME) == ATTR_LONG_NAME)
                continue;
            if (entry.DIR_Attr & ATTR_VOLUME_ID)
                continue;

            if (memcmp(entry.DIR_Name, target, 11) == 0) {
                *out = entry;
                if (offset_out != NULL)
                    *offset_out = offset;
                return 0;
            }
        }
        cluster = get_fat_entry(fs, cluster);
    }
    return -1;
}

void cmd_info(const FileSystem *fs)
{
    const BootSector *bs = &fs->bs;
    uint32_t data_sectors = bs->BPB_TotSec32 - fs->first_data_sector;
    uint32_t total_clusters = data_sectors / bs->BPB_SecPerClus;
    uint32_t fat_entries = bs->BPB_FATSz32 * bs->BPB_BytsPerSec / 4;
    uint64_t image_size = (uint64_t)bs->BPB_TotSec32 * bs->BPB_BytsPerSec;

    printf("Root cluster: %u\n", bs->BPB_RootClus);
    printf("Bytes per sector: %u\n", bs->BPB_BytsPerSec);
    printf("Sectors per cluster: %u\n", bs->BPB_SecPerClus);
    printf("Total clusters in data region: %u\n", total_clusters);
    printf("Entries in one FAT: %u\n", fat_entries);
    printf("Size of image (bytes): %llu\n", (unsigned long long)image_size);
}
