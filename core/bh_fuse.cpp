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

#include <string>
#include <stdexcept>
#include <boost/algorithm/string/predicate.hpp> //For iequals()
#include <bh.h>
#include "bh_fuse.h"

using namespace std;

namespace bohrium {

/* The default fuse model */
static const FuseModel default_fuse_model = BROADEST;

/* The current selected fuse model */
static FuseModel selected_fuse_model = NUM_OF_MODELS;


/************************************************************************/
/*************** Specific fuse model implementations ********************/
/************************************************************************/

static bool fuse_broadest(const bh_instruction *a, const bh_instruction *b)
{
    if(bh_opcode_is_system(a->opcode) || bh_opcode_is_system(b->opcode))
        return true;

    const int a_nop = bh_operands(a->opcode);
    for(int i=0; i<a_nop; ++i)
    {
        if(not bh_view_disjoint(&b->operand[0], &a->operand[i])
           && not bh_view_aligned(&b->operand[0], &a->operand[i]))
            return false;
    }
    const int b_nop = bh_operands(b->opcode);
    for(int i=0; i<b_nop; ++i)
    {
        if(not bh_view_disjoint(&a->operand[0], &b->operand[i])
           && not bh_view_aligned(&a->operand[0], &b->operand[i]))
            return false;
    }
    return true;
}

/* Does not allow two sweep operations of the same dimensionality but different
 * sweep dimensions to be put in the same kernel.
 */
static bool fuse_no_xsweep(const bh_instruction *a, const bh_instruction *b)
{
    return (fuse_broadest(a,b) &&
            not (bh_opcode_is_sweep(a->opcode) &&  bh_opcode_is_sweep(b->opcode) &&
                 a->operand[1].ndim == b->operand[1].ndim &&
                 a->constant.value.int64 != b->constant.value.int64));
}

static bool fuse_no_xsweep_scalar_seperate(const bh_instruction *a, const bh_instruction *b)
{
#define __scalar(i) (bh_is_scalar(&(i)->operand[0]) || \
                     (bh_opcode_is_accumulate((i)->opcode) && (i)->operand[0].ndim == 1))
    return (fuse_no_xsweep(a,b) &&
            ((__scalar(a) && __scalar(b)) ||
             (!__scalar(a) && !__scalar(b))));
}

static bool fuse_no_xsweep_scalar_seperate_shape_match(const bh_instruction *a, const bh_instruction *b)
{
    if(bh_opcode_is_system(a->opcode) || bh_opcode_is_system(b->opcode))
        return true;
    const bh_view va = (bh_opcode_is_sweep(a->opcode) ? a->operand[1] : a->operand[0]);
    const bh_view vb = (bh_opcode_is_sweep(b->opcode) ? b->operand[1] : b->operand[0]);
    const bh_intp ndim = MIN(va.ndim,vb.ndim);
    for (bh_intp i =  1; i <= ndim; ++i)
    { // Check that the inner most dimensions match
        if (va.shape[va.ndim-i] != vb.shape[vb.ndim-i])
            return false;
    }
    return fuse_no_xsweep_scalar_seperate(a, b);
}

static bool fuse_same_shape(const bh_instruction *a, const bh_instruction *b)
{
    if(bh_opcode_is_system(a->opcode) || bh_opcode_is_system(b->opcode))
        return true;

    if(!bh_opcode_is_elementwise(a->opcode) || !bh_opcode_is_elementwise(b->opcode))
        return false;

    const int a_nop = bh_operands(a->opcode);
    const int b_nop = bh_operands(b->opcode);
    const bh_intp *shape = a->operand[0].shape;
    const bh_intp ndim = a->operand[0].ndim;
    for(int i=1; i<a_nop; ++i)
    {
        if(bh_is_constant(&a->operand[i]))
            continue;
        if(ndim != a->operand[i].ndim)
            return false;
        for(bh_intp j=0; j<ndim; ++j)
        {
            if(a->operand[i].shape[j] != shape[j])
                return false;
        }
    }
    for(int i=0; i<b_nop; ++i)
    {
        if(bh_is_constant(&b->operand[i]))
            continue;
        if(ndim != b->operand[i].ndim)
            return false;
        for(bh_intp j=0; j<ndim; ++j)
        {
            if(b->operand[i].shape[j] != shape[j])
                return false;
        }
    }
    return fuse_broadest(a, b);
}

