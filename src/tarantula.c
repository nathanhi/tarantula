#include "tarantula.h"
#include <stdio.h> // TODO: Remove / printf
#include <fcntl.h> // O_RDONLY
#include <stdlib.h> // atoi
#include <string.h> // memcpy
#include <sys/stat.h> // fstat()
#include <stdint.h> //intmax_t

#if (defined(_MSC_VER) || defined(__MINGW32__))
#define WINBUILD
#include <windows.h>
#endif

#if (defined(__unix__) || defined(__HAIKU__))
#define POSIX
#include <sys/mman.h> // mmap()
#endif

tar __raw_to_norm(tar_raw *header) {
    /* Converts a RAW tar header (all char,
     * no ints) to a normalized header (chars+ints)
     */
    tar norm_header;

    /* Keep these values as chars */
    strncpy(norm_header.filename, header->filename, 100);
    strncpy(norm_header.checksum, header->checksum, 8);
    strncpy(norm_header.linktarget, header->linktarget, 100);
    strncpy(norm_header.ustarindicator, header->ustarindicator, 6);
    strncpy(norm_header.owner_username, header->owner_username, 32);
    strncpy(norm_header.owner_groupname, header->owner_groupname, 32);
    strncpy(norm_header.filename_prefix, header->filename_prefix, 155);
    
    /* Convert these values to int */
    norm_header.filemode = atoi(header->filemode);
    norm_header.owner_UID = atoi(header->owner_UID);
    norm_header.owner_GID = atoi(header->owner_GID);
    norm_header.filesize = atoi(header->filesize);
    norm_header.typeflag = atoi(header->typeflag);
    norm_header.ustarversion = atoi(header->ustarversion);
    norm_header.device_majornumber = atoi(header->device_majornumber);
    norm_header.device_minornumber = atoi(header->device_minornumber);
    
    /* Convert these values to long long int */
    norm_header.filesize = strtoll(header->filesize, NULL, 8);
    norm_header.modification_time = strtoll(header->modification_time, NULL, 8);

    return norm_header;
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

int get_next_header(tar_fle *tar_file) {
    /* Used to iterate over all headers in archive
     * Header can then be used to retrieve a specific file.
     */
    tar_raw raw_header;
    char *f;
    struct stat s;
    int fd = open(tar_file->filename, O_RDONLY);

    if ((fd >= 0) && (fstat(fd, &s) == 0)) {
        /* If open and fstat worked */

#ifdef POSIX
        // Use native mmap() on POSIX-systems
        // TODO: We possibly want to pass curpos to mmap()
        f = (char *) mmap(0, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (f == MAP_FAILED) {
            // TODO: Correct error handling
            printf("mmap() failed!");
            return 0;
        }

#elif (defined(WINBUILD))
        // Use MapViewOfFile on Windows-systems
        // TODO: We possibly want to pass curpos to MapViewOfFile()
        HANDLE fdhandle = CreateFile(tar_file->filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        HANDLE mmaphandle = CreateFileMapping(fdhandle, 0, PAGE_WRITECOPY, 0, s.st_size, 0);
        f = MapViewOfFile(mmaphandle, FILE_MAP_COPY, 0, 0, s.st_size);
        if (f == NULL) {
            // TODO: Correct error handling
            return 0;
        }
#endif

        if (s.st_size-tar_file->curpos <= 1024) {
            // Break loop on file end (1K of free blocks)
            return 0;
        }

        /* Map fields from header to respective variables in struct */
        memcpy(raw_header.filename, f+tar_file->curpos+FILENAME_OFFSET, sizeof(raw_header.filename));
        memcpy(raw_header.filemode, f+tar_file->curpos+FILEMODE_OFFSET, sizeof(raw_header.filemode));
        memcpy(raw_header.owner_UID, f+tar_file->curpos+UID_OFFSET, sizeof(raw_header.owner_UID));
        memcpy(raw_header.owner_GID, f+tar_file->curpos+GID_OFFSET, sizeof(raw_header.owner_GID));
        memcpy(raw_header.filesize, f+tar_file->curpos+FILESIZE_OFFSET, sizeof(raw_header.filesize));
        memcpy(raw_header.modification_time, f+tar_file->curpos+MTIME_OFFSET, sizeof(raw_header.modification_time));
        memcpy(raw_header.checksum, f+tar_file->curpos+CHECKSUM_OFFSET, sizeof(raw_header.checksum));
        memcpy(raw_header.typeflag, f+tar_file->curpos+TYPEFLAG_OFFSET, sizeof(raw_header.typeflag));
        memcpy(raw_header.linktarget, f+tar_file->curpos+LINKTARGET_OFFSET, sizeof(raw_header.linktarget));
        memcpy(raw_header.ustarindicator, f+tar_file->curpos+USTARINDICATOR_OFFSET, sizeof(raw_header.ustarindicator));
        memcpy(raw_header.ustarversion, f+tar_file->curpos+USTARVERSION_OFFSET, sizeof(raw_header.ustarversion));
        memcpy(raw_header.owner_username, f+tar_file->curpos+USERNAME_OFFSET, sizeof(raw_header.owner_username));
        memcpy(raw_header.owner_groupname, f+tar_file->curpos+GROUPNAME_OFFSET, sizeof(raw_header.owner_groupname));
        memcpy(raw_header.device_majornumber, f+tar_file->curpos+DEVMAJORNUM_OFFSET, sizeof(raw_header.device_majornumber));
        memcpy(raw_header.device_minornumber, f+tar_file->curpos+DEVMINORNUM_OFFSET, sizeof(raw_header.device_minornumber));
        memcpy(raw_header.filename_prefix, f+tar_file->curpos+FILENAMEPREFIX_OFFSET, sizeof(raw_header.filename_prefix));

        /* Convert from raw to normalized header struct */
        // TODO: This might fail?
        tar_file->curheader = __raw_to_norm(&raw_header);

        tar_file->curheader.offset = tar_file->curpos;  // Add current offset to header
        tar_file->curpos = tar_file->curpos+HEADERLEN+tar_file->curheader.filesize;
        tar_file->curpos = ((tar_file->curpos/512)+1)*512;

    } else {
        // Return 0 on error / stop while loop
        // TODO: Correct error handling
        printf("Failed to open file!\n");
        return 0;
    }

    // Continue loop if no errors occured
    return 1;
}

tar_fle tar_open(const char *tarfile) {
    // TODO: Also move open() to this function
    tar header;
    header.offset = 0;
    tar_fle tar_file;
    tar_file.filename = tarfile;
    tar_file.curheader = header;
    tar_file.curpos = 0;
    return tar_file;
}
