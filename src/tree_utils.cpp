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

#include <ios>
#include <stack>
#include <limits>
#include <cctype>
#include <vector>
#include <numeric>
#include <cassert>
#include <sstream>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <exception>

#include "tree_parser.hpp"
#include "utils.hpp"
#include "tsv.hpp"

#include <fmt/format.h>

namespace tsvtree
{

auto const* tikz_node =
   "\\treenode[fill=depthC{}] ({}) at ({}pt, {}pt) {{\\color{{textC}}{}}};";

auto const* tikz_arrow =
   "\\treearrow[color=arrowC] ({}.west) to ({}pt, {}pt) to ({}.south west);";

auto
node_dump(tree_node const& node,
          oconfig::format of,
          char field_sep,
          std::vector<bool> const& lasts,
	  int line,
	  oconfig::tikz const& conf,
          int at_depth)
{
   auto const depth = tsvtree::ssize(node.code) - at_depth;
   assert(depth >= 0);

   if (of == oconfig::format::tree) {
      std::string ret(depth, '\t');
      ret += node.name;
      return ret;
   }

   if (of == oconfig::format::comp) {
      std::string ret;
      ret += std::to_string(depth);
      ret += field_sep;
      ret += node.name;
      return ret;
   }

   if (of == oconfig::format::tree_deco) {
      auto ret = make_deco_indent(depth, lasts);
      ret += node.name;
      return ret;
   }

   if (of == oconfig::format::tikz) {
      auto const x = depth * conf.x_step;
      auto y = - line * conf.y_step;

      auto const name = "n" + to_string(node.code, '-');
      auto node_line = fmt::format(tikz_node, depth, name, x, y, node.name);

      if (depth == 0)
         return node_line;

      std::vector<int> const parent_code =
         {std::begin(node.code), std::prev(std::end(node.code))};

      auto const parent_name = "n" + to_string(parent_code, '-');
      y += conf.y_step / 2;
      node_line += "\n";
      node_line +=
         fmt::format(tikz_arrow, name, (depth - 1) * conf.x_step, y, parent_name);

      return node_line;
   }

   return to_string(node.code);
}

std::string
serialize(tree_node* p,
          oconfig::format of,
          char line_break,
          int max_depth,
          int at_depth,
          char field_sep,
	  oconfig::tikz const& conf)
{
   tree_tsv_view view {p, max_depth};

   std::string ret;
   int line = 0;
   for (auto iter = std::begin(view); iter != std::end(view); ++iter) {
      ret += node_dump(*iter,
		       of,
		       field_sep,
		       iter.lasts(),
		       line++,
		       conf,
                       at_depth);
      ret += line_break;
   }
   return ret;
}

std::string
join(std::vector<tree_node*> const& line, char field_sep)
{
   if (std::empty(line))
      return std::string {};

   auto acc = [=](auto init, auto const* p)
   {
      init += p->name;
      init += field_sep;
      return init;
   };

   auto ret =
      std::accumulate(std::cbegin(line),
                      std::prev(std::cend(line)),
                      std::string {},
                      acc);

   ret += line.back()->name;
   return ret;
}

std::string check_min_depth(tree_level_view view, int min_depth)
{
   for (auto iter = std::begin(view); iter != std::end(view); ++iter)
      if (iter.depth() < min_depth)
         return join(iter.line(), '\t');

   return {};
}

} // tsvtree
