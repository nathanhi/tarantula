#include "../src/tarantula.h"
#include <stdio.h>

int main() {
    struct tar_headers foo;
    int i = 0;
    foo = listArchiveContent("../sample/sample.tar");
    
    while (i < foo.files) {
        printf("filename: '%s'\n", foo.headers[i].filename);
        printf("filemode: '%i'\n", foo.headers[i].filemode);
        printf("owner_UID: '%i'\n", foo.headers[i].owner_UID);
        printf("owner_GID: '%i'\n", foo.headers[i].owner_GID);
        printf("filesize: '%lli'\n", foo.headers[i].filesize);
        printf("modification_time: '%lli'\n", foo.headers[i].modification_time);
        printf("checksum: '%s'\n", foo.headers[i].checksum);
        printf("typeflag: '%i'\n", foo.headers[i].typeflag);
        printf("linktarget: '%s'\n", foo.headers[i].linktarget);
        printf("ustarindicator: '%s'\n", foo.headers[i].ustarindicator);
        printf("ustarversion: '%i'\n", foo.headers[i].ustarversion);
        printf("owner_username: '%s'\n", foo.headers[i].owner_username);
        printf("owner_groupname: '%s'\n", foo.headers[i].owner_groupname);
        printf("device_majornumber: '%i'\n", foo.headers[i].device_majornumber);
        printf("device_minornumber: '%i'\n", foo.headers[i].device_minornumber);
        printf("filename_prefix: '%s'\n\n\n", foo.headers[i].filename_prefix);
        i++;
    }
    return 0;
}
