#include "libcivaMSFS24.h"

#include <stdio.h>

extern "C" MSFS_CALLBACK void module_init(void)
{
	// This is called when the module is initialized
}

extern "C" MSFS_CALLBACK void Update_StandAlone(float dTime)
{
	// This is called each frame
}

extern "C" MSFS_CALLBACK void module_deinit(void)
{
	// This is called when the module is deinitialized
}
