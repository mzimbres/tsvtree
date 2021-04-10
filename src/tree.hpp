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

namespace tsvtree
{

class tree {
public:
   struct node {
      std::string name;
      std::vector<int> code;
      int leaf_counter = 0;
      std::deque<node*> children;
   };

   struct config {
     enum class format {tabs, counter, tsv, deco, tikz};
     struct tikz {
        int y_step = 16;
        int x_step = 20;
     };

     char field_sep = '\t';
     char line_break = '\n';
     format fmt = format::tabs;
     tikz tikz_conf;
   };

private:
   node head_;
   int max_depth_ = 0;

   template <int>
   friend class tree_view;

public:
   tree(tree const&) = delete;
   tree& operator=(tree const&) = delete;
   tree(tree&&) = delete;
   tree& operator=(tree&&) = delete;
   tree(std::string const& str, config const& conf);
   ~tree();

   bool empty() const noexcept { return std::empty(head_.children); }
   bool max_depth() const noexcept {return max_depth_;};
   void load_leaf_counters();

   // Returns a pointer to the node at the specified position in the tree where
   // [0] is the root node.
   node* at(std::vector<int> const& coord);

};

// Iterators
// -------------------------------------------------------------------

using line_type = std::vector<tree::node*>;

class tree_post_order_traversal {
private:
   std::deque<std::deque<tree::node*>> st_;
   int depth_;
   std::vector<bool> lasts_;

public:
   tree_post_order_traversal(tree::node* root, int depth);
   auto depth() const noexcept { return ssize(st_) - 1; }
   auto const& lasts() const noexcept { return lasts_;}
   line_type advance();
   line_type next_internal();
   line_type next_leaf_node();
   line_type next_node();
};

// Traverses the tree in the same order as it appears in the tsv file.
class tree_tsv_traversal {
private:
   std::deque<std::deque<tree::node*>> st_;
   int depth_ = -1;
   std::vector<bool> lasts_;

public:
   tree_tsv_traversal(tree::node* root, int depth);
   auto depth() const noexcept { return ssize(st_) - 1; }
   auto const& lasts() const noexcept { return lasts_;}
   line_type next();
   line_type advance();
};

template <int N>
class tree_traversal {
private:
   tree_post_order_traversal t_;

public:
   tree_traversal(tree::node* root, int depth) : t_{root, depth} { }
   auto advance() { return t_.advance();}
   auto depth() const noexcept { return t_.depth(); }
   auto lasts() const noexcept { return t_.lasts(); }

   template <int M = N>
   typename std::enable_if<M == 0, line_type>::type
   next() { return t_.next_leaf_node(); }

   template <int M = N>
   typename std::enable_if<M == 1, line_type>::type
   next() { return t_.next_node(); }
};

template <int N> struct tree_iter_impl    { using type = tree_traversal<N>; };
template <>      struct tree_iter_impl<2> { using type = tree_tsv_traversal; };

template <int N>
class tree_iterator {
private:
   typename tree_iter_impl<N>::type iter_;
   std::vector<tree::node*> current_;

public:
   using value_type = tree::node;
   using difference_type = std::ptrdiff_t;
   using reference = tree::node&;
   using const_reference = tree::node const&;
   using pointer = tree::node*;
   using const_pointer = tree::node const*;
   using iterator_category = std::forward_iterator_tag;

   auto const& impl() const noexcept {return iter_;};

   tree_iterator(tree::node* root = nullptr,
                 int depth = std::numeric_limits<int>::max())
   : iter_ {root, depth}
   { 
      if (root)
         current_ = iter_.advance();
   }

   reference operator*() { return *current_.back();}
   const_reference const& operator*() const { return *current_.back();}

   tree_iterator& operator++()
   {
      current_ = iter_.next();
      return *this;
   }

   tree_iterator operator++(int)
   {
      tree_iterator ret(*this);
      ++(*this);
      return ret;
   }

   pointer operator->()
      {return current_.back();}

   const_pointer operator->() const
      {return current_.back();}

   friend
   auto operator==(tree_iterator const& a, tree_iterator const& b)
   {
      if (std::empty(a.current_) && std::empty(b.current_))
         return true;

      if (std::empty(a.current_) || std::empty(b.current_))
         return false;

      return a.current_.back() == b.current_.back();
   }

   friend
   auto operator!=(tree_iterator const& a, tree_iterator const& b)
      { return !(a == b); }

   auto depth() const noexcept { return iter_.depth(); }
   auto const& line() const noexcept { return current_; }
   auto const& lasts() const noexcept { return iter_.lasts(); }
};

template <int N>
class tree_view {
private:
   int depth_;
   tree::node* node_ = nullptr;

public:
   using iterator = tree_iterator<N>;

   tree_view(tree::node* root, int depth = std::numeric_limits<int>::max())
   : depth_ {depth}
   , node_ {root}
   { }

   iterator begin() const {return iterator{node_, depth_};}
   iterator end() const {return iterator{};}
};

using tree_level_view = tree_view<0>;
using tree_post_order_view = tree_view<1>;
using tree_tsv_view = tree_view<2>;

// -------------------------------------------------------------------

// Returns the line that does not have the minimun depth.
line_type check_leaf_min_depths(tree::node* p, int min_depth);

std::string join(std::vector<tree::node*> const& line, char field_sep);

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
tree::config::format
detect_iformat(std::string const& tree_str,
               char line_break,
               char field_sep,
               bool tsv);

std::string
serialize(tree::node* p,
          tree::config::format of,
          char line_sep,
          int max_depth,
          char field_sep,
	  tree::config::tikz const& conf = {});


}

