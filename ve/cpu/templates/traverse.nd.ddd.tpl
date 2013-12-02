{{#license}}
/*
This file is part of Bohrium and copyright (c) 2012 the Bohrium
team <http://www.bh107.org>.

Bohrium is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as 
published by the Free Software Foundation, either version 3 
of the License, or (at your option) any later version.

Bohrium is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the 
GNU Lesser General Public License along with Bohrium. 

If not, see <http://www.gnu.org/licenses/>.
*/
{{/license}}
{{#include}}
#include "assert.h"
#include "stdarg.h"
#include "string.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "complex.h"
#include "math.h"
#include <Random123/threefry.h>

#include "omp.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEG_CIR 360.0
#define DEG_RAD (M_PI / (DEG_CIR / 2.0))
#define RAD_DEG ((DEG_CIR / 2.0) / M_PI)

#ifndef CPU_MISC
#define CPU_MAXDIM 16
#endif
{{/include}}

void {{SYMBOL}}(int tool, ...)
{
    va_list list;               // Unpack arguments
    va_start(list, tool);

    {{TYPE_A0}} *a0_first = va_arg(list, {{TYPE_A0}}*);
    int64_t  a0_start   = va_arg(list, int64_t);
    int64_t *a0_stride  = va_arg(list, int64_t*);
    assert(a0_first != NULL);

    {{#a1_scalar}}
    {{TYPE_A1}} *a1_first   = va_arg(list, {{TYPE_A1}}*);
    {{/a1_scalar}}  
 
    {{#a1_dense}}
    {{TYPE_A1}} *a1_first   = va_arg(list, {{TYPE_A1}}*);
    int64_t  a1_start   = va_arg(list, int64_t);
    int64_t *a1_stride  = va_arg(list, int64_t*);
    assert(a1_first != NULL);
    {{/a1_dense}}

    {{#a2_scalar}}
    {{TYPE_A2}} *a2_first   = va_arg(list, {{TYPE_A2}}*);
    {{/a2_scalar}}

    {{#a2_dense}}
    {{TYPE_A2}} *a2_first   = va_arg(list, {{TYPE_A2}}*);
    int64_t  a2_start   = va_arg(list, int64_t);
    int64_t *a2_stride  = va_arg(list, int64_t*);
    assert(a2_first != NULL);
    {{/a2_dense}}
    
    int64_t *shape      = va_arg(list, int64_t*);
    int64_t ndim        = va_arg(list, int64_t);
    va_end(list);

    int64_t nelements = shape[0];
    for(int64_t i=1; i<ndim; ++i) {
        nelements *= shape[i];
    }

    a0_first += a0_start;
    {{#a1_dense}}
    a1_first += a1_start;
    {{/a1_dense}}
    {{#a2_dense}}
    a2_first += a2_start;
    {{/a2_dense}}

    int mthreads     = omp_get_max_threads();
    int64_t nworkers = nelements > mthreads ? mthreads : 1;

    #pragma omp parallel num_threads(nworkers)
    {
        int tid      = omp_get_thread_num();    // Work partitioning
        int nthreads = omp_get_num_threads();

        int64_t work = nelements / nthreads;
        int64_t work_offset = work * tid;
        if (tid==nthreads-1) {
            work += nelements % nthreads;
        }
        int64_t work_end = work_offset+work;

        {{TYPE_A0}} *a0_current = a0_first + work_offset;

        {{#a1_scalar}}
        {{TYPE_A1}} *a1_current = a1_first;
        {{/a1_scalar}}
        {{#a1_dense}}
        {{TYPE_A1}} *a1_current = a1_first + work_offset;
        {{/a1_dense}}

        {{#a2_scalar}}
        {{TYPE_A2}} *a2_current = a2_first;
        {{/a2_scalar}}
        {{#a2_dense}}
        {{TYPE_A2}} *a2_current = a2_first + work_offset;
        {{/a2_dense}}

        for (int64_t i = work_offset; i<work_end; ++i) {
            {{OPERATOR}};

            ++a0_current;
            {{#a1_dense}}
            ++a1_current;
            {{/a1_dense}}
            {{#a2_dense}}
            ++a2_current;
            {{/a2_dense}}
        }
    }
}
