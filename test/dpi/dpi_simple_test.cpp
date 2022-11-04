/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#include <cassert>
#include <stdint.h>

#include "spect_iss_dpi.h"



int main()
{
    uint32_t rv;

    rv = spect_dpi_init();
    assert(rv == 0 && "DPI Library initialized");

    spect_dpi_set_verbosity(3);

    spect_dpi_reset();

    // TODO: Adjust paths to be raltive to some var
    rv = spect_dpi_compile_program(DPI_TEST_FW, "tmp.hex", 0);
    assert(rv == 0);


    spect_dpi_exit();
}