/*
This file is part of cphVB and copyright (c) 2012 the cphVB team:
http://cphvb.bitbucket.org

cphVB is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as 
published by the Free Software Foundation, either version 3 
of the License, or (at your option) any later version.

cphVB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the 
GNU Lesser General Public License along with cphVB. 

If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <cphvb.h>

#ifdef __cplusplus
extern "C" {
#endif

static cphvb_component* component = NULL;

cphvb_error cphvb_ve_print_init(cphvb_component* _component)
{
    component = _component;
    return CPHVB_SUCCESS;
}

cphvb_error cphvb_ve_print_execute(cphvb_intp instruction_count,
                                   cphvb_instruction instruction_list[])
{
    std::cout << "# ----------------------------- Recieved batch with " << 
        instruction_count << 
        " instructions --------------------------------------- #" << std::endl;
    for (cphvb_intp i = 0; i < instruction_count; ++i)
        cphvb_pprint_instr(instruction_list+i);
    return CPHVB_SUCCESS;
}

cphvb_error cphvb_ve_print_shutdown()
{
    return CPHVB_SUCCESS;
}

cphvb_error cphvb_ve_print_reg_func(char *fun, 
                                  cphvb_intp *id)
{
    return CPHVB_SUCCESS;
}

#ifdef __cplusplus
}
#endif