#include "platform.h"

#include <sys/mman.h> // mmap()
#include <unistd.h> // close(), getpagesize()

char *map_file_on_offset(tar_fle *tar_file, int *new_offset) {
    /* Maps only a specific part of the file
     * to memory: current offset. f can only be
     * accessed with the correct file-based offset
     */
    char *f;

    // Use native mmap() on POSIX-systems
    int pgsize = sysconf(_SC_PAGESIZE);
    *new_offset = (((tar_file->curpos/pgsize)+1)*pgsize)-pgsize;

    f = (char *) mmap(0, pgsize, PROT_READ, MAP_PRIVATE,
                      tar_file->fd, *new_offset);

    if (f == MAP_FAILED) {
        return NULL;
    }
    return f;
}

void unmap_file(char *f, tar_fle *tar_file) {
    /* unmap the file from memory */
    munmap(f, sysconf(_SC_PAGESIZE));
}