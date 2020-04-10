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

#include <iostream>
#include <random>

char const table[] = {"abcdefghijklmopqrstuv"};

template <class R>
auto make_random_text(R r, int size)
{
   std::string s;
   for (auto i = 0; i < size; ++i) s += r();
   return s;
}

template <class R>
auto make_random_indent(R r, int last)
{
   if (last < 0)
      return 0;

   if (last < 1)
      return 1;

   auto const d = 2 * 10 * r();
   if (d > last && d > 10)
      return last + 1;

   if ((d + 10) > last)
      return last;

   if (d < 2)
      return 1;

   if ((d + 5) > last)
      return last - 1;

   return int(d);
}

int main(int argc, char* argv[])
{
   int length = 10;
   if (argc > 1)
      length = std::stoi(argv[1]);

   std::mt19937 gen(1);

   std::uniform_real_distribution<double> indent_dis(0, 1);
   auto f = [&]()
      { return indent_dis(gen); };

   std::uniform_int_distribution<int> words_dis('a', 'z');
   auto g = [&]()
      { return words_dis(gen); };

   auto last = -1;
   for (auto i = 0; i < length; ++i) {
      auto const indent = make_random_indent(f, last);
      std::cout
      << indent << '\t'
      << make_random_text(g, 7) << '\n';
      last = indent;
   }
}