static bool is_scalar(const bh_view* view)
{
    return ((view->ndim == 1) and (view->shape[0]==1));
}

static bool fuse_same_shape_stream_creduce(const bh_instruction *a, const bh_instruction *b)
{
    if(bh_opcode_is_system(a->opcode) || bh_opcode_is_system(b->opcode))
        return true;

    if((a->opcode != BH_RANGE and a->opcode != BH_RANDOM \
        and not bh_opcode_is_elementwise(a->opcode)      \
        and not bh_opcode_is_reduction(a->opcode))
        or                                               \
       (b->opcode != BH_RANGE and b->opcode != BH_RANDOM \
        and not bh_opcode_is_elementwise(b->opcode)      \
        and not bh_opcode_is_reduction(b->opcode))) {
        return false;
    }

    //  Check that the output of instruction "a" has the shape
    //  shape as all other operands.
    const int a_nop = bh_operands(a->opcode);
    const int b_nop = bh_operands(b->opcode);
    // a is reduction, b is reduction
    if (bh_opcode_is_reduction(a->opcode) and bh_opcode_is_reduction(b->opcode)) {
        return false;
    // a is NOT reduction, b is reduction
    } else if (not bh_opcode_is_reduction(a->opcode) and bh_opcode_is_reduction(b->opcode)) {
        const bh_intp *red_shape = b->operand[1].shape;
        const bh_intp red_ndim   = b->operand[1].ndim;

        // check that a does not depend on reduce-result of b
        for(int oidx=0; oidx<a_nop; ++oidx) {
            if(bh_is_constant(&a->operand[oidx])) {
                continue;
            }
            if (a->operand[oidx].base == b->operand[0].base) {
                return false;
            }
        }

        for(int oidx=0; oidx<a_nop; ++oidx) {
            if(bh_is_constant(&a->operand[oidx])) {
                continue;
            }
            if(red_ndim != a->operand[oidx].ndim) {
                return false;
            }
            for(bh_intp dim=0; dim<red_ndim; ++dim) {
                if(a->operand[oidx].shape[dim] != red_shape[dim]) {
                    return false;
                }
            }
        }
    // a is reduction, b is NOT reduction
    } else if (bh_opcode_is_reduction(a->opcode) and not bh_opcode_is_reduction(b->opcode)) {
        const bh_intp *red_shape = a->operand[1].shape;
        const bh_intp red_ndim   = a->operand[1].ndim;

        // check that b does not depend on reduce-result of a
        for(int oidx=0; oidx<b_nop; ++oidx) {
            if(bh_is_constant(&b->operand[oidx])) {
                continue;
            }
            if (b->operand[oidx].base == a->operand[0].base) {
                return false;
            }
        }

        for(int oidx=0; oidx<b_nop; ++oidx) {
            if(bh_is_constant(&b->operand[oidx])) {
                continue;
            }
            if(red_ndim != b->operand[oidx].ndim) {
                return false;
            }
            for(bh_intp dim=0; dim<red_ndim; ++dim) {
                if(b->operand[oidx].shape[dim] != red_shape[dim]) {
                    return false;
                }
            }
        }
    // everything else...
    } else {
        const bh_intp *shape = a->operand[0].shape;
        const bh_intp ndim = a->operand[0].ndim;

        if (not is_scalar(&a->operand[0])) {
            for(int i=0; i<b_nop; ++i) {
                if (bh_is_constant(&b->operand[i]))
                    continue;
                if (ndim != b->operand[i].ndim) {
                    return false;
                }
                for (bh_intp j=0; j<ndim; ++j) {
                    if(b->operand[i].shape[j] != shape[j]) {
                        return false;
                    }
                }
            }
        }
    }
    
    return fuse_broadest(a, b);
}

