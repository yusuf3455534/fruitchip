#include <stdio.h>

#include <tamtypes.h>
#include <loadcore.h>
#include <excepman.h>
#include <ioman.h>
#include <iop_regs.h>

#include "fwfs.h"

IRX_ID(MODNAME, 1, 0);

int _start(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    if (init_fwfs() != MODULE_RESIDENT_END)
    {
        DPRINTF("Failed initializing FWFS!\n");
        return MODULE_NO_RESIDENT_END;
    }

    FlushIcache();
    FlushDcache();

    DPRINTF("FWFS installed!\n");

    return MODULE_RESIDENT_END;
}
