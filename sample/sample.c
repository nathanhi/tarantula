#include "../src/tarantula.h"
#include <stdio.h>
#include <stdlib.h>

void example_get_all_headers() {
    // Example for function get_all_headers(),
    // which retrieves all headers at once from the archive
    // and returns an array of items.

    // Call get_all_headers()
    tar_headers header_array = get_all_headers("../sample/sample.tar");

    // Parse data from get_all_headers.
    printf("Number of files in archive: %i\n", header_array.files);
    for(int i = 0; i<header_array.files; i++) {
        // Iterate over every item in the array and print filename
        printf("%i. filename: \'%s\'\n", i+1, header_array.headers[i].filename);
    }
}

void example_get_next_header() {
    // Example for function get_next_header(),
    // which can be used for iteration over all
    // headers using a for loop.
    
    // Open tar file
    tar_fle tar_file;
    if (tar_open("../sample/sample.tar", &tar_file)) {
        printf("Failed to open archive file. Exiting.\n");
        exit(1);
    }

    // Iterate over every header in the tar file
    while (get_next_header(&tar_file)) {
        printf("filename: '%s'\n", tar_file.curheader.filename);
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
        printf("filename_prefix: '%s'\n", tar_file.curheader.filename_prefix);
        printf("--------------------------------\n");
    }

    // Close tar file
    tar_close(&tar_file);
}

int main() {
    printf("Example of get_all_headers():\n");
    printf("=============================\n");
    example_get_all_headers();
    
    printf("\n\nExample of get_next_header():\n");
    printf("=============================\n");
    example_get_next_header();
    return 0;
}
