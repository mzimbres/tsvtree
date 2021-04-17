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

#include "tree_parser.hpp"

#include <stack>
#include <vector>
#include <cassert>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <exception>

#include "utils.hpp"
#include "tree.hpp"

namespace tsvtree
{

auto
remove_depth(std::string& line,
             oconfig::format ifmt,
             char field_sep)
{
   if (std::empty(line))
      return -1;

   if (ifmt == oconfig::format::tree) {
      auto const i = line.find_first_not_of('\t');
      if (i == std::string::npos)
         throw std::runtime_error("Invalid line.");

      line.erase(0, i);
      return static_cast<int>(i);
   }

   if (ifmt == oconfig::format::comp) {
      if (std::empty(line))
         return -1;

      auto const p1 = line.find_first_of(field_sep);
      if (p1 == std::string::npos)
         throw std::runtime_error("No field separator found in line.");

      auto const p2 = line.find_first_of(field_sep, p1 + 1);
      if (p2 == std::string::npos) {
         auto const digit = line.substr(0, p1);
         line.erase(0, p1 + 1);
         // Now the line contains only the middle field.
         return std::stoi(digit);
      }

      // The middle data cannot be empty.
      if (p2 == p1 + 1)
         throw std::runtime_error("Invalid line.");

      auto const digit = line.substr(0, p1);
      line.erase(0, p1 + 1);
      line.erase(p2);

      // Now the line contains only the middle field.
      return std::stoi(digit);
   }

   return -1;
}

auto first_line(std::string const& tree_str, char line_break)
{
   std::stringstream ss(tree_str);
   std::string line;
   while (std::getline(ss, line, line_break))
      if (!std::empty(line))
         return line;

   return std::string {};
}

oconfig::format
detect_iformat(std::string const& tree_str,
               char line_break,
               char field_sep,
               bool tsv)
{
   if (tsv)
      return oconfig::format::tsv;

   auto const line = first_line(tree_str, line_break);

   auto const n =
      std::count(std::cbegin(line),
                 std::cend(line),
                 field_sep);
   if (n > 0)
      return oconfig::format::comp;

   return oconfig::format::tree;
}

class tree_parser {
private:
   std::vector<int> codes_;
   std::stack<tree_node*> stack_;
   int last_depth_ = 0;
   tree_node head_;
   int max_depth_ = 0;

public:
   tree_parser(int max_depth) : codes_(max_depth, -1) { }
   auto head() const noexcept {return head_;};
   auto max_depth() const noexcept {return max_depth_;};
   void add_line(std::string line, oconfig const& cfg)
   {
      auto const depth =
         remove_depth(line,
                      cfg.fmt,
                      cfg.field_sep);

      if (depth == -1)
         return;

      if (depth > max_depth_)
         max_depth_ = depth;

      if (std::empty(head_.children)) {
         auto* p = new tree_node {line, {0}};
         head_.children.push_front(p);
         stack_.push(p);
         return;
      }

      if (depth == 0)
         throw std::runtime_error("Unknown file input format.");

      if (tsvtree::ssize(codes_) <= depth)
         return; // Line is ignored.

      ++codes_.at(depth - 1);
      for (auto i = depth; i < tsvtree::ssize(codes_); ++i)
         codes_[i] = -1;

      std::vector<int> code {0};
      code.insert(std::end(code), std::cbegin(codes_), std::cbegin(codes_) + depth);

      if (depth > last_depth_) {
         if (last_depth_ + 1 != depth)
            throw std::runtime_error("Forward jump not allowed.");

         // We found the child of the last node pushed on the stack.
         auto* p = new tree_node {line, code};
         stack_.top()->children.push_front(p);
         stack_.push(p);
         ++last_depth_;
      } else if (depth < last_depth_) {
         // Now we have to pop that number of nodes from the stack
         // until we get to the node that is should be the parent of
         // the current line.
         auto const delta_depth = last_depth_ - depth;
         for (auto i = 0; i < delta_depth; ++i)
            stack_.pop();

         stack_.pop();

         // Now we can add the new node.
         auto* p = new tree_node {line, code};
         stack_.top()->children.push_front(p);
         stack_.push(p);

         last_depth_ = depth;
      } else {
         stack_.pop();
         auto* p = new tree_node {line, code};
         stack_.top()->children.push_front(p);
         stack_.push(p);
         // Last depth stays equal.
      }
   }
};

std::pair<tree_node, int>
parse_tree(std::string const& tree_str, oconfig const& cfg)
{
   // TODO: Make it exception safe.
   std::stringstream ss(tree_str);
   std::string line;
   tree_parser p {1000};
   while (std::getline(ss, line, cfg.line_break))
      p.add_line(std::move(line), cfg);

   return std::make_pair(p.head(), p.max_depth());
}

} // tsvtree
