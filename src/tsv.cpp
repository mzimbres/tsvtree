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

#include "tsv.hpp"

#include <stack>
#include <deque>
#include <vector>
#include <string>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <iterator>
#include <iostream>
#include <algorithm>

#include "utils.hpp"

namespace tsvtree
{

struct line_comp {
   int depth;
   auto operator()( std::vector<std::string> const& v1
                  , std::vector<std::string> const& v2) const
   { return v1.at(depth) < v2.at(depth); };
};

struct line_comp_pred {
   std::string s;
   int depth;
   auto operator()(std::vector<std::string> const& v) const
   { return s == v.at(depth); }
};

struct range {
   using iter_type = std::vector<std::vector<std::string>>::iterator;
   iter_type begin;
   iter_type end;
   int depth;
};

auto
make_tree_line(std::string const& name,
               int depth,
               int indent_size,
               char out_field_sep,
               std::vector<bool> const& lasts,
	       bool decorate)
{
   if (indent_size < 0) {
      auto ret = std::to_string(depth);
      ret += out_field_sep;
      ret += name;
      return ret;
   }

   if (decorate) {
     auto ret = make_deco_indent(depth, lasts);
     ret += name;
     return ret;
   }

   auto ret = std::string(depth, '\t');
   ret += name;
   return ret;
}

auto make_ranges(range const& r, int col)
{
   auto f = [=](auto const& l)
      { return ssize(l) > col; };

   auto const first_ok = std::partition(r.begin, r.end, f);

   std::sort(r.begin, first_ok, line_comp {col});

   std::deque<range> ret;
   auto iter = r.begin;
   while (iter != first_ok) {
      auto point =
         std::partition_point(
            iter,
            first_ok,
            line_comp_pred {iter->at(col), col});

      ret.push_front({iter, point, col});
      iter = point;
   }

   return ret;
}

std::string parse_tree(
   std::vector<std::vector<std::string>> data,
   int indent_size,
   char line_break,
   char out_field_sep,
   bool decorate)
{
   if (std::empty(data))
      return {};

   auto begin = std::begin(data);
   auto end = std::end(data);

   std::deque<std::deque<range>> st;
   auto const root_ranges = make_ranges({begin, end, 0}, 0);
   st.push_back({root_ranges});

   std::vector<bool> lasts(1000);
   lasts[0] = std::size(root_ranges) == 1;

   std::string ret;
   auto shift = 0;
   if (std::size(root_ranges) > 1) {
      // There is no root node so we have to add it here. We can only
      // parse trees that have a root node.
      ret += make_tree_line("Root", 0, indent_size, out_field_sep, lasts, decorate);
      ret += line_break;
      ++shift;
   }

   while (!std::empty(st)) {
      auto r = st.back().back();
      st.back().pop_back();

      auto const d = r.depth > 0 && shift == 0 ? 1 : 0;
      lasts[r.depth - d] = std::empty(st.back());

      if (std::empty(st.back()))
         st.pop_back();

      ret += make_tree_line(r.begin->at(r.depth),
                            r.depth + shift,
                            indent_size,
                            out_field_sep,
                            lasts,
			    decorate);
      ret += line_break;

      auto const next = r.depth + 1;
      if (next >= ssize(*r.begin))
         continue;

      st.push_back(std::move(make_ranges(r, next)));
   }

   return ret;
}

auto split_line(std::string const& in, char sep)
{
   std::string line;
   std::istringstream iss(in);
   std::vector<std::string> ret;
   while (std::getline(iss, line, sep))
      if (!std::empty(line))
         ret.push_back(line);

   return ret;
}

std::vector<std::vector<std::string>>
parse_tsv(std::string const& in, char sep)
{
   std::string line;
   std::istringstream iss(in);
   std::vector<std::vector<std::string>> ret;
   while (std::getline(iss, line, '\n')) {
      auto v = split_line(line, sep);
      if (!std::empty(v))
         ret.push_back(std::move(v));
   }

   return ret;
}

std::string
make_tree_string(std::string const& content,
                 tsv_cfg const& op)
{
   auto table = parse_tsv(content, op.in_field_sep);

   return parse_tree(std::move(table),
                     op.indentation,
                     op.out_line_break,
                     op.out_field_sep,
		     op.decorate);
}
}

