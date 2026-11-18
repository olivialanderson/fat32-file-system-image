#pragma once

#include <stdint.h>
#include <stdio.h>

/* Boot sector fields we care about (BPB = BIOS Parameter Block).
 * Packed so the struct matches the on-disk byte layout exactly. */
typedef struct __attribute__((packed)) {
    uint8_t  BS_jmpBoot[3];
    uint8_t  BS_OEMName[8];
    uint16_t BPB_BytsPerSec;   /* offset 11 */
    uint8_t  BPB_SecPerClus;   /* offset 13 */
    uint16_t BPB_RsvdSecCnt;   /* offset 14 */
    uint8_t  BPB_NumFATs;      /* offset 16 */
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t  BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32;     /* offset 32 */
    uint32_t BPB_FATSz32;      /* offset 36 */
    uint16_t BPB_ExtFlags;
    uint16_t BPB_FSVer;
    uint32_t BPB_RootClus;     /* offset 44 */
} BootSector;

/* One 32-byte directory entry, exactly as stored on disk. */
typedef struct __attribute__((packed)) {
    uint8_t  DIR_Name[11];     /* name, padded with spaces */
    uint8_t  DIR_Attr;         /* 0x10 = directory, 0x0F = long name */
    uint8_t  DIR_NTRes;
    uint8_t  DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI;    /* high 16 bits of first cluster */
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;    /* low 16 bits of first cluster */
    uint32_t DIR_FileSize;
} DirEntry;

#define ATTR_DIRECTORY 0x10
#define ATTR_LONG_NAME 0x0F
#define ATTR_VOLUME_ID 0x08    /* disk label, not a real file */
#define ENTRY_END      0x00    /* first name byte: no more entries */
#define ENTRY_DELETED  0xE5    /* first name byte: deleted entry */
#define END_OF_CHAIN   0x0FFFFFF8

/* Everything the program needs to know about the mounted image. */
typedef struct {
    FILE *fp;                  /* open handle to the image file */
    char image_name[256];      /* shown in the prompt */
    BootSector bs;
    uint32_t first_data_sector;
    uint32_t cwd_cluster;      /* cluster of the current working directory */
    char cwd_path[512];        /* e.g. "/" or "/DIR1/" */
} FileSystem;

int  mount_image(FileSystem *fs, const char *path);
void unmount_image(FileSystem *fs);
void cmd_info(const FileSystem *fs);

/* Shared helpers other commands will build on. */
uint32_t cluster_to_offset(const FileSystem *fs, uint32_t cluster);
uint32_t get_fat_entry(const FileSystem *fs, uint32_t cluster);
