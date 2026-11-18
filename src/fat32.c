#include "fat32.h"

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
