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

template <class R>
auto make_random_text(R r, int size)
{
   std::string s;
   for (auto i = 0; i < size; ++i) s += r();
   return s;
}

int main(int argc, char* argv[])
{
   int lines = 10;
   int depth = 10;
   int word_length = 4;
   int seed = 1;

   if (argc > 1) lines = std::stoi(argv[1]);
   if (argc > 2) depth = std::stoi(argv[2]);
   if (argc > 3) word_length = std::stoi(argv[3]);
   if (argc > 4) seed = std::stoi(argv[4]);

   std::mt19937 gen(seed);

   std::uniform_int_distribution<int> words_dis('a', 'a' + word_length);
   auto g = [&]()
      { return words_dis(gen); };

   for (auto i = 0; i < lines; ++i) {
      for (auto i = 0; i < depth; ++i)
         std::cout << make_random_text(g, word_length) << '\t';
      std::cout << '\n';
   }
}
