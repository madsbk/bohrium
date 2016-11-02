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

#ifndef __BH_JITK_BLOCK_HPP
#define __BH_JITK_BLOCK_HPP

#include <set>
#include <vector>
#include <iostream>

#include <bh_instruction.hpp>

namespace bohrium {
namespace jitk {

class Block {
public:
    std::vector <Block> _block_list;
    bh_instruction *_instr = NULL;
    int rank;
    int64_t size;
    std::set<bh_instruction*> _sweeps;
    std::set<bh_base *> _news;
    std::set<bh_base *> _frees;
    bool _reshapable = false;

    // Unique id of this block
    int _id;

    // Default constructor
    Block() { static int id_count = 0; _id = id_count++; }

    // Block Instruction Constructor
    // Note, the rank is only to make pretty printing easier
    Block(bh_instruction *instr, int rank) : Block() {
        _instr = instr;
        this->rank = rank;
    }

    // Returns true if this block is an instruction block, which has a
    // empty block list and a non-NULL instruction pointer
    bool isInstr() const {
        assert(_block_list.size() > 0 or _instr != NULL);
        return _block_list.size() == 0;
    }

    // Find the 'instr' in this block or in its children
    // Returns NULL if not found
    Block* findInstrBlock(const bh_instruction *instr);

    // Is this block innermost? (not counting instruction blocks)
    bool isInnermost() const {
        for (const Block &b: _block_list) {
            if (not b.isInstr()) {
                return false;
            }
        }
        return true;
    }

    // Pretty print this block
    std::string pprint() const;

    // Return all sub-blocks (incl. nested blocks)
    void getAllSubBlocks(std::vector<const Block *> &out) const;
    std::vector<const Block*> getAllSubBlocks() const;

    // Return all sub-blocks (excl. nested blocks)
    std::vector<const Block*> getLocalSubBlocks() const;

    // Return all instructions in the block (incl. nested blocks)
    void getAllInstr(std::vector<bh_instruction*> &out) const;
    std::vector<bh_instruction*> getAllInstr() const;

    // Return instructions in the block (excl. nested blocks)
    void getLocalInstr(std::vector<bh_instruction*> &out) const;
    std::vector<bh_instruction*> getLocalInstr() const;

    // Return all bases accessed by this block
    std::set<bh_base*> getAllBases() const {
        std::set<bh_base*> ret;
        for (bh_instruction *instr: getAllInstr()) {
            std::set<bh_base*> t = instr->get_bases();
            ret.insert(t.begin(), t.end());
        }
        return ret;
    }

    // Return all new arrays in this block (incl. nested blocks)
    void getAllNews(std::set<bh_base*> &out) const;
    std::set<bh_base*> getAllNews() const;

    // Return all freed arrays in this block (incl. nested blocks)
    void getAllFrees(std::set<bh_base*> &out) const;
    std::set<bh_base*> getAllFrees() const;

    // Return all local temporary arrays in this block (excl. nested blocks)
    // NB: The returned temporary arrays are the arrays that this block should declare
    void getLocalTemps(std::set<bh_base*> &out) const;
    std::set<bh_base*> getLocalTemps() const;

    // Return all temporary arrays in this block (incl. nested blocks)
    void getAllTemps(std::set<bh_base*> &out) const;
    std::set<bh_base*> getAllTemps() const;

    // Returns true when all instructions within the block is system or if the block is empty()
    bool isSystemOnly() const {
        if (isInstr()) {
            return bh_opcode_is_system(_instr->opcode);
        }
        for (const Block &b: _block_list) {
            if (not b.isSystemOnly()) {
                return false;
            }
        }
        return true;
    }

    // Validation check of this block
    bool validation() const;

    // Equality test based on the unique ID
    bool operator==(const Block &block) const {
        return this->_id == block._id;
    }

    // Determines whether this block must be executed after 'other'
    bool depend_on(const Block &other) const {
        const std::vector<bh_instruction*> this_instr_list = getAllInstr();
        const std::vector<bh_instruction*> other_instr_list = other.getAllInstr();
        for (const bh_instruction *this_instr: this_instr_list) {
            for (const bh_instruction *other_instr: other_instr_list) {
                if (this_instr != other_instr and
                    bh_instr_dependency(this_instr, other_instr)) {
                    return true;
                }
            }
        }
        return false;
    }

    // Append an instruction list to this block.
    // NB: Force reshape the instructions to match the last instructions within this block
    void append_instr_list(const std::vector<bh_instruction*> &instr_list) {
        assert(validation());
        assert(not isInstr());

        // Find the shape of the last instruction within this block
        if (not _block_list.back().isInstr()) {
            return _block_list.back().append_instr_list(instr_list);
        }
        const std::vector<int64_t> &shape = _block_list.back()._instr->dominating_shape();

        // Reshape and insert the instructions
        for (bh_instruction *instr: instr_list) {
            instr->reshape_force(shape);
            _block_list.emplace_back(instr, rank+1);
        }
        assert(validation());
    }

    // Prepend an instruction list to this block.
    // NB: Force reshape the instructions to match the last instructions within this block
    void prepend_instr_list(const std::vector<bh_instruction*> &instr_list) {
        assert(validation());
        assert(not isInstr());

        // Find the shape of the first instruction within this block
        if (not _block_list.front().isInstr()) {
            return _block_list.front().prepend_instr_list(instr_list);
        }
        const std::vector<int64_t> &shape = _block_list.front()._instr->dominating_shape();

        std::vector<Block> new_block_list;
        new_block_list.reserve(instr_list.size());
        for (bh_instruction *instr: instr_list) {
            instr->reshape_force(shape);
            new_block_list.emplace_back(instr, rank+1);
        }
        new_block_list.insert(new_block_list.end(), _block_list.begin(), _block_list.end());
        _block_list = new_block_list;
        assert(validation());
    }
};

// Merge the two blocks, 'a' and 'b', in that order. When 'based_on_block_b' is
// true, the meta-data such at size, rank etc. is taken from 'b' rather than 'a'
Block merge(const Block &a, const Block &b, bool based_on_block_b=false);

// Create a nested block based on 'instr_list' with the sets of new, free, and temp arrays given.
// The dimensions from zero to 'rank-1' are ignored.
// The 'size_of_rank_dim' specifies the size of the dimension 'rank'.
// 'news' is the set of instructions that creates new arrays
// 'temps' is the set of arrays that a temporary in the blocks "above" this block
Block create_nested_block(std::vector<bh_instruction*> &instr_list, int rank, int64_t size_of_rank_dim,
                          const std::set<bh_instruction*> &news, const std::set<bh_base*> &temps = std::set<bh_base*>());

//Implements pprint of block
std::ostream& operator<<(std::ostream& out, const Block& b);


//Implements pprint of a vector of blocks
std::ostream& operator<<(std::ostream& out, const std::vector<Block>& b);

} // jit
} // bohrium


#endif