static bool fuse_same_shape_stream_creduce_preduce_once(const bh_instruction *a, const bh_instruction *b)
{
    // Accept system opcodes
    if (bh_opcode_is_system(a->opcode) || bh_opcode_is_system(b->opcode)) {
        return true;
    }

    // Reject opcodes that aren't: ewise, reduction, range, random.
    if ((a->opcode != BH_RANGE and \
        a->opcode != BH_RANDOM and \
        not bh_opcode_is_elementwise(a->opcode) and \
        not bh_opcode_is_reduction(a->opcode)) \
        or \
       (b->opcode != BH_RANGE and \
        b->opcode != BH_RANDOM \
        and not bh_opcode_is_elementwise(b->opcode) \
        and not bh_opcode_is_reduction(b->opcode))) {
        return false;
    }

    const bool a_is_reduction = bh_opcode_is_reduction(a->opcode);
    const bool b_is_reduction = bh_opcode_is_reduction(b->opcode);

    // a is reduction, b is reduction
    if (a_is_reduction and b_is_reduction) {
        // TODO: Add support for multiple reductions as long as they share input-shape
        //       This will require expansion of the CAPE codegen as it currently
        //       splits the instructions into before/after reduction. This should
        //       then have proper handling of reduction in/out.
        return false;
    // a is NOT reduction, b is reduction
    } else if (a_is_reduction or b_is_reduction) {

        const bh_instruction& reduction = a_is_reduction ? *a : *b;
        const bh_instruction& other = a_is_reduction ? *b : *a;
        const int other_nop = bh_operands(other.opcode);

        bool depends_on_reduce_result = false;  // Check that 'other' depends on 'reduction'
        for (int oidx=0; oidx<other_nop; ++oidx) {
            if (bh_is_constant(&other.operand[oidx])) {
                continue;
            }
            if (other.operand[oidx].base == reduction.operand[0].base) {
                depends_on_reduce_result = true;
                break;
            }
        }

        //  Check shape of 'other', it must match:
        //  * 'reduction' output when it depends on output.
        //  * 'reduction' input when it does not
        const bh_intp* reduction_shape = depends_on_reduce_result ? reduction.operand[0].shape : reduction.operand[1].shape;
        const bh_intp reduction_ndim = depends_on_reduce_result ? reduction.operand[0].ndim : reduction.operand[1].ndim;

        // Since non-reduction operands has same-shape of operands
        // then we only need to compare with one of them.
        // We use the output operand since it is always defined.
        if (reduction_ndim != other.operand[0].ndim) {
            printf("a-or-b-red: Rejecting because of ndim.\n");
            return false;
        }
        for (bh_intp dim=0; dim<reduction_ndim; ++dim) {
            if (reduction_shape[dim] != other.operand[0].shape[dim]) {
                printf("a-or-b-red: Rejecting because of shape.\n");
                return false;
            }
        }

    // everything else 
    }  else {

        const int b_nop = bh_operands(b->opcode);

        const bh_intp *shape = a->operand[0].shape;
        const bh_intp ndim = a->operand[0].ndim;

        if (not is_scalar(&a->operand[0])) {
            for (int i=0; i<b_nop; ++i) {
                if (bh_is_constant(&b->operand[i]))
                    continue;
                if (ndim != b->operand[i].ndim) {
                    printf("else: Rejecting because of ndim.");
                    return false;
                }
                for (bh_intp j=0; j<ndim; ++j) {
                    if (b->operand[i].shape[j] != shape[j]) {
                        printf("else: Rejecting because of shape.");
                        return false;
                    }
                }
            }
        }
    }
    
    return fuse_broadest(a, b);
}

