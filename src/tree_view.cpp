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

#include "tree_view.hpp"

#include "utils.hpp"

namespace tsvtree
{

// Returns a vector containing all parents of a leaf node. Represents
// a line in the tsv file.
line_type parents(std::deque<std::deque<tree_node*>> const& st)
{
   line_type r;
   for (auto iter = std::cbegin(st); iter != std::cend(st); ++iter)
      if (!std::empty(*iter))
         r.push_back(iter->back());

   return r;
}

tree_post_order_traversal::
tree_post_order_traversal(tree_node* root, int depth)
: depth_(depth)
, lasts_(depth > 1000 ? 1000 : depth)
{
   if (root)
      st_.push_back({root});
}

line_type tree_post_order_traversal::advance()
{
   while (!std::empty(st_.back().back()->children) && (tsvtree::ssize(st_) <= depth_))
      st_.push_back(st_.back().back()->children);

   auto tmp = parents(st_);
   st_.back().pop_back();
   return tmp;
}

line_type tree_post_order_traversal::next_internal()
{
   st_.pop_back();
   if (std::empty(st_))
      return line_type {};

   auto tmp = parents(st_);
   st_.back().pop_back();
   return tmp;
}

line_type tree_post_order_traversal::next_leaf_node()
{
   while (std::empty(st_.back()))
      if (std::empty(next_internal()))
         return line_type {};

   return advance();
}

line_type tree_post_order_traversal::next_node()
{
   if (std::empty(st_.back()))
      return next_internal();

   return advance();
}

//-------------------------------------------------------------------
tree_tsv_traversal::tree_tsv_traversal(tree_node* root, int depth)
: depth_(depth)
, lasts_(depth > 1000 ? 1000 : depth)
{
   if (root)
      st_.push_back({root});
}

line_type tree_tsv_traversal::advance()
{
   auto line = parents(st_);
   st_.back().pop_back();

   auto const d = depth() == 0 ? 0 : depth() - 1;
   lasts_[d] = std::empty(st_.back());

   if (!std::empty(line.back()->children) && tsvtree::ssize(st_) <= depth_)
      st_.push_back(line.back()->children);

   return line;
}

line_type tree_tsv_traversal::next()
{
   while (std::empty(st_.back())) {
      st_.pop_back();
      if (std::empty(st_))
         return line_type {};
   }

   return advance();
}

}


