#include "fs.h"
#include "../../klib/stdlib.h"
#include "../../klib/stdio.h"

/*
    API to work with filesystems, non-specific fs implementations should be here.
    Considered the VFS.
*/

#define MAX_DESCRIPTORS 256

struct FILE file_descriptors[MAX_DESCRIPTORS];

