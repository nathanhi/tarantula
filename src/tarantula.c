#include "tarantula.h"
#include <stdio.h> // TODO: Remove / printf
#include <fcntl.h> // O_RDONLY
#include <stdlib.h> // atoi
#include <string.h> // memcpy
#include <sys/stat.h> // fstat()
#include <stdint.h> //intmax_t

#if (defined(_MSC_VER))
#include <windows.h>
#endif

#if (defined(__unix__) || defined(__HAIKU__))
#define POSIX
#endif

#ifdef POSIX
#include <sys/mman.h> // mmap()
#endif

tar __rawToNorm(struct tar_raw *header) {
    /* Converts a RAW tar header (all char,
     * no ints) to a normalized header (chars+ints)
     */
    struct tar norm_header;

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

tar_headers listArchiveContent(const char *tarfile) {
    struct tar_raw header;
    struct tar header_norm;
    struct tar_headers headers;

    char *f;
    struct stat s;
    int fd = open(tarfile, O_RDONLY);

    if ((fd >= 0) && (fstat(fd, &s) == 0)) {
        /* If open and fstat worked */
#ifdef POSIX
        f = (char *) mmap (0, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
#elif (defined(_MSC_VER))
        HANDLE fdhandle = CreateFile(tarfile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        HANDLE mmaphandle = CreateFileMapping(fdhandle, 0, PAGE_WRITECOPY, 0, s.st_size, 0);
        f = MapViewOfFile(mmaphandle, FILE_MAP_COPY, 0, 0, s.st_size);
#endif

        int curpos = 0;
        tar *tarHeaders = NULL;
        int aritems = 0;
        int arsize = 0;

        while (s.st_size-curpos > 1024) {
            //printf("%li-%i (%li) > 1024\n", s.st_size, curpos, s.st_size-curpos);
            /* Iterate over archive */

            /* Map fields from header to respective variables in struct */
            memcpy(header.filename, f+curpos+FILENAME_OFFSET, sizeof(header.filename));
            memcpy(header.filemode, f+curpos+FILEMODE_OFFSET, sizeof(header.filemode));
            memcpy(header.owner_UID, f+curpos+UID_OFFSET, sizeof(header.owner_UID));
            memcpy(header.owner_GID, f+curpos+GID_OFFSET, sizeof(header.owner_GID));
            memcpy(header.filesize, f+curpos+FILESIZE_OFFSET, sizeof(header.filesize));
            memcpy(header.modification_time, f+curpos+MTIME_OFFSET, sizeof(header.modification_time));
            memcpy(header.checksum, f+curpos+CHECKSUM_OFFSET, sizeof(header.checksum));
            memcpy(header.typeflag, f+curpos+TYPEFLAG_OFFSET, sizeof(header.typeflag));
            memcpy(header.linktarget, f+curpos+LINKTARGET_OFFSET, sizeof(header.linktarget));
            memcpy(header.ustarindicator, f+curpos+USTARINDICATOR_OFFSET, sizeof(header.ustarindicator));
            memcpy(header.ustarversion, f+curpos+USTARVERSION_OFFSET, sizeof(header.ustarversion));
            memcpy(header.owner_username, f+curpos+USERNAME_OFFSET, sizeof(header.owner_username));
            memcpy(header.owner_groupname, f+curpos+GROUPNAME_OFFSET, sizeof(header.owner_groupname));
            memcpy(header.device_majornumber, f+curpos+DEVMAJORNUM_OFFSET, sizeof(header.device_majornumber));
            memcpy(header.device_minornumber, f+curpos+DEVMINORNUM_OFFSET, sizeof(header.device_minornumber));
            memcpy(header.filename_prefix, f+curpos+FILENAMEPREFIX_OFFSET, sizeof(header.filename_prefix));

            /* Convert from raw to normalized header struct */
            header_norm = __rawToNorm(&header);

            /* // Read data from current file
             * char *data = malloc(header_norm.filesize);
             * memcpy(data, f+curpos+HEADERLEN, header_norm.filesize);
             * printf("data: '%s'\n", data);
             */

            /* // Debug output
             * printf("curpos: %i\n", curpos);
             * printf("HEADERLEN: %i\n", HEADERLEN);
             * printf("header_norm.filesize: %lli\n", header_norm.filesize);
             * printf("END == %lli\n\n", curpos+HEADERLEN+header_norm.filesize);
             */

            /* Add header size + file size to current position.
             * Blocksize is 512, so we also have to round up
             * to the next block (for the next file / or end).
             */
            curpos = curpos+HEADERLEN+header_norm.filesize;
            curpos = ((curpos/512)+1)*512;

            // Here be dragons
            // Add struct (tar header_norm) to dynamic array
            arsize += sizeof(tar);
            void *_artmp = realloc(tarHeaders, arsize);
            tarHeaders = (tar*)_artmp;
            tarHeaders[aritems] = header_norm;
            aritems++;
        }

        /* Add array to struct including number of items */
        headers.headers = tarHeaders;
        headers.files = aritems;
    }
    return headers;
}
