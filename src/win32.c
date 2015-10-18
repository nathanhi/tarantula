#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#include <windows.h>  // HANDLE, CreateFileMapping, MapViewOfFile, UnmapViewOfFile
#include <io.h> //_get_osfhandle
#include "platform.h"

char *map_file_on_offset(tar_fle *tar_file, int *new_offset) {
    /* Maps only a specific part of the file
     * to memory: current offset. f can only be
     * accessed with the correct file-based offset
     */
    char *f;
    
    // Retrieve page size
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    int pgsize = sysinfo.dwPageSize;
    
    // Adjust offset according to pagesize
    /* Not required on Windows, but lets keep things
     * compatible
     */
    *new_offset = (((tar_file->curpos/pgsize)+1)*pgsize)-pgsize;

    // Convert file descriptor to HANDLE
    HANDLE fdhandle = _get_osfhandle(tar_file->fd);
    if (fdhandle == INVALID_HANDLE_VALUE)
        return NULL;

    // Create file mapping object for tar file
    HANDLE mmaphandle = CreateFileMapping(fdhandle, 0, PAGE_WRITECOPY,
                                          0, (DWORD)*new_offset, 0);
    if (mmaphandle == INVALID_HANDLE_VALUE)
        return NULL;

    // Map view of file mapping to memory
    f = MapViewOfFile(mmaphandle, FILE_MAP_COPY, 0, 0, (SIZE_T)*new_offset);
    if (f == NULL)
        return NULL;

    return f;
}

void unmap_file(char *f, tar_fle *tar_file) {
    /* unmap the file from memory */
	UnmapViewOfFile(f);
}