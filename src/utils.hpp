/* Copyright (c) 2020 Marcelo Zimbres Silva (mzimbres at gmail dot com)
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
#include <cstdint>
#include <iterator>

namespace tsvtree
{

template <class C>
auto ssize(C const& c)
   { return static_cast<int>(std::size(c)); }

using code_type = std::uint64_t;
code_type make_code(std::vector<int> const& c, int depth);

struct tree_elem {
   std::string data;
   int depth = 0;
   int version = 0;
};

std::vector<code_type>
to_channel_codes(std::vector<std::vector<int>> const& o,
                 int depth,
                 int max);

std::string to_string(std::vector<int> const& v, char delimiter = '.');

std::string make_deco_indent(int depth, std::vector<bool> const& lasts);

std::vector<std::string> split_line(std::string const& in, char sep);

}

