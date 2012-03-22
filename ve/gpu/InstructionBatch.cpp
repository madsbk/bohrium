/*
 * Copyright 2011 Troels Blum <troels@blum.dk>
 *
 * This file is part of cphVB <http://code.google.com/p/cphvb/>.
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
 * along with cphVB. If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <sstream>
#include <cassert>
#include <stdexcept>
#include <cphvb.h>
#include "InstructionBatch.hpp"
#include "GenerateSourceCode.hpp"

int InstructionBatch::kernelNo = 0;
InstructionBatch::KernelMap InstructionBatch::kernelMap = InstructionBatch::KernelMap();

InstructionBatch::InstructionBatch(cphvb_instruction* inst, const std::vector<BaseArray*>& operandBase)
    : arraynum(0)
    , scalarnum(0)
    , variablenum(0)
{
#ifdef STATS
    gettimeofday(&createTime,NULL);
#endif
    if (inst->operand[0]->ndim > 3)
        throw std::runtime_error("More than 3 dimensions not supported.");        
    shape = std::vector<cphvb_index>(inst->operand[0]->shape, inst->operand[0]->shape + inst->operand[0]->ndim);
    add(inst, operandBase);
}

bool InstructionBatch::shapeMatch(cphvb_intp ndim,const cphvb_index dims[])
{
    int size = shape.size();
    if (ndim == size)
    {
        for (int i = 0; i < ndim; ++i)
        {
            if (dims[i] != shape[i])
                return false;
        }
        return true;
    }
    return false;
}

bool InstructionBatch::sameView(const cphvb_array* a, const cphvb_array* b)
{
    //assumes the the views shape are the same and they have the same base
    if (a == b)
        return true;
    if (a->start != b->start)
        return false;
    for (size_t i = 0; i < shape.size(); ++i)
    {
        if (a->stride[i] != b->stride[i])
            return false;
    }
    return true;
}

inline int gcd(int a, int b)
{
    int c = a % b;
    while(c != 0)
    {
        a = b;
        b = c;
        c = a % b;
    }
    return b;
}

bool InstructionBatch::disjointView(const cphvb_array* a, const cphvb_array* b)
{
    //assumes the the views shape are the same and they have the same base
    int astart = a->start;
    int bstart = b->start;
    int stride = 1;
    for (int i = 0; i < a->ndim; ++i)
    {
        stride = gcd(a->stride[i], b->stride[i]);
        int as = astart / stride;
        int bs = bstart / stride;
        int ae = as + a->shape[i] * (a->stride[i]/stride);
        int be = bs + b->shape[i] * (b->stride[i]/stride);
        if (ae <= bs || be <= as)
            return true;
        astart %= stride;
        bstart %= stride;
    }
    if (stride > 1 && a->start % stride != b->start % stride)
        return true;
    return false;
}

void InstructionBatch::add(cphvb_instruction* inst, const std::vector<BaseArray*>& operandBase)
{
    assert(!operandBase[0]->isScalar());

    // Check that the shape matches
    if (!shapeMatch(inst->operand[0]->ndim, inst->operand[0]->shape))
        throw BatchException(0);

    // If any operand's base is already used as output, it has to be alligned or disjoint
    for (size_t op = 0; op < operandBase.size(); ++op)
    {
        if (!cphvb_scalar(inst->operand[op]))
        {
            OutputMap::iterator oit = output.find(operandBase[op]);
            if (oit != output.end())
            {
                if (sameView(oit->second, inst->operand[op]))
                {
                    inst->operand[op] = oit->second;
                } 
                else if (!disjointView(oit->second, inst->operand[op])) 
                { 
#ifdef DEBUG
                    std::cout << "FAIL: disjointView("<< oit->second << ", " << 
                        inst->operand[op] << ")" << std::endl;
#endif
                    throw BatchException(0);
                }
            }
        }
    }

    // If the output operans is allready used as input it has to be alligned or disjoint
    std::pair<InputMap::iterator, InputMap::iterator> irange = input.equal_range(operandBase[0]);
    for (InputMap::iterator iit = irange.first ; iit != irange.second; ++iit)
    {
        if (sameView(iit->second, inst->operand[0]))
        {
            inst->operand[0] = iit->second;
        } 
        else if (!disjointView(iit->second, inst->operand[0]))
        {
#ifdef DEBUG
            std::cout << "FAIL: disjointView("<< iit->second << ", " << 
                inst->operand[0] << ")" << std::endl;
#endif
            throw BatchException(0);
        }
    }

    // OK so we can accept the instruction
    instructions.push_back(inst);
    // Are some of the input parameters allready know? Otherwise register them
    for (size_t op = 1; op < operandBase.size(); ++op)
    {
        OutputMap::iterator oit = output.find(operandBase[op]);
        if (oit != output.end() && sameView(oit->second, inst->operand[op]))
        {  //it is allready part of the output
            inst->operand[op] = oit->second;
            continue;
        }
        bool found = false;
        irange = input.equal_range(operandBase[op]);
        for (InputMap::iterator iit = irange.first ; iit != irange.second; ++iit)
        {
            if (sameView(iit->second, inst->operand[op]))
            { //it is allready part of the input
                inst->operand[op] = iit->second;
                found = true;
                break;
            }
        }
        if (!found)
        { //it is a new array or view
            input.insert(std::make_pair(operandBase[op], inst->operand[op]));
        }
    }
    

    // Register output
    output[operandBase[0]] = inst->operand[0];
    
    // Register Kernel parameters
    for (size_t op = 0; op < operandBase.size(); ++op)
    {
        cphvb_array* base = cphvb_base_array(inst->operand[op]);
        if (parameters.find(base) == parameters.end())
        {
            std::stringstream ss;
            ss << "p" << arraynum++;
            parameters[base] = std::make_pair(operandBase[op],ss.str()); 
        }
    }
}

Kernel InstructionBatch::generateKernel(ResourceManager* resourceManager)
{
#ifdef STATS
    timeval start, end;
    gettimeofday(&start,NULL);
#endif
    std::string code = generateCode();
#ifdef STATS
    gettimeofday(&end,NULL);
    resourceManager->batchSource += (end.tv_sec - start.tv_sec)*1000000.0 + (end.tv_usec - start.tv_usec);
#endif

    KernelMap::iterator kit = kernelMap.find(code);
    if (kit == kernelMap.end())
    {
        std::stringstream source, kname;
        kname << "kernel" << kernelNo++;
        source << "__kernel void " << kname.str() << code;
        Kernel kernel(resourceManager, shape.size(), source.str(), kname.str());
        kernelMap.insert(std::make_pair(code, kernel));
        return kernel;
    } else {
        return kit->second;
    }
}

void InstructionBatch::run(ResourceManager* resourceManager)
{
#ifdef STATS
    timeval now;
    gettimeofday(&now,NULL);
    resourceManager->batchBuild += (now.tv_sec - createTime.tv_sec)*1000000.0 + (now.tv_usec - createTime.tv_usec);
#endif

    if (output.begin() != output.end())
    {
        Kernel kernel = generateKernel(resourceManager);
        Kernel::Parameters kernelParameters;
        for (ParameterMap::iterator pit = parameters.begin(); pit != parameters.end(); ++pit)
        {
            if (output.find(pit->second.first) == output.end())
                kernelParameters.push_back(std::make_pair(pit->second.first, false));
            else
                kernelParameters.push_back(std::make_pair(pit->second.first, true));
        }
        kernel.call(kernelParameters, shape);
    }
}

std::string InstructionBatch::generateCode()
{
    std::stringstream source;
    source << "( ";
    // Add Array kernel parameters
    ParameterMap::iterator pit = parameters.begin();
    pit->second.first->printKernelParameterType(true, source);
    source << " " << pit->second.second;
    for (++pit; pit != parameters.end(); ++pit)
    {
        source << "\n                     , ";
        pit->second.first->printKernelParameterType(true, source);
        source << " " << pit->second.second;
    }

    source << ")\n{\n";
    
    generateGIDSource(shape.size(), source);
    
    // Load input parameters
    for (InputMap::iterator iit = input.begin(); iit != input.end(); ++iit)
    {
        if (!iit->first->isScalar())
        {
            std::stringstream ss;
            ss << "v" << variablenum++;
            kernelVariables[iit->second] = ss.str();
            source << "\t" << oclTypeStr(iit->first->type()) << " " << ss.str() << " = " <<
                parameters[cphvb_base_array(iit->second)].second << "[";
            generateOffsetSource(iit->second, source);
            source << "];\n";
        }
    }

    // Generate code for instructions
    for (std::vector<cphvb_instruction*>::iterator iit = instructions.begin(); iit != instructions.end(); ++iit)
    {
        std::vector<std::string> operands;
        // Has the output operand been assigned a variable name?
        VariableMap::iterator kvit = kernelVariables.find((*iit)->operand[0]);
        if (kvit == kernelVariables.end())
        {
            std::stringstream ss;
            ss << "v" << variablenum++;
            kernelVariables[(*iit)->operand[0]] = ss.str();
            operands.push_back(ss.str());
            source << "\t" << oclTypeStr(oclType((*iit)->operand[0]->type)) << " " << ss.str() << ";\n";
        }
        else
        {
            operands.push_back(kvit->second);
        }
        // find variable names for input operands
        for (int op = 1; op < cphvb_operands((*iit)->opcode); ++op)
        {
            if (cphvb_scalar((*iit)->operand[op]))
                operands.push_back(parameters[cphvb_base_array((*iit)->operand[op])].second);  
            else
                operands.push_back(kernelVariables[(*iit)->operand[op]]);  
        }

        // generate source code for the instruction
        generateInstructionSource((*iit)->opcode, oclType((*iit)->operand[0]->type), operands, source);
    }

    // Save output parameters
    for (OutputMap::iterator oit = output.begin(); oit != output.end(); ++oit)
    {
        source << "\t" << parameters[cphvb_base_array(oit->second)].second << "[";
        generateOffsetSource(oit->second, source);
        source << "] = " <<  kernelVariables[oit->second] << ";\n";
    }

    source << "}\n";
    return source.str();
}

bool InstructionBatch::read(BaseArray* array)
{
    if (input.find(array) == input.end())
        return false;
    return true;
}

bool InstructionBatch::write(BaseArray* array)
{
    if (output.find(array) == output.end())
        return false;
    return true;
}

bool InstructionBatch::access(BaseArray* array)
{
    return (read(array) || write(array));
}

bool InstructionBatch::discard(BaseArray* array)
{
    OutputMap::iterator oit = output.find(array);
    bool r =  read(array);
    if (oit != output.end())
    {
        output.erase(oit);
        if (!r)
            parameters.erase(array->getSpec());
    }
    return !r;
}
