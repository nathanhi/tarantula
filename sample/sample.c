#include "../src/tarantula.h"
#include <stdio.h>

int main() {
	struct tar foo;
	foo = getHeader("../sample/sample.tar");
	printf("filename: '%s'\n", foo.filename);
	printf("filemode: '%i'\n", foo.filemode);
	printf("owner_UID: '%i'\n", foo.owner_UID);
	printf("owner_GID: '%i'\n", foo.owner_GID);
	printf("filesize: '%i'\n", foo.filesize);
	printf("modification_time: '%i'\n", foo.modification_time);
	printf("checksum: '%s'\n", foo.checksum);
	printf("typeflag: '%i'\n", foo.typeflag);
	printf("linktarget: '%s'\n", foo.linktarget);
	printf("ustarindicator: '%s'\n", foo.ustarindicator);
	printf("ustarversion: '%i'\n", foo.ustarversion);
	printf("owner_username: '%s'\n", foo.owner_username);
	printf("owner_groupname: '%s'\n", foo.owner_groupname);
	printf("device_majornumber: '%i'\n", foo.device_majornumber);
	printf("device_minornumber: '%i'\n", foo.device_minornumber);
	printf("filename_prefix: '%s'\n", foo.filename_prefix);
	return 0;
}
