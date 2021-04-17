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

namespace tsvtree
{

using line_type = std::vector<tree_node*>;

class tree_post_order_traversal {
private:
   std::deque<std::deque<tree_node*>> st_;
   int depth_;
   std::vector<bool> lasts_;

public:
   tree_post_order_traversal(tree_node* root, int depth);
   auto depth() const noexcept { return tsvtree::ssize(st_) - 1; }
   auto const& lasts() const noexcept { return lasts_;}
   line_type advance();
   line_type next_internal();
   line_type next_leaf_node();
   line_type next_node();
};

// Traverses the tree in the same order as it appears in the tsv file.
class tree_tsv_traversal {
private:
   std::deque<std::deque<tree_node*>> st_;
   int depth_ = -1;
   std::vector<bool> lasts_;

public:
   tree_tsv_traversal(tree_node* root, int depth);
   auto depth() const noexcept { return tsvtree::ssize(st_) - 1; }
   auto const& lasts() const noexcept { return lasts_;}
   line_type next();
   line_type advance();
};

template <int N>
class tree_traversal {
private:
   tree_post_order_traversal t_;

public:
   tree_traversal(tree_node* root, int depth) : t_{root, depth} { }
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
   std::vector<tree_node*> current_;

public:
   using value_type = tree_node;
   using difference_type = std::ptrdiff_t;
   using reference = tree_node&;
   using const_reference = tree_node const&;
   using pointer = tree_node*;
   using const_pointer = tree_node const*;
   using iterator_category = std::forward_iterator_tag;

   auto const& impl() const noexcept {return iter_;};

   tree_iterator(tree_node* root = nullptr,
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
   tree_node* node_ = nullptr;

public:
   using iterator = tree_iterator<N>;

   tree_view(tree_node* root, int depth = std::numeric_limits<int>::max())
   : depth_ {depth}
   , node_ {root}
   { }

   iterator begin() const {return iterator{node_, depth_};}
   iterator end() const {return iterator{};}
};

using tree_level_view = tree_view<0>;
using tree_postorder_view = tree_view<1>;
using tree_tsv_view = tree_view<2>;

} // tsvtree
