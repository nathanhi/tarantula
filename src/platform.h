#include "tarantula.h"

char *map_file_on_offset(tar_fle *tar_file, int *new_offset);
void unmap_file(char *f, tar_fle *tar_file);
int tar_close(tar_fle *tar_file);