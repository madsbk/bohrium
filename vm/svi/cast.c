/*
 * Copyright 2011 Troels Blum <troels@blum.dk>
 *
 * This file is part of cphVB.
 *
 * cphVB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cphVB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cphVB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cast.h"

inline cphvb_int32 svi_to_int32(const void* var, cphvb_type type)
{
    switch(type)
    {
    case CPHVB_INT32:
        return *(cphvb_int32*)var;
    case CPHVB_UINT32:
        return (cphvb_int32)(*(cphvb_uint32*)var);
    case CPHVB_FLOAT32:
        return (cphvb_int32)(*(cphvb_float32*)var);
    default:
        return 0;
    }    
}

bool svi_can_cast_to_int32(cphvb_type type)
{
    switch(type)
    {
    case CPHVB_INT32:
    case CPHVB_UINT32:
    case CPHVB_FLOAT32:
        return TRUE;
    default:
        return FALSE;
    }    
}

inline cphvb_uint32 svi_to_uint32(const void* var, cphvb_type type)
{
    switch(type)
    {
    case CPHVB_INT32:
        return (cphvb_uint32)(*(cphvb_int32*)var);
    case CPHVB_UINT32:
        return *(cphvb_uint32*)var;
    case CPHVB_FLOAT32:
        return (cphvb_uint32)(*(cphvb_float32*)var);
    default:
        return 0;
    }
}

bool svi_can_cast_to_uint32(cphvb_type type)
{
    switch(type)
    {
    case CPHVB_INT32:
    case CPHVB_UINT32:
    case CPHVB_FLOAT32:
        return TRUE;
    default:
        return FALSE;
    }    
}

inline cphvb_float32 svi_to_float32(const void* var, cphvb_type type)
{
    switch(type)
    {
    case CPHVB_INT32:
        return (cphvb_float32)(*(cphvb_int32*)var);
    case CPHVB_UINT32:
        return (cphvb_float32)(*(cphvb_uint32*)var);
    case CPHVB_FLOAT32:
        return *(cphvb_float32*)var;
    default:
        return 0.0;
    }
}

bool svi_can_cast_to_float32(cphvb_type type)
{
    switch(type)
    {
    case CPHVB_INT32:
    case CPHVB_UINT32:
    case CPHVB_FLOAT32:
        return TRUE;
    default:
        return FALSE;
    }    
}
