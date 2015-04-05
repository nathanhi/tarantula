#include "../src/tarantula.h"
#include <stdio.h>

int main() {
    tar_fle tar_file = tar_open("../sample/sample.tar");

    while (get_next_header(&tar_file)) {
        printf("filename: %s\n", tar_file.curheader.filename);
        printf("filemode: '%i'\n", tar_file.curheader.filemode);
        printf("owner_UID: '%i'\n", tar_file.curheader.owner_UID);
        printf("owner_GID: '%i'\n", tar_file.curheader.owner_GID);
#if (defined(_MSC_VER) || defined(__MINGW32__))
        printf("filesize: '%I64u'\n", tar_file.curheader.filesize);
        printf("modification_time: '%I64u'\n", tar_file.curheader.modification_time);
#else
        printf("filesize: '%lli'\n", tar_file.curheader.filesize);
        printf("modification_time: '%lli'\n", tar_file.curheader.modification_time);
#endif
        printf("checksum: '%s'\n", tar_file.curheader.checksum);
        printf("typeflag: '%i'\n", tar_file.curheader.typeflag);
        printf("linktarget: '%s'\n", tar_file.curheader.linktarget);
        printf("ustarindicator: '%s'\n", tar_file.curheader.ustarindicator);
        printf("ustarversion: '%i'\n", tar_file.curheader.ustarversion);
        printf("owner_username: '%s'\n", tar_file.curheader.owner_username);
        printf("owner_groupname: '%s'\n", tar_file.curheader.owner_groupname);
        printf("device_majornumber: '%i'\n", tar_file.curheader.device_majornumber);
        printf("device_minornumber: '%i'\n", tar_file.curheader.device_minornumber);
        printf("filename_prefix: '%s'\n\n\n", tar_file.curheader.filename_prefix);
    }

    return 0;
}
