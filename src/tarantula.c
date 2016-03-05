#include "tarantula.h"
#include <fcntl.h> // O_RDONLY
#include <stdlib.h> // atoi, strtoll
#include <string.h> // strncpy
#include "platform.h"

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
    strncpy(norm_header->typeflag, header->typeflag, 1);
    strncpy(norm_header->filename_prefix, header->filename_prefix, 155);

    /* Convert these values to int */
    norm_header->filemode = atoi(header->filemode);
    norm_header->owner_UID = atoi(header->owner_UID);
    norm_header->owner_GID = atoi(header->owner_GID);
    norm_header->filesize = atoi(header->filesize);
    norm_header->ustarversion = atoi(header->ustarversion);
    norm_header->device_majornumber = atoi(header->device_majornumber);
    norm_header->device_minornumber = atoi(header->device_minornumber);

    /* Convert these values to long long int */
    norm_header->filesize = strtoll(header->filesize, NULL, 8);
    norm_header->modification_time = strtoll(header->modification_time, NULL, 8);
}

char *extract_to_mem(tar_fle *tar_file, const char *filename, int *filesize) {
    // Extracts given file from given archive. Returns NULL if file not found.
    // TODO: Return latest file in archive if there is more than one file with the same name!
    tar_headers headers;
    char fname[255];
    strncpy(fname, filename, 255);

    get_all_headers(tar_file, &headers);

    for (int i=0; i <= headers.files; i++) {
        // Search for correct file
        if (strcmp(headers.headers[i].filename, fname) == 0 &&
            strcmp(headers.headers[i].typeflag, TYPE_FILE) == 0) {
            // Backup old offset and seek to new offset
            int old_fd_offset = fd_get_offset(tar_file->fd);
            fd_set_offset(tar_file->fd, headers.headers[i].offset+HEADERLEN);

            // Allocate buffer and write file to memory
            char *data = malloc(headers.headers[i].filesize);
            fd_read(tar_file->fd, data, headers.headers[i].filesize);

            // Restore old offset
            fd_set_offset(tar_file->fd, old_fd_offset);

            // Return data and filesize
            *filesize = headers.headers[i].filesize;
            return data;
        }
    }

    // Returns NULL if file not found
    return NULL;
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
        unmap_file(f, tar_file);
        return 0;
    }

    if (tar_file->s.st_size-tar_file->curpos <= HEADERLEN) {
        // Break loop on file end (less than 500 bytes)
        unmap_file(f, tar_file);
        return 0;
    }

    // Write raw data from file to tar_raw struct
    raw_header = (tar_raw*)(f+(tar_file->curpos-new_offset));

    /* Convert from raw to normalized header struct */
    __raw_to_norm(raw_header, &tar_file->curheader);

    tar_file->curheader.offset = tar_file->curpos;  // Add current offset to header
    tar_file->curpos = tar_file->curpos+HEADERLEN+tar_file->curheader.filesize;
    if (tar_file->curpos % 512 != 0) {
        // If the current position is not a multiple
        // of 512, align it to the next..
        tar_file->curpos = ((tar_file->curpos/512)+1)*512;
    }

    char empty_buf[HEADERLEN] = {0};
    if (memcmp(raw_header, empty_buf, sizeof(tar_raw)) == 0) {
        // Skip current header if it is empty
        unmap_file(f, tar_file);
        return get_next_header(tar_file);
    }

    // Unmap page from memory
    unmap_file(f, tar_file);

    // Continue loop if no errors occured
    return 1;
}

int get_all_headers_from_file(const char *tarfile, tar_headers *headers) {
    // Iterates over every header and fills a
    // dynamic array with every encountered header.
    // Handles tar_open() and tar_close() automatically.

    // Open tar file
    tar_fle tar_file;
    if (tar_open(tarfile, &tar_file) != 0) {
        return 1;
    }

    // Iterate over every header and fill a dynamic array
    get_all_headers(&tar_file, headers);

    // Close tar file
    tar_close(&tar_file);
    return 0;
}

void get_all_headers(tar_fle *tar_file, tar_headers *headers) {
    // Iterates over every header and fills a
    // dynamic array with every encountered header.
    // Variable declaration for dynamic array
    tar *tar_array = {NULL};
    int arsize = 0;
    int aritems = 0;

    // Preparation in case tar_fle's FD is already used / on a different offset
    int old_offset = tar_file->curpos;
    int old_fd_offset = fd_get_offset(tar_file->fd);
    fd_set_offset(tar_file->fd, 0);
    tar_file->curpos = 0;

    while(get_next_header(tar_file)) {
        // Iterate over headers of tar file
        // Add struct to dynamic array
        arsize += sizeof(tar);
        void *_artmp = realloc(tar_array, arsize);
        tar_array = _artmp;
        tar_array[aritems] = tar_file->curheader;
        aritems++;
    }

    // Build final data type tar_headers
    headers->headers = tar_array;
    headers->files = aritems;

    // Reset offset
    tar_file->curpos = old_offset;
    fd_set_offset(tar_file->fd, old_fd_offset);
}

int tar_open(const char *tarfile, tar_fle *tar_file) {
    //_tar_open(const char *tarfile, tar_fle *tar_file)
    tar header;
    header.offset = 0;
    tar_file->filename = tarfile;
    tar_file->curheader = header;
    tar_file->curpos = 0;

    tar_file->fd = open(tarfile, O_RDONLY);

    if ((tar_file->fd < 0) || (fstat(tar_file->fd, &tar_file->s) != 0))
        return 1;

    return 0;
}
