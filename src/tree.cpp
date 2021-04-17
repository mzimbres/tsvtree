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

#include "tree.hpp"

#include <cassert>
#include <iterator>
#include <numeric>
#include <algorithm>
#include <exception>

#include "tree_parser.hpp"
#include "utils.hpp"

namespace tsvtree
{

tree::tree(std::string const& str, oconfig const& cfg)
{
   // TODO: Catch exceptions and release already acquired memory.
   auto const p = parse_tree(str, cfg);
   head_ = p.first;
   max_depth_ = p.second;
}

tree_node* tree::at(std::vector<int> const& coord)
{
  if (std::empty(coord))
     return nullptr;

  auto* ret = &head_;
  for (auto i = 0; i < tsvtree::ssize(coord); ++i) {
     if (coord[i] >= tsvtree::ssize(ret->children))
        return ret;

     ret = ret->children[coord[i]];
  }

  return ret;
}

auto node_leaf_counter(tree_node const& node)
{
   if (std::empty(node.children))
      return 0;

   auto acc = [](auto a, auto const* p)
   {
      if (std::empty(p->children))
         return a + 1;
      return a + p->leaf_counter;
   };

   return
      std::accumulate(std::begin(node.children),
                      std::end(node.children),
                      0,
                      acc);
}

void tree::load_leaf_counters()
{
   if (empty())
      return;

   auto f = [](auto& node)
      { node.leaf_counter = node_leaf_counter(node); };

   tree_postorder_view view {head_.children.front()};
   std::for_each(std::cbegin(view), std::cend(view), f);
}

tree::~tree()
{
   tree_postorder_view view {at({0})};
   for (auto iter = std::begin(view); iter != std::end(view); ++iter)
      delete iter.line().back();
}

} // tsvtree

