/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#include <stdio.h>
#include <stdarg.h>

#include <cassert>
#include <stdint.h>

#include "spect_iss_lib.h"

int main()
{
    spect_iss_init(2);

    spect_iss_load_s_file(DPI_TEST_FW);
    spect_iss_cmd_start(std::cout);
    spect_iss_cmd_step(std::cout, 10);
    spect_iss_cmd_run(std::cout);

    spect_iss_dump_data_ram_out_hex(std::string("dram_out.hex"));
    spect_iss_dump_emem_out_hex(std::string("emem_out.hex"));
    spect_iss_dump_key_mem_out_hex(std::string("kmem_out.hex"));

    spect_iss_exit();
}
