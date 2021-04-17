/* Copyright (c) 2020 - 2021 Marcelo Zimbres Silva (mzimbres at gmail dot com)
 *
 * This file is part of tsvtree.
 * 
 * tsvtree is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * tsvtree is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with tsvtree.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <vector>
#include <string>
#include <limits>

#include "utils.hpp"
#include "tree_node.hpp"
#include "tree_view.hpp"
#include "tree_utils.hpp"

namespace tsvtree
{

class tree {
private:
   tree_node head_;
   int max_depth_ = 0;
 
public:
   tree(tree const&) = delete;
   tree& operator=(tree const&) = delete;
   tree(tree&&) = delete;
   tree& operator=(tree&&) = delete;
   tree(std::string const& str, oconfig const& conf);
   ~tree();

   bool empty() const noexcept { return std::empty(head_.children); }
   bool max_depth() const noexcept {return max_depth_;};
   void load_leaf_counters();

   // Returns a pointer to the node at the specified position in the tree where
   // [0] is the root node.
   //
   // TODO: Make this private
   tree_node* at(std::vector<int> const& coord);

   tree_level_view
   level_view(
      std::vector<int> const& coord,
      int depth = std::numeric_limits<int>::max())
      { return {at(coord), depth}; }

   tree_postorder_view
   postorder_view(
      std::vector<int> const& coord,
      int depth = std::numeric_limits<int>::max())
      { return {at(coord), depth}; }

   tree_tsv_view
   tsv_view(
      std::vector<int> const& coord,
      int depth = std::numeric_limits<int>::max())
      { return {at(coord), depth}; }
};

} // tsvtree
