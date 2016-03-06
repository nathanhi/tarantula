#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#include <windows.h>  // HANDLE, CreateFileMapping, MapViewOfFile, UnmapViewOfFile
#include <io.h> //_get_osfhandle, _close
#include "platform.h"

char *map_file_on_offset(tar_fle *tar_file, int *new_offset) {
    /* Maps only a specific part of the file
     * to memory: current offset. f can only be
     * accessed with the correct file-based offset
     */
    char *f = NULL;
    
    // Retrieve page size
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    const int pgsize = sysinfo.dwAllocationGranularity;
    
    // Adjust offset according to pagesize (called 
    // allocation granularity on Windows)
    *new_offset = (((tar_file->curpos/pgsize)+1)*pgsize)-pgsize;

    // Convert file descriptor to HANDLE
    HANDLE fdhandle = (HANDLE)_get_osfhandle(tar_file->fd);
    if (fdhandle == INVALID_HANDLE_VALUE)
        return NULL;

    // Create file mapping object for tar file
    HANDLE mmaphandle = CreateFileMapping(fdhandle, NULL, PAGE_READONLY,
                                          (DWORD)0, (DWORD)0, NULL);
    if (mmaphandle == INVALID_HANDLE_VALUE)
        return NULL;

    // Map view of file mapping to memory
    f = (char*)MapViewOfFile(mmaphandle, FILE_MAP_COPY, (DWORD)0, (DWORD)*new_offset, (SIZE_T)(tar_file->s.st_size-*new_offset));
    if (f == NULL)
        return NULL;

    return f;
}

void unmap_file(char *f, tar_fle *tar_file) {
    /* unmap the file from memory */
	UnmapViewOfFile(f);
}

int tar_close(tar_fle *tar_file) {
    return _close(tar_file->fd);
}

int fd_get_offset(int fd) {
    HANDLE fdhandle = (HANDLE)_get_osfhandle(fd);
    return (int)SetFilePointer(fdhandle, 0, NULL, FILE_CURRENT);
}

int fd_set_offset(int fd, int offset) {
    HANDLE fdhandle = (HANDLE)_get_osfhandle(fd);
    return (int)SetFilePointer(fdhandle, offset, NULL, FILE_BEGIN);
}

int fd_read(int fd, void *buffer, int count) {
    return _read(fd, buffer, count);
}
