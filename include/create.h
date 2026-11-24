#pragma once

#include "fat32.h"

void cmd_mkdir(FileSystem *fs, const char *dirname);
void cmd_creat(FileSystem *fs, const char *filename);
