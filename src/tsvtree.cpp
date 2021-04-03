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

#include <stack>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iterator>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "tsv.hpp"
#include "tree.hpp"
#include "utils.hpp"
#include "config.h"

namespace tsvtree {

namespace print
{

struct node_info {
   char sep = '\t';
   void operator()(tree::node const& node) const
   {
      std::cout
      << node.name << sep
      << to_string(node.code) << sep
      << node.leaf_counter << "\n";
   }
};

}

struct options {
   int oformat;
   int depth;

   char in_field_sep = '\t';
   char in_line_break = '\n';

   int out_indent = 5;
   char out_line_break = '\n';
   char out_field_sep = '\t';

   std::string file;
   bool check_leaf_min_depth = false;
   bool exit = false;
   bool tsv = true;
   bool decorate_tree = true;

   tree::config::tikz tikz_conf;

   auto
   make_tree_cfg(std::string const& content,
                 bool tsv_arg) const
   {
      auto const fmt =
         detect_iformat(content,
                        in_line_break,
                        in_field_sep,
                        tsv_arg);

      return tree::config
      { in_field_sep
      , in_line_break
      , fmt};
   }

   auto make_tsv_cfg() const
   {
      return tsv_cfg
      { out_indent
      , in_field_sep
      , out_field_sep
      , out_line_break
      , decorate_tree};
   }
};

auto
to_channels(tree_elem const& elem,
            tree::config const& cfg)
{
   tree t {elem.data, cfg};
   tree_level_view view {t, elem.depth};

   auto f = [](auto const& o)
      { return o.code; };

   std::vector<std::vector<int>> channels;
   std::transform(std::cbegin(view),
                  std::cend(view),
                  std::back_inserter(channels),
                  f);

   return channels;
}

auto readfile(std::string const& file)
{
   using iter_type = std::istreambuf_iterator<char>;

   if (std::empty(file))
      return std::string {iter_type {std::cin}, {}};

   std::ifstream ifs(file);
   return std::string {iter_type {ifs}, {}};
}

auto to_oformat(int i)
{
   if (i == 1) return tree::config::format::tabs;
   if (i == 2) return tree::config::format::counter;
   if (i == 7) return tree::config::format::deco;
   if (i == 8) return tree::config::format::tikz;

   throw std::runtime_error("to_oformat: Invalid input.");
   return tree::config::format::tabs;
}

struct tree_info {
   std::string file;
   int depth = 0;
   int version = 0;
};

/*  Converts the input in the form
 *
 *    file:depth:version
 *
 *  to the struct tree_info.
 */
tree_info decode_tree_info(std::string const& data, char sep = ':')
{
   if (std::count(std::cbegin(data), std::cend(data), sep) == 0)
      return {data, std::numeric_limits<int>::max(), 0};

   if (std::size(data) < 5)
      throw std::runtime_error("Invalid name length (n < 5).");

   if (data.front() == sep)
      throw std::runtime_error("Invalid tree info.");

   if (data.back() == sep)
      throw std::runtime_error("Invalid tree info.");

   auto const p1 = data.find_first_of(sep);

   if (p1 == std::string::npos)
      return {data, std::numeric_limits<int>::max(), 0};

   auto const p2 = data.find_first_of(sep, p1 + 1);

   if (p2 == p1 + 1)
      throw std::runtime_error("Invalid tree info.");

   if (p2 == std::string::npos)
      return { data.substr(0, p1)
             , std::stoi(data.substr(p1 + 1))
             , 0};
   
   return { data.substr(0, p1)
          , std::stoi(data.substr(p1 + 1))
          , std::stoi(data.substr(p2 + 1))};
}

auto to_tree_elem(options const& op)
{
   using iter_type = std::istreambuf_iterator<char>;

   auto const info = decode_tree_info(op.file);

   std::ifstream ifs(info.file);
   std::string tree_str {iter_type {ifs}, {}};

   auto const cfg = op.make_tree_cfg(tree_str, op.tsv);
   tree t {tree_str, cfg};

   auto const raw = 
      serialize(t,
                tree::config::format::counter,
                op.out_line_break,
                std::numeric_limits<int>::max(),
                op.out_field_sep,
		op.tikz_conf);

   return tree_elem {raw, info.depth, info.version};
}

auto op6(options const& op)
{
   auto const info = decode_tree_info(op.file);
   auto const str = readfile(info.file);
   tree t {str, op.make_tree_cfg(str, op.tsv)};

   tree_level_view view {t, op.depth};
   for (auto iter = std::begin(view); iter != std::end(view); ++iter) {
      auto const& line = iter.line();
      auto const ret = join(line, op.out_field_sep);
      if (!std::empty(ret))
         std::cout << ret << std::endl;
   }

   return 0;
}

auto op3(options const& op)
{
   auto const info = decode_tree_info(op.file);
   auto content = readfile(info.file);

   if (op.tsv) {
      auto const cfg = op.make_tsv_cfg();
      content = make_tree_string(content, cfg);
   }

   auto const cfg = op.make_tree_cfg(content, false);
   tree t {content, cfg};
   t.load_leaf_counters();

   tree_level_view view {t, op.depth};

   std::for_each(std::cbegin(view),
                 std::cend(view),
                 print::node_info{op.out_field_sep});

   return 0;
}

auto op1(options const& op)
{
   auto const info = decode_tree_info(op.file);
   auto const content = readfile(info.file);

   if (op.tsv) {
      auto const cfg = op.make_tsv_cfg();
      auto const out = make_tree_string(content, cfg);
      std::cout << out << std::flush;
      return 0;
   }

   auto const cfg = op.make_tree_cfg(content, op.tsv);
   tree t {content, cfg};
   auto const format = to_oformat(op.oformat);

   auto const out =
      serialize(t,
                format,
                op.out_line_break,
                op.depth,
                op.out_field_sep,
		op.tikz_conf);

   if (format == tree::config::format::tikz) {
      std::cout <<
      "\\documentclass[11pt]{article}\n"
      "\\usepackage{graphics}\n"
      "\\usepackage[dvipsnames]{xcolor}\n"
      "\\usepackage{tikz}\n"
      "\\usetikzlibrary{positioning}\n"
      "\\usetikzlibrary{arrows}\n"
      "\n"
      "\\colorlet{memC}{Apricot}\n"
      "\\colorlet{arrowC}{black!70!white}\n"
      "\\colorlet{docC}{RoyalBlue!50}\n"
      "\\colorlet{textC}{black!80}\n"
      "\\tikzstyle{treenode}=[anchor=south west, rounded corners=2pt, node distance=0pt, fill=memC,shape=rectangle,minimum height=12pt, minimum width=0pt, inner sep=3pt]\n"
      "\\tikzstyle{textnode}=[anchor=south west, rounded corners=2pt, node distance=0pt, shape=rectangle,minimum height=0.0cm  ,minimum width=0.5cm  ,inner sep=2pt]\n"
      "\\tikzstyle{marrow}=[very thick, densely dotted,>=stealth,->, color=black]\n"
      "\\tikzstyle{treearrow}=[rounded corners=8pt, very thick, >=stealth,<-, color=arrowC]\n"
      "\\def\\treenode{\\node[style=treenode]}\n"
      "\\def\\textnode{\\node[style=textnode]}\n"
      "\\def\\marrow{\\draw[style=marrow]}\n"
      "\\def\\treearrow{\\draw[style=treearrow]}\n"
      "\n"
      "\\pgfrealjobname{tree}\n"
      "\n"
      "\\begin{document}\n"
      "\\beginpgfgraphicnamed{tree-f0}\n"
      "   \\begin{tikzpicture}[scale=1.0]\n"
      "\\fill[color=docC] (-1,4) rectangle (24, -33);\n"
      "%\\shade[left color=BlueViolet!50,right color=BlueViolet!10] (-1,3) rectangle +(15,-15);\n"
      "\\textnode at (4, 3) {\\huge\\bf tsvtree};\n";
   }

   std::cout << out << std::flush;

   if (format == tree::config::format::tikz) {
      std::cout <<
      "\\end{tikzpicture}\n"
      "\\endpgfgraphicnamed\n"
      "\\end{document}\n";
   }

   return 0;
}

int check_leaf_min_depth_op(options const& op)
{
   auto const info = decode_tree_info(op.file);
   auto const str = readfile(info.file);
   auto const cfg = op.make_tree_cfg(str, op.tsv);

   tree t {str, cfg};

   auto const line = check_leaf_min_depths(t, op.depth);
   auto const out = join(line, op.out_field_sep);
   if (std::empty(out)) {
      std::cout << "Ok" << std::endl;
      return 0;
   }

   std::cout << "Error on line: " << out << std::endl;
   return 1;
}

int impl(options const& op)
{
   if (op.check_leaf_min_depth)
      return check_leaf_min_depth_op(op); 

   if (op.oformat == 1) return op1(op);
   if (op.oformat == 2) return op1(op);
   if (op.oformat == 3) return op3(op);
   if (op.oformat == 6) return op6(op);
   if (op.oformat == 7) return op1(op);
   if (op.oformat == 8) return op1(op);

   return 1;
}

}

