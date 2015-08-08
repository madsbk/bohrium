#compiler-settings
directiveStartToken= %
#end compiler-settings
%slurp
#ifndef __BH_VE_CPU_TAC
#define __BH_VE_CPU_TAC
//
//  NOTE: This file is autogenerated based on the tac-definition.
//        You should therefore not edit it manually.
//
#include "stdint.h"

// Bohrium custom types, used of representing
// two inputs in one constant... hopefully we can get
// rid of it... at some point...
typedef struct { uint64_t first, second; } pair_LL; 

typedef struct
{
    void* data;         // Pointer to memory allocation supporting the buffer.
    int64_t type;       // Datatype for which the buffer is intended to store.
    int64_t nelem;      // Number of elements of the given datatype for which there is room for in the buffer.
} kp_buffer;            // NOTE: Must be binary compatible with Bohrium for interoperability

typedef enum KP_ETYPE {
    %for $type in $types
    $addw($type['name'])$addsep($type, $types)
    %end for
} KP_ETYPE;             // NOTE: Must be binary compatible with Bohrium for interoperability

typedef enum KP_LAYOUT {
    %for $layout in $layouts
    $addw($layout['name']) = ${layout['id']}$addsep($layout, $layouts)
    %end for
} KP_LAYOUT;            // Bitmasks

typedef struct kp_operand {
    KP_LAYOUT  layout;  // The layout of the data
    void*   const_data; // Pointer to constant
    KP_ETYPE   etype;   // Type of the elements stored
    int64_t start;      // Offset from memory allocation to start of array
    int64_t nelem;      // Number of elements available in the allocation

    int64_t ndim;       // Number of dimensions of the array
    int64_t* shape;     // Shape of the array
    int64_t* stride;    // Stride in each dimension of the array
    kp_buffer* base;    // Pointer to operand base or NULL when layout == SCALAR_CONST.
} kp_operand;           // Meta-data for a block argument

typedef enum KP_OPERATION {
    %for $op in $ops
    $addw($op['name']) = ${op['id']}$addsep($op, $ops)
    %end for
} KP_OPERATION;         // Bitmasks

typedef enum KP_OPERATOR {
    %for $oper in $opers
    $addw($oper['name'],15) = ${oper['id']}$addsep($oper, $opers)
    %end for
} KP_OPERATOR;          // Bitmasks

typedef struct kp_tac {
    KP_OPERATION op;    // Operation
    KP_OPERATOR  oper;  // Operator
    uint32_t  out;      // Output operand
    uint32_t  in1;      // First input operand
    uint32_t  in2;      // Second input operand
    void* ext;
} kp_tac;

typedef struct kp_iterspace {
    KP_LAYOUT layout;   // The dominating layout
    int64_t ndim;       // The dominating rank/dimension of the iteration space
    int64_t* shape;     // Shape of the iteration space
    int64_t nelem;      // The number of elements in the iteration space
} kp_iterspace;

typedef void (*kp_krnl_func)(kp_buffer** buffers, kp_operand ** args, kp_iterspace * iterspace, const int offload_devid);

#define KP_SCALAR_LAYOUT   ( KP_SCALAR | KP_SCALAR_CONST | KP_SCALAR_TEMP )
#define KP_ARRAY_LAYOUT    ( KP_CONTRACTABLE | KP_CONTIGUOUS | KP_CONSECUTIVE | KP_STRIDED | KP_SPARSE )
#define KP_COLLAPSIBLE     ( KP_SCALAR | KP_SCALAR_CONST | KP_CONTRACTABLE | KP_CONTIGUOUS | KP_CONSECUTIVE )
#define KP_DYNALLOC_LAYOUT ( KP_SCALAR | KP_CONTIGUOUS | KP_CONSECUTIVE | KP_STRIDED | KP_SPARSE )

#define KP_EWISE           ( KP_MAP | KP_ZIP | KP_GENERATE )
#define KP_REDUCTION       ( KP_REDUCE_COMPLETE | KP_REDUCE_PARTIAL )
#define KP_ACCUMULATION    ( KP_REDUCE_COMPLETE | KP_REDUCE_PARTIAL | KP_SCAN )
#define KP_ARRAY_OPS       ( KP_MAP | KP_ZIP | KP_GENERATE | KP_REDUCE_COMPLETE | KP_REDUCE_PARTIAL | KP_SCAN | KP_INDEX )
#define KP_NBUILTIN_OPS    %echo $len($ops)-1
#define KP_NBUILTIN_OPERS  %echo $len($opers)-1

#endif
