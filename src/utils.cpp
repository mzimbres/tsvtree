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

#include "utils.hpp"

#include <vector>
#include <sstream>
#include <cassert>
#include <algorithm>

namespace tsvtree
{

code_type make_code_d1(std::vector<int> const& c)
{
   if (std::size(c) < 1)
      return 0;

   return c[0];
}

code_type make_code_d2(std::vector<int> const& c)
{
   if (std::size(c) < 2)
      return 0;

   code_type ca = c[0];
   code_type cb = c[1];

   constexpr auto shift = 32;

   ca <<= shift;
   return ca | cb;
}

code_type make_code_d3(std::vector<int> const& c)
{
   if (std::size(c) < 3)
      return 0;

   code_type ca = c[0];
   code_type cb = c[1];
   code_type cc = c[2];

   auto constexpr shift = 21;

   ca <<= 2 * shift;
   cb <<= 1 * shift;
   return ca | cb | cc;
}

code_type make_code_d4(std::vector<int> const& c)
{
   if (std::size(c) < 4)
      return 0;

   code_type ca = c[0];
   code_type cb = c[1];
   code_type cc = c[2];
   code_type cd = c[3];

   auto constexpr shift = 16;

   ca <<= 3 * shift;
   cb <<= 2 * shift;
   cc <<= 1 * shift;
   return ca | cb | cc | cd;
}

code_type make_code(std::vector<int> const& c, int depth)
{
   switch (depth) {
      case 1: return make_code_d1(c);
      case 2: return make_code_d2(c);
      case 3: return make_code_d3(c);
      case 4: return make_code_d4(c);
      default: return 0;
   }
}

std::vector<code_type>
to_channel_codes(std::vector<std::vector<int>> const& o,
                 int depth,
                 int max)
{
   auto const max_channels = std::min(tsvtree::ssize(o), max);

   auto f = [depth](auto const& e)
      { return make_code(e, depth); };

   std::vector<code_type> ret;
   std::transform(std::cbegin(o),
                  std::cbegin(o) + max_channels,
                  std::back_inserter(ret),
                  f);

   std::sort(std::begin(ret), std::end(ret));
   return ret;
}

std::string to_str_raw(int i, int width, char fill)
{
   std::ostringstream oss;
   oss.fill(fill);
   oss.width(width);
   oss << std::hex << i;
   return oss.str();
}

std::string to_str(int i)
{
   return to_str_raw(i, 10, '0');
}

std::string to_string(std::vector<int> const& v, char delimiter)
{
   if (std::empty(v))
      return {};

   if (std::size(v) == 1)
      return to_str_raw(v.front(), 3, '0');

   std::string code;
   for (auto i = 0; i != tsvtree::ssize(v) - 1; ++i)
      code += to_str_raw(v[i], 3, '0') + delimiter;

   code += to_str_raw(v.back(), 3, '0');
   return code;
}

char const* make_indent_block(int i, int depth, bool last)
{
   static char const* blocks[] =
   { ""     // 0
   , "    " // 1
   , "│   " // 2
   , "└── " // 3
   , "├── " // 4
   , "Please, report error for entry: "
   };

   if (depth == 0)             return blocks[0];
   if (i < depth - 1 && last)  return blocks[1];
   if (i < depth - 1)          return blocks[2];
   if (i == depth - 1 && last) return blocks[3];
   if (i == depth - 1)         return blocks[4];

   assert(false);
   return blocks[5];
}

std::string make_deco_indent(int depth, std::vector<bool> const& lasts)
{
  std::string ret;
  for (auto i = 0; i < depth; ++i)
     ret += make_indent_block(i, depth, lasts[i]);
  return ret;
}

std::vector<std::string> split_line(std::string const& in, char sep)
{
   std::string line;
   std::istringstream iss(in);
   std::vector<std::string> ret;
   while (std::getline(iss, line, sep))
      if (!std::empty(line))
         ret.push_back(line);

   return ret;
}

} // tsvtree