namespace po = boost::program_options;
using namespace tsvtree;

auto make_oformat(std::string const& s, bool deco)
{
   if (s == "tree" && deco)     return 7;
   if (s == "tree")             return 1;
   if (s == "comp")             return 2;
   if (s == "info")             return 3;
   if (s == "tsv")              return 6;
   if (s == "tikz")             return 8;
   return -1;
}

auto parse_options(int argc, char* argv[])
{
   options op;
   std::string oformat = "tree";
   po::options_description desc("Options");
   desc.add_options()
   ( "help,h", "This help message.")
   ( "version,v", "Program version.")
   ( "tree,k", "Input file in tsv format.")
   ( "indent-with-tab,p", "Uses tab to represent the tree depth.")
   ( "depth,d", po::value<int>(&op.depth)->default_value(std::numeric_limits<int>::max()), "Influences the output.")
   ( "file,f", po::value<std::string>(&op.file), "The file containing the tree.")
   ( "input-separator,e", po::value<char>(&op.in_field_sep), "Field separator in the input file.")
   ( "input-line-break,r", po::value<char>(&op.in_line_break), "Line break in the input file.")
   ( "output-line-break,b", po::value<char>(&op.out_line_break), "Line break character used in the output.")
   ( "output-separator,s", po::value<char>(&op.out_field_sep), "Output field separator.")
   ( "check-leaf-min-depth,c","Checks whether all leaf nodes have at least the depth specified in --depth.")
   ( "output,o"
   , po::value<std::string>(&oformat)->default_value("tree")
   , "Format used in the output file. Available options:\n"
     "• tree: \tNode depth with indentation.\n"
     "• comp: \tCompressed tree.\n"
     "• info: \tInfo of nodes at --depth.\n"
     "• tsv:  \tTSV format.\n"
     "• tikz:  \tTikZ format."
   )
   ( "tikz-x-step,x", po::value<int>(&op.tikz_conf.x_step)->default_value(30), "Node horizontal distance in point units.")
   ( "tikz-y-step,y", po::value<int>(&op.tikz_conf.y_step)->default_value(20), "Node vertical distance in point units.")
   ( "tikz-color-min,a", po::value<int>(&op.tikz_conf.min)->default_value(30), "Right color minimum.")
   ( "tikz-color-max,g", po::value<int>(&op.tikz_conf.max)->default_value(60), "Right colot maximum.")
   ( "tikz-right-color,i", po::value<std::string>(&op.tikz_conf.right_color)->default_value("BurntOrange"), "Color of the right nodes.")
   ( "tikz-left-color,j", po::value<std::string>(&op.tikz_conf.left_color)->default_value("White"), "Color of the node left nodes.")
   ;

   po::positional_options_description pos;
   pos.add("file", -1);

   po::variables_map vm;        
   po::store(po::command_line_parser(argc, argv).
      options(desc).positional(pos).run(), vm);
   po::notify(vm);    

   op.decorate_tree = vm.count("indent-with-tab") == 0;
   op.oformat = make_oformat(oformat, op.decorate_tree);

   if (op.oformat == -1) {
      std::cerr << "Invalid output option." << std::endl;
      op.exit = true;
      return op;
   }

   if (vm.count("help")) {
      op.exit = true;
      std::cout << desc << "\n";
      return op;
   }

   if (vm.count("version")) {
      op.exit = true;
      std::cout << PACKAGE_VERSION << "\n";
      return op;
   }

   op.check_leaf_min_depth = vm.count("check-leaf-min-depth") > 1;
   op.tsv = vm.count("tree") == 0;

   if (op.tsv) {
      if (op.oformat == 2) op.out_indent = -1;
      if (op.oformat == 3) op.out_indent = -1;
   }

   return op;
}

int main(int argc, char* argv[])
{
   try {
      auto const op = parse_options(argc, argv);
      if (op.exit)
         return 0;

      return impl(op);
   } catch (std::exception const& e) {
      std::cerr << e.what() << std::endl;
      return 1;
   }
}

