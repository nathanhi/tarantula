#include "tarantula.h"
#include <stdio.h> // TODO: Remove / printf
#include <fcntl.h> // O_RDONLY
#include <stdlib.h> // atoi
#include <string.h> // memcpy

#if (defined(__unix__) || defined(__HAIKU__))
#define POSIX
#endif

#ifdef POSIX
#include <sys/stat.h> // fstat()
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
	
	/* Convert these valuese to int */
	norm_header.filemode = atoi(header->filemode);
	norm_header.owner_UID = atoi(header->owner_UID);
	norm_header.owner_GID = atoi(header->owner_GID);
	norm_header.filesize = atoi(header->filesize);
	norm_header.modification_time = atoi(header->modification_time);
	norm_header.typeflag = atoi(header->typeflag);
	norm_header.ustarversion = atoi(header->ustarversion);
	norm_header.device_majornumber = atoi(header->device_majornumber);
	norm_header.device_minornumber = atoi(header->device_minornumber);

	return norm_header;
}

tar getHeader(int offset, const char *tarfile) {
	struct tar_raw header;
	struct tar header_norm;

#ifdef POSIX
	char *f;
	struct stat s;
	int fd = open(tarfile, O_RDONLY);
	int curr_offset = 0;

	if ((fd >= 0) && (fstat(fd, &s) == 0)) {
		/* If open and fstat worked */
		f = (char *) mmap (0, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

		/* Map fields from header to respective variables in struct */
		memcpy(header.filename, f+FILENAME_OFFSET, sizeof(header.filename));
		memcpy(header.filemode, f+FILEMODE_OFFSET, sizeof(header.filemode));
		memcpy(header.owner_UID, f+UID_OFFSET, sizeof(header.owner_UID));
		memcpy(header.owner_GID, f+GID_OFFSET, sizeof(header.owner_GID));
		memcpy(header.filesize, f+FILESIZE_OFFSET, sizeof(header.filesize));
		memcpy(header.modification_time, f+MTIME_OFFSET, sizeof(header.modification_time));
		memcpy(header.checksum, f+CHECKSUM_OFFSET, sizeof(header.checksum));
		memcpy(header.typeflag, f+TYPEFLAG_OFFSET, sizeof(header.typeflag));
		memcpy(header.linktarget, f+LINKTARGET_OFFSET, sizeof(header.linktarget));
		memcpy(header.ustarindicator, f+USTARINDICATOR_OFFSET, sizeof(header.ustarindicator));
		memcpy(header.ustarversion, f+USTARVERSION_OFFSET, sizeof(header.ustarversion));
		memcpy(header.owner_username, f+USERNAME_OFFSET, sizeof(header.owner_username));
		memcpy(header.owner_groupname, f+GROUPNAME_OFFSET, sizeof(header.owner_groupname));
		memcpy(header.device_majornumber, f+DEVMAJORNUM_OFFSET, sizeof(header.device_majornumber));
		memcpy(header.device_minornumber, f+DEVMINORNUM_OFFSET, sizeof(header.device_minornumber));
		memcpy(header.filename_prefix, f+FILENAMEPREFIX_OFFSET, sizeof(header.filename_prefix));

		/* Convert from raw to normalized header struct */
		header_norm = __rawToNorm(&header);

		char data[header_norm.filesize];
		memcpy(data, f+HEADERLEN, sizeof(data));
		printf("data: %s\n", data);
	}
#endif
	return header_norm;
}
