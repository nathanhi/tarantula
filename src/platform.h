#include "tarantula.h"

#if !defined(O_BINARY)
// Needed for O_BINARY on Windows...
#define O_BINARY 0
#endif

char *map_file_on_offset(tar_fle *tar_file, int *new_offset);
void unmap_file(char *f, tar_fle *tar_file);
int tar_close(tar_fle *tar_file);
int fd_get_offset(int fd);
int fd_set_offset(int fd, int offset);
int fd_read(int fd, void *buffer, int count);
