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

#include "utils.hpp"
#include "tsv.hpp"

#include <fmt/format.h>

namespace tsvtree
{

auto
remove_depth(std::string& line,
             tree::config::format ifmt,
             char field_sep)
{
   if (std::empty(line))
      return -1;

   if (ifmt == tree::config::format::tabs) {
      auto const i = line.find_first_not_of('\t');
      if (i == std::string::npos)
         throw std::runtime_error("Invalid line.");

      line.erase(0, i);
      return static_cast<int>(i);
   }

   if (ifmt == tree::config::format::counter) {
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

tree::config::format
detect_iformat(std::string const& tree_str,
               char line_break,
               char field_sep,
               bool tsv)
{
   if (tsv)
      return tree::config::format::tsv;

   auto const line = first_line(tree_str, line_break);

   auto const n =
      std::count(std::cbegin(line),
                 std::cend(line),
                 field_sep);
   if (n > 0)
      return tree::config::format::counter;

   return tree::config::format::tabs;
}

class tree_parser {
private:
   std::vector<int> codes_;
   std::stack<tree::node*> stack_;
   int last_depth_ = 0;
   tree::node head_;

public:
   tree_parser(int max_depth) : codes_(max_depth, -1) { }
   auto head() const {return head_;};
   void add_line(std::string line, tree::config const& cfg)
   {
      auto const depth =
         remove_depth(line,
                      cfg.fmt,
                      cfg.field_sep);

      if (depth == -1)
         return;

      if (std::empty(head_.children)) {
         auto* p = new tree::node {line, {}};
         head_.children.push_front(p);
         stack_.push(p);
         return;
      }

      if (depth == 0)
         throw std::runtime_error("Unknown file input format.");

      if (ssize(codes_) <= depth)
         return; // Line is ignored.

      ++codes_.at(depth - 1);
      for (auto i = depth; i < ssize(codes_); ++i)
         codes_[i] = -1;

      std::vector<int> const code { std::cbegin(codes_)
                                  , std::cbegin(codes_) + depth};
      if (depth > last_depth_) {
         if (last_depth_ + 1 != depth)
            throw std::runtime_error("Forward Jump not allowed.");

         // We found the child of the last node pushed on the stack.
         auto* p = new tree::node {line, code};
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
         auto* p = new tree::node {line, code};
         stack_.top()->children.push_front(p);
         stack_.push(p);

         last_depth_ = depth;
      } else {
         stack_.pop();
         auto* p = new tree::node {line, code};
         stack_.top()->children.push_front(p);
         stack_.push(p);
         // Last depth stays equal.
      }
   }
};

// Parses the three contained in tree_str and puts its root node in
// root.children.
auto
parse_tree(std::string const& tree_str,
           tree::config const& cfg)
{
   // TODO: Make it exception safe.
   std::stringstream ss(tree_str);
   std::string line;
   tree_parser p {1000};
   while (std::getline(ss, line, cfg.line_break))
      p.add_line(std::move(line), cfg);

   return p.head();
}

tree::tree(std::string const& str, config const& cfg)
{
   // TODO: Catch exceptions and release already acquired memory.
   head_ = parse_tree(str, cfg);
}

auto node_leaf_counter(tree::node const& node)
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

   tree_post_order_view view {head_.children.front()};
   std::for_each(std::cbegin(view), std::cend(view), f);
}

auto const* tikz_node = "\\treenode[fill={0}!50] ({1}) at ({2}, {3}) {{{4}}};";
auto const* tikz_arrow = "\\treearrow[color={0}] ({1}.west) to ({2}, {3}) to ({4}.south west);";

std::vector<std::string> const& node_colors =
{ "Apricot"
, "Mulberry"
, "RoyalPurple"
, "SpringGreen"
, "Cyan"
, "BurntOrange"
, "CadetBlue"
, "Maroon"
, "Orchid"
, "PineGreen"
, "Aquamarine"
, "RedViolet"
, "Bittersweet"
, "Rhodamine"
//, "Black"
, "Cerulean"
, "Red"
, "Brown"
, "YellowOrange"
, "RubineRed"
, "Peach"
, "Turquoise"
, "JungleGreen"
, "RawSienna"
, "ForestGreen"
, "MidnightBlue"
, "Mahogany"
, "VioletRed"
, "LimeGreen"
, "Salmon"
, "YellowGreen"
, "Magenta"
, "Plum"
, "WildStrawberry"
, "Orange"
, "Green"
, "Sepia"
, "Gray"
, "DarkOrchid"
, "CornflowerBlue"
, "Yellow"
, "RoyalBlue"
, "Periwinkle"
, "BlueGreen"
, "GreenYellow"
, "Thistle"
, "Emerald"
, "SeaGreen"
, "NavyBlue"
, "CarnationPink"
, "Lavender"
, "BrickRed"
, "Melon"
, "Fuchsia"
, "Purple"
, "Blue"
, "RedOrange"
, "SkyBlue"
, "OrangeRed"
, "Violet"
, "Goldenrod"
, "OliveGreen"
, "ProcessBlue"
, "TealBlue"
, "White"
, "BlueViolet"
, "Dandelion"
, "Tan"
};


auto
node_dump(tree::node const& node,
          tree::config::format of,
          char field_sep,
          std::vector<bool> const& lasts,
	  int line)
{
   auto const depth = std::size(node.code);

   if (of == tree::config::format::tabs) {
      std::string ret(depth, '\t');
      ret += node.name;
      return ret;
   }

   if (of == tree::config::format::counter) {
      std::string ret;
      ret += std::to_string(depth);
      ret += field_sep;
      ret += node.name;
      return ret;
   }

   if (of == tree::config::format::deco) {
      auto ret = make_deco_indent(depth, lasts);
      ret += node.name;
      return ret;
   }

   if (of == tree::config::format::tikz) {
      auto const name = "n" + to_string(node.code, '-');
      auto const x = std::size(node.code);
      auto y = - line * 0.45;
      auto const color_idx = x % std::size(node_colors);
      auto const color = node_colors[color_idx];
      auto node_line = fmt::format(tikz_node, color, name, x, y, node.name);
      if (x != 0) {
	 auto const parent_code =
            std::vector<int>{std::begin(node.code), std::prev(std::end(node.code))};
	 auto const parent_name = "n" + to_string(parent_code, '-');
	 y += 0.3;
	 node_line += "\n";
         node_line += fmt::format(tikz_arrow, color, name, x - 1, y, parent_name);
      }
      return node_line;
   }

   return to_string(node.code);
}

std::string
serialize(tree& t,
          tree::config::format of,
          char line_break,
          int const max_depth,
          char field_sep)
{
   tree_tsv_view view {t, max_depth};

   std::string ret;
   int line = 0;
   for (auto iter = std::begin(view); iter != std::end(view); ++iter) {
      ret += node_dump(*iter, of, field_sep, iter.lasts(), line++);
      ret += line_break;
   }
   return ret;
}

std::vector<tree::node*>
check_leaf_min_depths(tree& m, int min_depth)
{
   tree_level_view view {m, min_depth};
   for (auto iter = std::begin(view); iter != std::end(view); ++iter)
      if (iter.depth() < min_depth)
         return iter.line();

   return {};
}

std::string
join(std::vector<tree::node*> const& line, char field_sep)
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

tree::~tree()
{
   tree_post_order_view view {*this};
   for (auto iter = std::begin(view); iter != std::end(view); ++iter)
      delete iter.line().back();
}

// Returns a vector containing all parents of a leaf node. Represents
// a line in the tsv file.
line_type parents(std::deque<std::deque<tree::node*>> const& st)
{
   line_type r;
   for (auto iter = std::cbegin(st); iter != std::cend(st); ++iter)
      if (!std::empty(*iter))
         r.push_back(iter->back());

   return r;
}

//-------------------------------------------------------------------
tree_post_order_traversal::
tree_post_order_traversal(tree::node* root, int depth)
: depth_(depth)
, lasts_(depth > 1000 ? 1000 : depth)
{
   if (root)
      st_.push_back({root});
}

line_type tree_post_order_traversal::advance()
{
   while (!std::empty(st_.back().back()->children) && (ssize(st_) <= depth_))
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
tree_tsv_traversal::tree_tsv_traversal(tree::node* root, int depth)
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

   if (!std::empty(line.back()->children) && ssize(st_) <= depth_)
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

//-------------------------------------------------------------------

}

