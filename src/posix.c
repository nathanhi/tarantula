#include "platform.h"

#include <sys/mman.h> // mmap()
#include <unistd.h> // close(), getpagesize(), lseek()


char *map_file_on_offset(tar_fle *tar_file, int *new_offset) {
    /* Maps only a specific part of the file
     * to memory: current offset. f can only be
     * accessed with the correct file-based offset
     */
    char *f;

    // Use native mmap() on POSIX-systems
    const int pgsize = sysconf(_SC_PAGESIZE);
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

int tar_close(tar_fle *tar_file) {
    return close(tar_file->fd);
}

int fd_get_offset(int fd) {
    return lseek(fd, 0, SEEK_CUR);
}

int fd_set_offset(int fd, int offset) {
    return lseek(fd, offset, SEEK_SET);
}

int fd_read(int fd, void *buffer, int count) {
    return read(fd, buffer, count);
}
