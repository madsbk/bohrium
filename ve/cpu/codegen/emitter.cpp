#include <sstream>
#include <set>
#include "utils.hpp"
#include "codegen.hpp"

using namespace std;
using namespace kp::core;

namespace kp{
namespace engine{
namespace codegen{

Emitter::Emitter(Plaid& plaid, Block& block) : plaid_(plaid), block_(block), iterspace_(block.iterspace()) {

    for(size_t tac_idx=0; tac_idx<block_.ntacs(); ++tac_idx) {
        kp_tac & tac = block_.tac(tac_idx);
        if (not ((tac.op & (KP_ARRAY_OPS))>0)) {   // Only interested in array ops
            continue;
        }
        tacs_.push_back(&tac);
        switch(tac_noperands(tac)) {
            case 3:
                add_operand(tac.in2);
            case 2:
                add_operand(tac.in1);
            case 1:
                add_operand(tac.out);
            default:
                break;
        }
    }
}

void Emitter::add_operand(uint64_t global_idx)
{
    uint64_t local_idx = block_.global_to_local(global_idx);

    kp_operand & operand = block_.operand(local_idx);
    
    Buffer* buffer = NULL;  // Associate a Buffer instance
    if ((operand.base) && ((operand.layout & KP_DYNALLOC_LAYOUT)>0)) {
        size_t buffer_id = block_.resolve_buffer(operand.base);
        buffer = new Buffer(operand.base, buffer_id);
        buffers_[buffer_id] = *buffer;
    }

    operands_[global_idx] = Operand(
        &operand,
        local_idx,
        buffer
    );
}

string Emitter::oper_neutral_element(KP_OPERATOR oper, KP_ETYPE etype)
{
    switch(oper) {
        case KP_ADD:               return "0";
        case KP_MULTIPLY:          return "1";
        case KP_MAXIMUM:
            switch(etype) {
                case KP_BOOL:      return "0";
                case KP_INT8:      return "INT8_MIN";
                case KP_INT16:     return "INT16_MIN";
                case KP_INT32:     return "INT32_MIN";
                case KP_INT64:     return "INT64_MIN";
                case KP_UINT8:     return "UINT8_MIN";
                case KP_UINT16:    return "UINT16_MIN";
                case KP_UINT32:    return "UINT32_MIN";
                case KP_UINT64:    return "UINT64_MIN";
                case KP_FLOAT32:   return "FLT_MIN";
                case KP_FLOAT64:   return "DBL_MIN";
                default:        return "UNKNOWN_NEUTRAL_FOR_MAXIMUM_OF_GIVEN_TYPE";
            }
        case KP_MINIMUM:
            switch(etype) {
                case KP_BOOL:      return "1";
                case KP_INT8:      return "INT8_MAX";
                case KP_INT16:     return "INT16_MAX";
                case KP_INT32:     return "INT32_MAX";
                case KP_INT64:     return "INT64_MAX";
                case KP_UINT8:     return "UINT8_MAX";
                case KP_UINT16:    return "UINT16_MAX";
                case KP_UINT32:    return "UINT32_MAX";
                case KP_UINT64:    return "UINT64_MAX";
                case KP_FLOAT32:   return "FLT_MAX";
                case KP_FLOAT64:   return "DBL_MAX";
                default:        return "UNKNOWN_NEUTRAL_FOR_MINIMUM_OF_GIVEN_TYPE";
            }
        case KP_LOGICAL_AND:       return "1";
        case KP_LOGICAL_OR:        return "0";
        case KP_LOGICAL_XOR:       return "0";
        case KP_BITWISE_AND:
            switch(etype) {
                case KP_BOOL:      return "1";
                case KP_INT8:      return "-1";
                case KP_INT16:     return "-1";
                case KP_INT32:     return "-1";
                case KP_INT64:     return "-1";
                case KP_UINT8:     return "UINT8_MAX";
                case KP_UINT16:    return "UINT16_MAX";
                case KP_UINT32:    return "UINT32_MAX";
                case KP_UINT64:    return "UINT64_MAX";
                default:        return "UNKNOWN_NEUTRAL_FOR_BITWISE_AND_OF_GIVEN_TYPE";
            }
        case KP_BITWISE_OR:        return "0";
        case KP_BITWISE_XOR:       return "0";
        default:                return "UNKNOWN_NEUTRAL_FOR_OPERATOR";
    }
}

string Emitter::oper_description(kp_tac tac)
{
    stringstream ss;
    ss << operator_text(tac.oper) << " (";
    switch(tac_noperands(tac)) {
        case 3:
            ss << layout_text(operand_glb(tac.out).meta().layout);
            ss << ", ";
            ss << layout_text(operand_glb(tac.in1).meta().layout);
            ss << ", ";
            ss << layout_text(operand_glb(tac.in2).meta().layout);
            break;
        case 2:
            ss << layout_text(operand_glb(tac.out).meta().layout);
            ss << ", ";
            ss << layout_text(operand_glb(tac.in1).meta().layout);
            break;
        case 1:
            ss << layout_text(operand_glb(tac.out).meta().layout);
            break;
        default:
            break;
    }
    ss << ")";
    return ss.str();
}

string Emitter::oper(KP_OPERATOR oper, KP_ETYPE etype, string in1, string in2)
{
    switch(oper) {
        case KP_ABSOLUTE:
            switch(etype) {
                case KP_COMPLEX128:    return _cabs(in1);
                case KP_COMPLEX64:     return _cabsf(in1);
                default:            return _abs(in1);
            }
        case KP_ADD:                   return _add(in1, in2);
        case KP_ARCCOS:
            switch(etype) {
                case KP_COMPLEX128:    return _cacos(in1);
                case KP_COMPLEX64:     return _cacosf(in1);
                default:            return _acos(in1);
            }
        case KP_ARCCOSH:
            switch(etype) {
                case KP_COMPLEX128:    return _cacosh(in1);
                case KP_COMPLEX64:     return _cacosf(in1);
                default:            return _acosh(in1);
            }
        case KP_ARCSIN:
            switch(etype) {
                case KP_COMPLEX128:    return _casin(in1);
                case KP_COMPLEX64:     return _casinf(in1);
                default:            return _asin(in1);
            }
        case KP_ARCSINH:
            switch(etype) {
                case KP_COMPLEX128:    return _casinh(in1);
                case KP_COMPLEX64:     return _casinhf(in1);
                default:            return _asinh(in1);
            }
        case KP_ARCTAN:
            switch(etype) {
                case KP_COMPLEX128:    return _catan(in1);
                case KP_COMPLEX64:     return _catanf(in1);
                default:            return _atan(in1);
            }
        case KP_ARCTAN2:               return _atan2(in1, in2);
        case KP_ARCTANH:
            switch(etype) {
                case KP_COMPLEX128:    return _catanh(in1);
                case KP_COMPLEX64:     return _catanhf(in1);
                default:            return _atanh(in1);
            }
        case KP_BITWISE_AND:           return _bitw_and(in1, in2);
        case KP_BITWISE_OR:            return _bitw_or(in1, in2);
        case KP_BITWISE_XOR:           return _bitw_xor(in1, in2);
        case KP_CEIL:                  return _ceil(in1);
        case KP_COS:
            switch(etype) {
                case KP_COMPLEX128:    return _ccos(in1);
                case KP_COMPLEX64:     return _ccosf(in1);
                default:            return _cos(in1);
            }
        case KP_COSH:
            switch(etype) {
                case KP_COMPLEX128:    return _ccosh(in1);
                case KP_COMPLEX64:     return _ccoshf(in1);
                default:            return _cosh(in1);
            }
        case KP_DISCARD:               break;  // TODO: Raise exception
        case KP_DIVIDE:                return _div(in1, in2);
        case KP_EQUAL:                 return _eq(in1, in2);
        case KP_EXP:
            switch(etype) {
                case KP_COMPLEX128:    return _cexp(in1);
                case KP_COMPLEX64:     return _cexpf(in1);
                default:            return _exp(in1);
            }
        case KP_EXP2:
            switch(etype) {
                case KP_COMPLEX128:    return _cexp2(in1);
                case KP_COMPLEX64:     return _cexp2f(in1);
                default:            return _exp2(in1);
            }
        case KP_EXPM1:                 return _expm1(in1);
        case KP_EXTENSION_OPERATOR:    break;  // TODO: Raise exception
        case KP_FLOOD:                 break;  // TODO: Raise exception
        case KP_FLOOR:                 return _floor(in1);
        case KP_FREE:                  break;  // TODO: Raise exception
        case KP_GREATER:               return _gt(in1, in2);
        case KP_GREATER_EQUAL:         return _gteq(in1, in2);
        case KP_IDENTITY:              return in1;
        case KP_IMAG:
            switch(etype) {
                case KP_FLOAT32:       return _cimagf(in1);
                default:            return _cimag(in1);
            }
        case KP_INVERT:
            switch(etype) {
                case KP_BOOL:          return _invertb(in1);
                default:            return _invert(in1);
            }
        case KP_ISINF:                 return _isinf(in1);
        case KP_ISNAN:                 return _isnan(in1);
        case KP_LEFT_SHIFT:            return _bitw_leftshift(in1, in2);
        case KP_LESS:                  return _lt(in1, in2);
        case KP_LESS_EQUAL:            return _lteq(in1, in2);
        case KP_LOG:
            switch(etype) {
                case KP_COMPLEX128:    return _clog(in1);
                case KP_COMPLEX64:     return _clogf(in1);
                default:            return _log(in1);
            }
        case KP_LOG10:
            switch(etype) {
                case KP_COMPLEX128:    return _clog10(in1);
                case KP_COMPLEX64:     return _clog10f(in1);
                default:            return _log10(in1);
            }
        case KP_LOG1P:                 return _log1p(in1);
        case KP_LOG2:                  return _log2(in1);
        case KP_LOGICAL_AND:           return _logic_and(in1, in2);
        case KP_LOGICAL_NOT:           return _logic_not(in1);
        case KP_LOGICAL_OR:            return _logic_or(in1, in2);
        case KP_LOGICAL_XOR:           return _logic_xor(in1, in2);
        case KP_MAXIMUM:               return _max(in1, in2);
        case KP_MINIMUM:               return _min(in1, in2);
        case KP_MOD:                   return _mod(in1, in2);
        case KP_MULTIPLY:              return _mul(in1, in2);
        case KP_NONE:                  break;  // TODO: Raise exception
        case KP_NOT_EQUAL:             return _neq(in1, in2);
        case KP_POWER:
            switch(etype) {
                case KP_COMPLEX128:    return _cpow(in1, in2);
                case KP_COMPLEX64:     return _cpowf(in1, in2);
                default:            return _pow(in1, in2);
            }
        case KP_RANDOM:                return _random("idx0", in1, in2);
        //case KP_RANGE:                 return _range();
        case KP_RANGE:                 return "idx0";
        case KP_REAL:
            switch(etype) {
                case KP_FLOAT32:       return _crealf(in1);
                default:            return _creal(in1);
            }
        case KP_RIGHT_SHIFT:           return _bitw_rightshift(in1, in2);
        case KP_RINT:                  return _rint(in1);
        case KP_SIN:
            switch(etype) {
                case KP_COMPLEX128:    return _csin(in1);
                case KP_COMPLEX64:     return _csinf(in1);
                default:            return _sin(in1);
            }
        case KP_SIGN:
            switch(etype) {
                case KP_COMPLEX128:    return _div(in1, _parens(_add(_cabs(in1), _parens(_eq(in1, "0")))));
                case KP_COMPLEX64:     return _div(in1, _parens(_add(_cabsf(in1), _parens(_eq(in1, "0")))));
                default:            return _sub(
                                            _parens(_gt(in1, "0")),
                                            _parens(_lt(in1, "0"))
                                           );
            }

        case KP_SINH:
            switch(etype) {
                case KP_COMPLEX128:    return _csinh(in1);
                case KP_COMPLEX64:     return _csinhf(in1);
                default:            return _sinh(in1);
            }
        case KP_SQRT:
            switch(etype) {
                case KP_COMPLEX128:    return _csqrt(in1);
                case KP_COMPLEX64:     return _csqrtf(in1);
                default:            return _sqrt(in1);
            }
        case KP_SUBTRACT:              return _sub(in1, in2);
        case KP_SYNC:                  break;  // TODO: Raise exception
        case KP_TAN:
            switch(etype) {
                case KP_COMPLEX128:    return _ctan(in1);
                case KP_COMPLEX64:     return _ctanf(in1);
                default:            return _tan(in1);
            }
        case KP_TANH:
            switch(etype) {
                case KP_COMPLEX128:    return _ctanh(in1);
                case KP_COMPLEX64:     return _ctanhf(in1);
                default:            return _tanh(in1);
            }
        case KP_TRUNC:                 return _trunc(in1);
        default:                    return "NON_IMPLEMENTED_OPERATOR";
    }
    return "NO NO< NO NO NO NO NONO NO NO NO NOTHERES NO LIMITS";
}

string Emitter::synced_oper(KP_OPERATOR operation, KP_ETYPE etype, string out, string in1, string in2)
{
    stringstream ss;
    switch(operation) {
        case KP_MAXIMUM:
        case KP_MINIMUM:
        case KP_LOGICAL_AND:
        case KP_LOGICAL_OR:
        case KP_LOGICAL_XOR:
            ss << _omp_critical(_assign(out, oper(operation, etype, in1, in2)), "accusync");
            break;
        default:
            switch(etype) {
                case KP_COMPLEX64:
                case KP_COMPLEX128:
                    ss << _omp_critical(_assign(out, oper(operation, etype, in1, in2)), "accusync");
                    break;
                default:
                    ss << _omp_atomic(_assign(out, oper(operation, etype, in1, in2)));
                    break;
            }
            break;
    }
    return ss.str();
}

kp::core::Block& Emitter::block(void)
{
    return block_;
}

string Emitter::text(void)
{
    stringstream ss;
    ss << block_.text() << endl;
    return ss.str();
}

string Emitter::buffers(void)
{
    return "buffers";
}

string Emitter::args(void)
{
    return "args";
}

Iterspace& Emitter::iterspace(void)
{
    return iterspace_;
}

uint64_t Emitter::base_refcount(uint64_t gidx)
{
    return block_.buffer_refcount(operand_glb(gidx).meta().base);
}

uint64_t Emitter::noperands(void)
{
    return tacs_.size();
}

Operand& Emitter::operand_glb(uint64_t gidx)
{
    return operands_[gidx];
}

Operand& Emitter::operand_lcl(uint64_t lidx)
{
    return operands_[block_.local_to_global(lidx)];
}

kernel_operand_iter Emitter::operands_begin(void)
{
    return operands_.begin();
}

kernel_operand_iter Emitter::operands_end(void)
{
    return operands_.end();
}

kernel_buffer_iter Emitter::buffers_begin(void)
{
    return buffers_.begin();
}

kernel_buffer_iter Emitter::buffers_end(void)
{
    return buffers_.end();
}

uint32_t Emitter::omask(void)
{
    return block_.omask();
}

uint64_t Emitter::ntacs(void)
{
    return tacs_.size();
}

kp_tac & Emitter::tac(uint64_t tidx)
{
    return *tacs_[tidx];
}

kernel_tac_iter Emitter::tacs_begin(void)
{
    return tacs_.begin();
}

kernel_tac_iter Emitter::tacs_end(void)
{
    return tacs_.end();
}

string Emitter::axis_access(int64_t glb_idx, const int64_t axis)
{
    stringstream ss;

    Operand& operand = operand_glb(glb_idx);

    switch(operand.meta().layout) {
    case KP_CONTIGUOUS:
    case KP_STRIDED:
    case KP_CONSECUTIVE:
        if ((axis == operand.meta().ndim-1) && (operand.meta().stride[axis] == 0)) {
            ss << _index(operand.walker(), "0");
        } else if ((axis == operand.meta().ndim-1) && (operand.meta().stride[axis] == 1)) {
            ss << _index(operand.walker(), "idx"+to_string(axis));
        } else {
            ss << _index(
                operand.walker(),
                _mul(
                    "idx"+to_string(axis),
                    operand.strides()+"_d"+to_string(axis)
                )
            );
        }
        break;

    default:
        ss << operand_glb(glb_idx).name();
        break;
    }

    return ss.str();
}

string Emitter::unpack_iterspace(void)
{
    stringstream ss;
    ss << _declare_init(
        "KP_LAYOUT",
        iterspace().layout(),
        _access_ptr(iterspace().name(), "layout")
    )
    << _end();
    ss << _declare_init(
        _const(_int64()),
        iterspace().ndim(),
        _access_ptr(iterspace().name(), "ndim")
    )
    << _end();
    ss << _declare_init(
        _ptr(_int64()),
        iterspace().shape(),
        _access_ptr(iterspace().name(), "shape")
    )
    << _end();
    for(int64_t dim=0; dim<iterspace().meta().ndim; ++dim) {
        ss << _declare_init(
            _const(_int64()),
            iterspace().shape()+"_d"+to_string(dim),
            _index(_access_ptr(iterspace().name(), "shape"), to_string(dim))
        ) << _end();
    }
    ss << _declare_init(
        _const(_int64()),
        iterspace().nelem(),
        _access_ptr(iterspace().name(), "nelem")
    )
    << _end();

    return ss.str();
}

string Emitter::unpack_buffers(void)
{
    stringstream ss;
    for(int64_t bid=0; bid< block_.nbuffers(); ++bid) {
        Buffer buffer(&block_.buffer(bid), bid);
        ss << endl;
        ss << "// Buffer " << buffer.name() << endl;
        ss << _declare_init(
            _ptr(buffer.etype()),
            buffer.data(),
            _access_ptr(
                _index("buffers", bid),
                "data"
            )
        ) << _end();
        ss << _declare_init(
            _int64(),
            buffer.nelem(),
            _access_ptr(
                _index("buffers", bid),
                "nelem"
            )
        ) << _end();
        ss << _assert_not_null(buffer.data()) << _end();
    }
    return ss.str();
}

string Emitter::unpack_arguments(void)
{
    stringstream ss;
    for(kernel_operand_iter oit=operands_begin(); oit != operands_end(); ++oit) {
        Operand& operand = oit->second;
        uint64_t id = operand.local_id();
        ss << endl;
        ss << "// Argument " << operand.name() << " [" << operand.layout() << "]" << endl;
        switch(operand.meta().layout) {
            case KP_STRIDED:
            case KP_CONSECUTIVE:
            case KP_CONTIGUOUS:
            case KP_SCALAR:
                ss
                << _declare_init(
                    _const(_int64()),
                    operand.start(),
                    _access_ptr(_index(args(), id), "start")
                )
                << _end();
                ss
                << _declare_init(
                    _const(_int64()),
                    operand.nelem(),
                    _access_ptr(_index(args(), id), "nelem")
                )
                << _end();
                ss
                << _declare_init(
                    _ptr_const(_int64()),
                    operand.strides(),
                    _access_ptr(_index(args(), id), "stride")
                )
                << _end();
                for(int64_t dim=0; dim<operand.meta().ndim; ++dim) {
                    ss
                    << _declare_init(
                        _const(_int64()),
                        operand.strides()+"_d"+to_string(dim),
                        _index(_access_ptr(_index(args(), id), "stride"), to_string(dim))
                    )
                    << _end();               
                }
                break;

            case KP_SCALAR_CONST:
                ss
                << _declare_init(
                    _const(operand.etype()),
                    operand.walker(),
                    _deref(_cast(
                        _ptr(operand.etype()),
                        _access_ptr(_index(args(), id), "const_data")
                    ))
                )
                << _end();
                break;

            case KP_SCALAR_TEMP:
            case KP_CONTRACTABLE:  // Data pointer is never used.
                ss << _comment("No unpacking needed.") << endl;
                break;

            case KP_SPARSE:
                ss << _beef("Unpacking not implemented for KP_LAYOUT!");
                break;
        }
    }
    return ss.str();
}

string Emitter::scalar_walk()
{
    return "";
}

string Emitter::collapsed_walk()
{
    return "";
}

string Emitter::nested_walk()
{
    return "";
}

void Emitter::declare_init_opds(Skeleton& skel, string sect)
{
    const uint32_t ispace_ndim = iterspace().meta().ndim;
    int64_t ispace_axis = iterspace().meta().axis;

    for (kernel_operand_iter oit = operands_begin();
        oit != operands_end();
        ++oit) {

        Operand& operand = oit->second;
        bool restrictable = base_refcount(oit->first) == 1;
        switch(operand.meta().layout) {
            case KP_SCALAR_CONST:
                break;

            case KP_SCALAR:
                skel[sect] += _declare_init(
                    operand.etype(),
                    operand.walker(),
                    _deref(_add(operand.buffer_data(), operand.start()))
                );
                break;

            case KP_SCALAR_TEMP:
            case KP_CONTRACTABLE:
                skel[sect] += _declare(
                    operand.etype(),
                    operand.walker()
                );
                break;

            case KP_CONTIGUOUS:
            case KP_CONSECUTIVE:
            case KP_STRIDED:
                {
                    stringstream offset;    // DO: offset += idx*stride for every non-axis dim
                    int64_t opd_ndim = operand.meta().ndim;
                    int64_t opd_axis = 0;
                    for (int64_t axis=0; axis<ispace_ndim; ++axis) {
                        if (axis==ispace_axis) {
                            if (opd_ndim==ispace_ndim) {
                                ++opd_axis;
                            }
                            continue;
                        }
                        offset << " + " << _mul(
                            operand.strides()+"_d"+to_string(opd_axis),
                            "idx"+to_string(axis)
                        );
                        ++opd_axis;
                    }
                                            // Declare array pointers
                    if (restrictable) {     // Annotate *restrict*
                        skel[sect] += _declare_init(
                            _restrict(_ptr(operand.etype())),
                            operand.walker(),
                            _add(operand.buffer_data(), operand.start())+offset.str()
                        );
                    } else {
                        skel[sect] += _declare_init(
                            _ptr(operand.etype()),
                            operand.walker(),
                            _add(operand.buffer_data(), operand.start())+offset.str()
                        );
                    }
                }
                break;

            case KP_SPARSE:
                skel[sect] += _beef("Unimplemented KP_LAYOUT.");
                break;
        }
        skel[sect] += _end(operand.layout());
    }
}

void Emitter::emit_operations(Skeleton& skel)
{
    int64_t ispace_axis = iterspace().meta().axis;

    for(kernel_tac_iter tit=tacs_begin();
        tit!=tacs_end();
        ++tit) {
        kp_tac& tac = **tit;
        KP_ETYPE etype = (KP_ABSOLUTE == tac.oper) ?  operand_glb(tac.in1).meta().etype : operand_glb(tac.out).meta().etype;

        string out = "ERROR_OUT", in1 = "ERROR_IN1", in2 = "ERROR_IN2";
        switch(tac.op) {
        case KP_ZIP:
            skel["BODY"] += _assign(
                axis_access(tac.out, ispace_axis),
                oper(
                    tac.oper,
                    etype,
                    axis_access(tac.in1, ispace_axis),
                    axis_access(tac.in2, ispace_axis)
                )
            );
            skel["BODY"] += _end(oper_description(tac));
            break;

        case KP_MAP:
            skel["BODY"] += _assign(
                axis_access(tac.out, ispace_axis),
                oper(
                    tac.oper,
                    etype,
                    axis_access(tac.in1, ispace_axis),
                    ""
                )
            );
            skel["BODY"] += _end(oper_description(tac));
            break;

        case KP_GENERATE:
            switch(tac.oper) {
            case KP_RANDOM: // Random has inputs
                skel["BODY"] += _assign(
                    axis_access(tac.out, ispace_axis),
                    oper(
                        tac.oper,
                        etype,
                        axis_access(tac.in1, ispace_axis),
                        axis_access(tac.in2, ispace_axis)
                    )
                );
                break;
            default:
                skel["BODY"] += _assign(
                    axis_access(tac.out, ispace_axis),
                    oper(tac.oper, etype, "", "")
                );
                break;
            }
            skel["BODY"] += _end(oper_description(tac));
            break;

        case KP_REDUCE_COMPLETE:
            //
            // Private accumulator

            // Declare and initialize to neutral
            skel["PROLOG"] += _line(_declare_init(
                operand_glb(tac.in1).etype(),
                operand_glb(tac.in1).accu_private(),
                oper_neutral_element(tac.oper, operand_glb(tac.in1).meta().etype)
            ));
            // Update shared accumulator with value of private accumulator
            skel["EPILOG"] += _line(synced_oper(
                tac.oper,
                operand_glb(tac.in1).meta().etype,
                operand_glb(tac.in1).accu_shared(),
                operand_glb(tac.in1).accu_shared(),
                operand_glb(tac.in1).accu_private()
            )) + _end(" Syncing shared <-> private accumultor var");
            
            // The reduction operation itself

            skel["BODY"] += _assign(
                operand_glb(tac.in1).accu_private(),
                oper(
                    tac.oper,
                    operand_glb(tac.in1).meta().etype,
                    operand_glb(tac.in1).accu_private(),
                    axis_access(tac.in1, ispace_axis)
                )
            );
            skel["BODY"] += _end(oper_description(tac));
            break;

        case KP_REDUCE_PARTIAL:
            // Declare and initialize private accumulator to neutral
            skel["PROLOG"] += _line(_declare_init(
                operand_glb(tac.in1).etype(),
                operand_glb(tac.in1).accu_private(),
                oper_neutral_element(tac.oper, operand_glb(tac.in1).meta().etype)
            ));
            // The reduction operation itself
            skel["BODY"] += _assign(
                operand_glb(tac.in1).accu_private(),
                oper(
                    tac.oper,
                    operand_glb(tac.in1).meta().etype,
                    operand_glb(tac.in1).accu_private(),
                    axis_access(tac.in1, ispace_axis)
                )
            );
            skel["BODY"] += _end(oper_description(tac));

            // TODO: Needs change when streaming since "out" could be intermediate
            skel["EPILOG"] += _assign(
                _deref(operand_glb(tac.out).walker()),
                operand_glb(tac.in1).accu_private()
            )+_end(" Write accumulated variable to array.");
            break;

        case KP_SCAN:
            // Declare and initialize private accumulator to neutral
            skel["PROLOG"] += _line(_declare_init(
                operand_glb(tac.in1).etype(),
                operand_glb(tac.in1).accu_private(),
                oper_neutral_element(tac.oper, operand_glb(tac.in1).meta().etype)
            ));
            // The accumulation operation itself
            skel["BODY"] += _assign(
                operand_glb(tac.in1).accu_private(),
                oper(
                    tac.oper,
                    operand_glb(tac.in1).meta().etype,
                    operand_glb(tac.in1).accu_private(),
                    axis_access(tac.in1, ispace_axis)
                )
            );
            skel["BODY"] += _end("Accumulatation");
            skel["BODY"] += _assign(
                axis_access(tac.out, ispace_axis),
                operand_glb(tac.in1).accu_private()
            );
            skel["BODY"] += _end(oper_description(tac));
            break;

        default:
            skel["BODY"] += "UNSUPPORTED_OPERATION["+ operation_text(tac.op) +"]_AT_EMITTER_STAGE";
            break;
        }
    }
}

string Emitter::simd_reduction_annotation(void)
{
    std::vector<string> annotations;

    for(kernel_tac_iter tit=tacs_begin();
        tit!=tacs_end();
        ++tit) {
        kp_tac& tac = **tit;

        switch(tac.op) {
        case KP_REDUCE_COMPLETE:
        case KP_REDUCE_PARTIAL:
            switch(tac.oper) {
            case KP_ADD:
                annotations.push_back("+:"+operand_glb(tac.in1).accu_private());
                break;
            case KP_MULTIPLY:
                annotations.push_back("*:"+operand_glb(tac.in1).accu_private());
                break;
            case KP_LOGICAL_AND:
                annotations.push_back("&&:"+operand_glb(tac.in1).accu_private());
                break;
            case KP_BITWISE_AND:
                annotations.push_back("&:"+operand_glb(tac.in1).accu_private());
                break;
            case KP_LOGICAL_OR:
                annotations.push_back("||:"+operand_glb(tac.in1).accu_private());
                break;
            case KP_BITWISE_OR:
                annotations.push_back("|:"+operand_glb(tac.in1).accu_private());
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
    
    if (annotations.size()>0) {
        stringstream ss;
        ss << "reduction(";
        for(std::vector<string>::iterator it=annotations.begin(); it!=annotations.end(); ++it) {
            ss << *it;
        }
        ss << ")";
        return ss.str();
    } else {
        return "";
    }
}

string Emitter::generate_source(bool offload)
{
    if ((omask() & KP_ARRAY_OPS)==0) { // There must be at lest one array operation
        cerr << text() << endl;
        throw runtime_error("No array operations!");
    }
    if (omask() & KP_EXTENSION) {      // Extensions are not allowed.
        cerr << text() << endl;
        throw runtime_error("KP_EXTENSION in kernel");
    }

    const int64_t ispace_axis = iterspace().meta().axis;
    const int64_t ispace_ndim = iterspace().meta().ndim;
    const KP_LAYOUT ispace_layout = iterspace().meta().layout;

    Skeleton krn(plaid_, "skel.kernel");

    krn["MODE"]            = (block().narray_tacs()>1) ? "FUSED" : "SIJ";
    krn["LAYOUT"]          = layout_text(ispace_layout);
    krn["NINSTR"]          = to_string(block().ntacs());
    krn["NARRAY_INSTR"]    = to_string(block().narray_tacs());
    krn["NARGS"]           = to_string(block().noperands());
    krn["NARRAY_ARGS"]     = to_string(operands_.size());
    krn["OMASK"]           = omask_text(block().omask());
    krn["SYMBOL_TEXT"]     = block().symbol_text();
    krn["SYMBOL"]          = block().symbol();

    //
    //  Construct kernel HEAD BEFORE any loop constructs / kernel-body.
    //
    krn["HEAD"]    += unpack_iterspace();
    krn["HEAD"]    += unpack_buffers();
    krn["HEAD"]    += unpack_arguments();
    for(kernel_tac_iter tit=tacs_begin(); tit!=tacs_end(); ++tit) {
        kp_tac& tac = **tit;

        if (KP_REDUCE_COMPLETE==tac.op) {
            // Declare and initialize to neutral
            krn["HEAD"] += _line(_declare_init(
                operand_glb(tac.in1).etype(),
                operand_glb(tac.in1).accu_shared(),
                oper_neutral_element(tac.oper, operand_glb(tac.in1).meta().etype)
            ));
        }
    }

    //
    // Construct the kernel FOOT, code AFTER any loop constructs / kernel-body.
    //
    for(kernel_tac_iter tit=tacs_begin(); tit!=tacs_end(); ++tit) {
        kp_tac& tac = **tit;

        if (KP_REDUCE_COMPLETE==tac.op) {
            // Write accumulator to memory
            krn["FOOT"] += _line(_assign(
                _deref(_add(
                    operand_glb(tac.out).buffer_data(),
                    operand_glb(tac.out).start()
                )),
                operand_glb(tac.in1).accu_shared()
            ));
        }
    }
            
    //
    //  Construct the kernel body
    //
    krn["BODY"] = "";

    //
    // Start with a code-block, handling scalars
    Skeleton code_block(plaid_, "skel.block");  // Start with a code-block

    declare_init_opds(code_block, "PROLOG");    // Declare operand variables and offset them

    emit_operations(code_block);                // Fills PROLOG, BODY, EPILOG

    set<uint64_t> written;                      // Write scalars back to memory
    for(kernel_tac_iter tit=tacs_begin();
        tit!=tacs_end();
        ++tit) {
        kp_tac & tac = **tit;

        Operand& opd = operand_glb(tac.out);
        if (((tac.op & (KP_MAP | KP_ZIP | KP_GENERATE))>0) and \
            ((opd.meta().layout & KP_SCALAR)>0) and \
            (written.find(tac.out)==written.end())) {
            code_block["EPILOG"] += _line(_assign(
                _deref(_add(opd.buffer_data(), opd.start())),
                opd.walker_val()
            ));
            written.insert(tac.out);
        }
    }
                                                            // Scalar kernel
    if (((ispace_layout & (KP_SCALAR|KP_SCALAR_CONST|KP_CONTRACTABLE))>0) &&
        ((omask() & KP_ACCUMULATION)==0)) {                 // Accumulations aren't allowed here
                                                            // They should be filtered out.
        krn["BODY"] = code_block.emit();

    } else if (((omask() & KP_SCAN)==0) && (ispace_ndim==1)) { // Promote code_block to loop

        /*
        Emits code on the form:
        #pragma omp parallel - this is the "pblock"
        {
            #pragma omp for schedule(static) - this is the ploop
            {
                // Chunked bound
                #pragma omp simd [reduction(...)] - this is the vloop
                {
                }
            }
        }
        */

        Skeleton vloop(plaid_, "skel.loop");    // Vectorized loop
        vloop["PROLOG"] = _line(_declare_init(
            _const(_int64()),
            "idx"+to_string(ispace_axis)+"_chunked_bound",
            _min(
                "iterspace_shape_d"+to_string(ispace_axis),
                _add("idx"+to_string(ispace_axis)+"_chunked", "KP_CHUNKSIZE")
            )
        ));
        #ifdef CAPE_WITH_OMP_SIMD
        vloop["PRAGMA"] += "#pragma omp simd";
        vloop["PRAGMA"] += " "+simd_reduction_annotation();
        #endif

        vloop["INIT"] = _declare_init(_int64(), "idx"+to_string(ispace_axis),  "idx"+to_string(ispace_axis)+"_chunked");
        vloop["COND"] =_lt(
            "idx"+ to_string(ispace_axis),
            "idx"+ to_string(ispace_axis)+"_chunked_bound"
        );
        vloop["INCR"] = _inc("idx" + to_string(ispace_axis));
        vloop["BODY"] = code_block["BODY"];

        Skeleton ploop(plaid_, "skel.loop");    // Parallel loop
        ploop["INIT"] = _declare_init(_int64(), "idx"+to_string(ispace_axis)+"_chunked",  "0");
        ploop["COND"] =_lt(
            "idx"+to_string(ispace_axis)+"_chunked",
            "iterspace_shape_d"+ to_string(ispace_axis)
        );
        ploop["INCR"] = "idx" + to_string(ispace_axis)+ "_chunked += KP_CHUNKSIZE";

        ploop["PRAGMA"] += "#pragma omp for";
        ploop["PRAGMA"] += " schedule(static)";
        ploop["PROLOG"] = code_block["PROLOG"];
        ploop["EPILOG"] = code_block["EPILOG"];
        ploop["BODY"] = vloop.emit();

        Skeleton pblock(plaid_, "skel.block");
        pblock.reset();                         // Parallel block
        pblock["PRAGMA"] = "#pragma omp parallel";
        pblock["BODY"] = ploop.emit();

        krn["BODY"] = pblock.emit();

    } else {                                        // Promote code_block to nested loop

        Skeleton loop(plaid_, "skel.loop");         // Construct inner-most loop
        loop["INIT"] = _declare_init(_int64(), "idx"+to_string(ispace_axis),  "0");
        loop["COND"] =_lt(
            "idx"+to_string(ispace_axis),
            "iterspace_shape_d"+ to_string(ispace_axis)
        );
        loop["INCR"] = _inc("idx" + to_string(ispace_axis));

        #ifdef CAPE_WITH_OMP_SIMD
        loop["PRAGMA"] += "#pragma omp simd";
        loop["PRAGMA"] += " "+simd_reduction_annotation();
        #endif

        loop["PROLOG"] = code_block["PROLOG"];
        loop["BODY"] = code_block["BODY"];
        loop["EPILOG"] = code_block["EPILOG"];

        krn["BODY"] = loop.emit();                  // Overwrite the code-block in kernel BODY

        vector<int64_t> outer_axes;                 // Determine the outer axes
        for(int64_t axis=ispace_ndim-1; axis>=0; --axis) {
            if (axis==ispace_axis) {
                continue;
            }
            outer_axes.push_back(axis);
        }

        for(vector<int64_t>::iterator idx=outer_axes.begin(); idx!=outer_axes.end(); ++idx) {
            loop.reset();   // Clear the skeleton
   
            if (*idx==outer_axes.back()) {
                loop["PRAGMA"] = "#pragma omp parallel for schedule(static)";
                if (ispace_ndim > 2) {
                    loop["PRAGMA"] += " collapse(" + to_string(ispace_ndim-1) + ")";
                }
            }
 
            loop["INIT"] = _declare_init(_int64(), "idx"+to_string(*idx),  "0");
            loop["COND"] =_lt(
                "idx"+to_string(*idx),
                "iterspace_shape_d"+ to_string(*idx)
            );
            loop["INCR"] = _inc("idx" + to_string(*idx));

            loop["BODY"] = krn["BODY"];

            krn["BODY"] = loop.emit();
        }
    }

    return krn.emit();
}

}}}