/************************************************************************/
/*************** The public interface implementation ********************/
/************************************************************************/

/* Get the selected fuse model by reading the environment
 * variable 'BH_FUSE_MODEL' */
FuseModel fuse_get_selected_model()
{
    using namespace boost;

    if(selected_fuse_model != NUM_OF_MODELS)
        return selected_fuse_model;

    string default_model;
    fuse_model_text(default_fuse_model, default_model);

    //Check enviroment variable
    const char *env = getenv("BH_FUSE_MODEL");
    if(env != NULL)
    {
        string e(env);
        //Iterate through the 'FuseModel' enum and find the enum that matches
        //the enviroment variable string 'e'
        for(FuseModel m = BROADEST; m < NUM_OF_MODELS; m = FuseModel(m + 1))
        {
            string model;
            fuse_model_text(m, model);
            if(iequals(e, model))
            {
//                cout << "[FUSE] info: selected fuse model: '" << model << "'" << endl;
                return m;
            }
        }
        cerr << "[FUSE] WARNING: unknown fuse model: '" << e;
        cerr << "', using the default model '" << default_model << "' instead" << endl;
        setenv("BH_FUSE_MODEL", default_model.c_str(), 1);
    }
//    cout << "[FUSE] info: selected fuse model: '" << default_model << "'" << endl;
    return default_fuse_model;
}

/* Writes the name of the 'fuse_model' to the 'output' string
 *
 * @fuse_model  The fuse model
 * @output      The output string
 */
void fuse_model_text(FuseModel fuse_model, string &output)
{
    switch(fuse_model)
    {
    case BROADEST:
        output = "broadest";
        break;
    case NO_XSWEEP:
        output = "no_xsweep";
        break;
    case NO_XSWEEP_SCALAR_SEPERATE:
        output = "no_xsweep_scalar_seperate";
        break;
    case NO_XSWEEP_SCALAR_SEPERATE_SHAPE_MATCH:
        output = "no_xsweep_scalar_seperate_shape_match";
        break;
    case SAME_SHAPE:
        output = "same_shape";
        break;
    case SAME_SHAPE_STREAM_CREDUCE:
        output = "same_shape_stream_creduce";
        break;
    case SAME_SHAPE_STREAM_CREDUCE_PREDUCE_ONCE:
        output = "same_shape_stream_creduce_preduce_once";
        break;
    default:
        output = "unknown";
    }
}

/* Determines whether it is legal to fuse two instructions into one
 * kernel using the 'selected_fuse_model'.
 *
 * @a The first instruction
 * @b The second instruction
 * @return The boolean answer
 */
bool check_fusible(const bh_instruction *a, const bh_instruction *b)
{
    switch(selected_fuse_model)
    {
    case NUM_OF_MODELS:
        selected_fuse_model = fuse_get_selected_model();
        return check_fusible(a, b);
    case BROADEST:
        return fuse_broadest(a,b);
    case NO_XSWEEP:
        return fuse_no_xsweep(a,b);
    case NO_XSWEEP_SCALAR_SEPERATE:
        return fuse_no_xsweep_scalar_seperate(a,b);
    case NO_XSWEEP_SCALAR_SEPERATE_SHAPE_MATCH:
        return fuse_no_xsweep_scalar_seperate_shape_match(a,b);
    case SAME_SHAPE:
        return fuse_same_shape(a,b);
    case SAME_SHAPE_STREAM_CREDUCE:
        return fuse_same_shape_stream_creduce(a,b);
    case SAME_SHAPE_STREAM_CREDUCE_PREDUCE_ONCE:
        return fuse_same_shape_stream_creduce_preduce_once(a,b);
    default:
        throw runtime_error("No fuse module is selected!");
    }
}

} //namespace bohrium
