#include "tarantula.h"
#include <stdio.h> // TODO: Remove / printf
#include <fcntl.h> // O_RDONLY
#include <stdlib.h> // atoi
#include <string.h> // memcpy
#include <stdint.h> // intmax_t
#include <unistd.h> // close(), getpagesize()

#if (defined(_MSC_VER) || defined(__MINGW32__))
#define WINBUILD
#include <windows.h>
#endif

#if (defined(__unix__) || defined(__HAIKU__))
#define POSIX
#include <sys/mman.h> // mmap()
#endif

void __raw_to_norm(tar_raw *header, tar *norm_header) {
    /* Converts a RAW tar header (all char,
     * no ints) to a normalized header (chars+ints)
     */

    /* Keep these values as chars */
    strncpy(norm_header->filename, header->filename, 100);
    strncpy(norm_header->checksum, header->checksum, 8);
    strncpy(norm_header->linktarget, header->linktarget, 100);
    strncpy(norm_header->ustarindicator, header->ustarindicator, 6);
    strncpy(norm_header->owner_username, header->owner_username, 32);
    strncpy(norm_header->owner_groupname, header->owner_groupname, 32);
    strncpy(norm_header->filename_prefix, header->filename_prefix, 155);
    
    /* Convert these values to int */
    norm_header->filemode = atoi(header->filemode);
    norm_header->owner_UID = atoi(header->owner_UID);
    norm_header->owner_GID = atoi(header->owner_GID);
    norm_header->filesize = atoi(header->filesize);
    norm_header->typeflag = atoi(header->typeflag);
    norm_header->ustarversion = atoi(header->ustarversion);
    norm_header->device_majornumber = atoi(header->device_majornumber);
    norm_header->device_minornumber = atoi(header->device_minornumber);
    
    /* Convert these values to long long int */
    norm_header->filesize = strtoll(header->filesize, NULL, 8);
    norm_header->modification_time = strtoll(header->modification_time, NULL, 8);
}

void get_file_from_archive(const char *tarfile, const char *filename) {
    /*struct tar_headers headers = list_archive_content(tarfile);
    for (int i=0; i <= headers.files; i++) {
        if (strcmp(headers.headers[i].filename, filename) == 0) {
            // Read data from current file
            char *data = malloc(header_norm.filesize);
            memcpy(data, f+curpos+HEADERLEN, header_norm.filesize);
            printf("data: '%s'\n", data);
            printf("%s", headers.headers[i].);
        }
    }*/
}

char *map_file_on_offset(tar_fle *tar_file, int *new_offset) {
    /* Maps only a specific part of the file
     * to memory: current offset. f can only be
     * accessed with the correct file-based offset
     */
    char *f;

#ifdef POSIX
    // Use native mmap() on POSIX-systems
    int pgsize = sysconf(_SC_PAGESIZE);
    *new_offset = (((tar_file->curpos/pgsize)+1)*pgsize)-pgsize;

    f = (char *) mmap(0, pgsize, PROT_READ, MAP_PRIVATE,
                      tar_file->fd, *new_offset);

    if (f == MAP_FAILED) {
#ifdef DEBUG
        fprintf(stderr, "tarantula: mmap() failed: %s\n", f);
#endif
        return NULL;
#ifdef DEBUG
    } else {
        printf("tarantula: mmap (offset: '%i') SUCCESS\n", *new_offset);
#endif
    }

#elif (defined(WINBUILD))
    // Use MapViewOfFile on Windows-systems
    // TODO: Adapt this to the used pagesize boundary model above
    HANDLE fdhandle = CreateFile(tar_file->filename,
                                 GENERIC_READ,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL, OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE mmaphandle = CreateFileMapping(fdhandle, 0, PAGE_WRITECOPY,
                                          0, s.st_size, 0);
    f = MapViewOfFile(mmaphandle, FILE_MAP_COPY, 0, 0, s.st_size);
    if (f == NULL) {
        // TODO: Correct error handling
        return NULL;
    }
#endif
    return f;
}

void unmap_file(char *f, tar_fle *tar_file) {
    /* unmap the file from memory */
#ifdef POSIX
    if (munmap(f, sysconf(_SC_PAGESIZE)) != 0) {
#ifdef DEBUG
        fprintf(stderr, "tarantula: munmap failed!\n");
    } else {
        printf("tarantula: munmap SUCCESS\n");
#endif
    }
#elif (defined(WINBUILD))
    DO STUFF HERE! TODO!
#endif
}

int get_next_header(tar_fle *tar_file) {
    /* Used to iterate over all headers in archive
     * Header can then be used to retrieve a specific file.
     */
    tar_raw *raw_header;
    int new_offset;
    char *f = map_file_on_offset(tar_file, &new_offset);
    
    if (f == NULL) {
        // Break loop if mmap failed
        // Try to undo mmap
        unmap_file(f,tar_file);
        return 0;
    }

    if (tar_file->s.st_size-tar_file->curpos <= HEADERLEN) {
        // Break loop on file end (less than 500 bytes)
#ifdef DEBUG
        printf("tarantula: File end reached: (%i/%li)\n",
               tar_file->curpos, tar_file->s.st_size);
#endif
        unmap_file(f, tar_file);
        return 0;
    }

    // Write raw data from file to tar_raw struct
    raw_header = (tar_raw*)((f-new_offset)+tar_file->curpos);

    /* Convert from raw to normalized header struct */
    // TODO: This might fail?
    __raw_to_norm(raw_header, &tar_file->curheader);

    tar_file->curheader.offset = tar_file->curpos;  // Add current offset to header
    tar_file->curpos = tar_file->curpos+HEADERLEN+tar_file->curheader.filesize;
    if (tar_file->curpos % 512 != 0) {
        // If the current position is not a multiple
        // of 512, align it to the next..
        tar_file->curpos = ((tar_file->curpos/512)+1)*512;
    }

    char empty_buf[HEADERLEN] = { 0 };
    if (memcmp(raw_header, empty_buf, sizeof(tar_raw)) == 0) {
        // Skip current header if it is empty
        unmap_file(f, tar_file);
#ifdef DEBUG
        printf("tarantula: Skipping current header as it is empty\n");
#endif
        return get_next_header(tar_file);
    }

    // Continue loop if no errors occured
    unmap_file(f, tar_file);
    return 1;
}

int tar_open(const char *tarfile, tar_fle *tar_file) {
    tar header;
    header.offset = 0;
    tar_file->filename = tarfile;
    tar_file->curheader = header;
    tar_file->curpos = 0;

    tar_file->fd = open(tarfile, O_RDONLY);

    if ((tar_file->fd < 0) || (fstat(tar_file->fd, &tar_file->s) != 0)) {
#ifdef DEBUG
        fprintf(stderr, "tarantula: Failed to open file '%s'\n", tarfile);
#endif
        return 1;
    }
#ifdef DEBUG
    printf("tarantula: Opened file '%s'\n", tarfile);
#endif
    return 0;
}

int tar_close(tar_fle *tar_file) {
    return close(tar_file->fd);
}
