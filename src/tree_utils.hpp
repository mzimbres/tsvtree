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

#include <stack>
#include <deque>
#include <vector>
#include <string>
#include <limits>

#include "utils.hpp"
#include "tree_node.hpp"
#include "tree_view.hpp"

namespace tsvtree
{

struct oconfig {
   enum class format
   { tree
   , tree_deco
   , comp
   , info
   , tsv
   , tikz
   , check_min_depth
   , invalid
   };

   struct tikz {
      int y_step = 16;
      int x_step = 20;
   };
   
   char field_sep = '\t';
   char line_break = '\n';

   format fmt = format::tree;
   tikz tikz_conf;
};

std::string
join(std::vector<tree_node*> const& line, char field_sep);

// Returns the line that does not have the minimun depth.
std::string check_min_depth(tree_level_view view, int min_depth);

/* Detects the file tree format which can be
 *
 *    1. Node depth from identation.
 *       
 *       |r
 *       |   a
 *       |      b
 *       |      c
 *       |   d
 *       |      e
 *       |   f
 *
 *    2. Node depth from number.
 *
 *       |0;r
 *       |1;a
 *       |2;b
 *       |2;c
 *       |1;d
 *       |2;e
 *       |1;f
 *
 * To do it we can read the first line and count the number of
 * field separators. If it is not zero, it is format 2.
 */
oconfig::format
detect_iformat(std::string const& tree_str,
               char line_break,
               char field_sep,
               bool tsv);

std::string
serialize(tree_node* p,
          oconfig::format of,
          char line_sep,
          int max_depth,
          int at_depth,
          char field_sep,
	  oconfig::tikz const& conf = {});

} // tsvtree
