#pragma once

#include <stdint.h>

#include "tamtypes.h"

#define MODNAME "fwfs"

#ifdef DEBUG
#define DPRINTF(fmt, x...) printf(MODNAME ": " fmt, ##x)
#else
#define DPRINTF(x...)
#endif

int init_fwfs(void);
