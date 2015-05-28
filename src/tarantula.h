#ifndef TARANTULA_H
#define TARANTULA_H

#include <sys/stat.h> // fstat()

#define FILENAME_OFFSET 0
#define FILEMODE_OFFSET 100
#define UID_OFFSET 108
#define GID_OFFSET 116
#define FILESIZE_OFFSET 124
#define MTIME_OFFSET 136
#define CHECKSUM_OFFSET 148
#define TYPEFLAG_OFFSET 156
#define LINKTARGET_OFFSET 157
#define USTARINDICATOR_OFFSET 257
#define USTARVERSION_OFFSET 263
#define USERNAME_OFFSET 265
#define GROUPNAME_OFFSET 297
#define DEVMAJORNUM_OFFSET 329
#define DEVMINORNUM_OFFSET 337
#define FILENAMEPREFIX_OFFSET 345
#define HEADERLEN 512

/* Field Offset  Field Size  Field
 * 0             100         File name
 * 100           8           File mode
 * 108           8           Owner's numeric user ID
 * 116           8           Group's numeric user ID
 * 124           12          File size in bytes (octal base)
 * 136           12          Last modification time in numeric Unix time format (octal)
 * 148           8           Checksum for header record
 * 156                       Type flag, possible values:
 *                            - 0: Normal file
 *                            - 1: Hard link
 *                            - 2: Symbolic link
 *                            - 3: Character special
 *                            - 4: Block special
 *                            - 5: Directory
 *                            - 6: FIFO
 *                            - 7: Contiguous file
 *                            - g: global extended header with metadata (POSIX.1-2001)
 *                            - x: extended header with metadata for the next file in the archive (POSIX.1-2001)
 *                            - A-Z: Vendor specific extensions (POSIX.1-1988)
 *                            - rest: reserved for future standardization
 * 157           100         Name of linked file
 * 257           6           UStar indicator "ustar"
 * 263           2           UStar version "00"
 * 265           32          Owner user name
 * 297           32          Owner group name
 * 329           8           Device major number
 * 337           8           Device minor number
 * 345           155         Filename prefix
 */

typedef struct tar {
    int offset;
    char filename[100];
    int filemode;
    int owner_UID;
    int owner_GID;
    long long int filesize;
    long long int modification_time;
    char checksum[8];
    int typeflag;
    char linktarget[100];
    char ustarindicator[6];
    int ustarversion;
    char owner_username[32];
    char owner_groupname[32];
    int device_majornumber;
    int device_minornumber;
    char filename_prefix[155];
} tar;

typedef struct tar_raw {
    char filename[100];
    char filemode[8];
    char owner_UID[8];
    char owner_GID[8];
    char filesize[12];
    char modification_time[12];
    char checksum[8];
    char typeflag[1];
    char linktarget[100];
    char ustarindicator[6];
    char ustarversion[2];
    char owner_username[32];
    char owner_groupname[32];
    char device_majornumber[8];
    char device_minornumber[8];
    char filename_prefix[155];
} tar_raw;

typedef struct tar_headers {
    tar *headers;
    int files;
} tar_headers;

typedef struct tar_fle {
    const char *filename;
    int fd;
    struct stat s;
    tar curheader;
    int curpos;
} tar_fle;

void get_file_from_archive(const char *tarfile, const char *filename);
int get_next_header(tar_fle *tar_file);
int tar_open(const char *tarfile, tar_fle *tar_file);
int tar_close(tar_fle *tar_file);

#endif /* TARANTULA_H */
