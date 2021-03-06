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

struct options {
   oconfig oc;
   int depth;

   char in_field_sep = '\t';
   char in_line_break = '\n';

   int out_indent = 5;
   char out_line_break = '\n';
   char out_field_sep = '\t';

   std::string file;
   std::string at_coord;
   bool exit = false;
   bool tsv = true;
   bool decorate_tree = true;

   auto at() const
   {
     auto const coord = split_line(at_coord, ':');
     auto f = [](auto const& in)
       { return std::stoi(in); };

     std::vector<int> ret;
     std::transform(std::cbegin(coord), std::cend(coord), std::back_inserter(ret), f);
     return ret;
   }

   auto
   make_tree_cfg(std::string const& content,
                 bool tsv_arg) const
   {
      auto const fmt =
         detect_iformat(content,
                        in_line_break,
                        in_field_sep,
                        tsv_arg);

      return oconfig
      { in_field_sep
      , in_line_break
      , fmt
      , {}};
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
to_channels(std::string const& data,
            oconfig const& cfg,
            int depth)
{
   tree t {data, cfg};
   auto view = t.level_view({0}, depth);

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

int op_tsv(options const& op)
{
   auto const str = readfile(op.file);
   tree t {str, op.make_tree_cfg(str, op.tsv)};

   auto view = t.level_view(op.at(), op.depth);
   for (auto iter = std::begin(view); iter != std::end(view); ++iter) {
      auto const& line = iter.line();
      auto const ret = join(line, op.out_field_sep);
      if (!std::empty(ret))
         std::cout << ret << std::endl;
   }

   return 0;
}

auto op_info(options const& op)
{
   auto content = readfile(op.file);

   if (op.tsv) {
      auto const cfg = op.make_tsv_cfg();
      content = make_tree_string(content, cfg);
   }

   auto const cfg = op.make_tree_cfg(content, false);
   tree t {content, cfg};
   t.load_leaf_counters();

   auto view = t.tsv_view(op.at(), op.depth);

   auto f =  [&](auto const& node)
   {
     std::cout
        << node.name
        << op.out_field_sep
        << to_string(node.code)
        << op.out_field_sep
        << node.leaf_counter
        << "\n";
   };

   std::for_each(std::cbegin(view), std::cend(view), f);

   return 0;
}

auto op1(options const& op)
{
   auto const content = readfile(op.file);

   if (op.tsv) {
      auto const cfg = op.make_tsv_cfg();
      auto const out = make_tree_string(content, cfg);
      std::cout << out << std::flush;
      return 0;
   }

   auto const cfg = op.make_tree_cfg(content, op.tsv);
   tree t {content, cfg};
   auto const coord = op.at();
   auto* node = t.at(coord);

   auto const out =
      serialize(node,
                op.oc.fmt,
                op.out_line_break,
                op.depth,
                tsvtree::ssize(coord),
                op.out_field_sep,
		op.oc.tikz_conf);

   if (op.oc.fmt == oconfig::format::tikz) {
      std::cout <<
      "\\documentclass[11pt]{article}\n"
      "\\usepackage{graphics}\n"
      "\\usepackage[dvipsnames]{xcolor}\n"
      "\\usepackage{tikz}\n"
      "\\usetikzlibrary{positioning}\n"
      "\\usetikzlibrary{arrows}\n"
      "\n"
      "\\colorlet{nodeColor}{RoyalBlue!20}\n"
      "\\colorlet{depthC0}{nodeColor}\n"
      "\\colorlet{depthC1}{nodeColor}\n"
      "\\colorlet{depthC2}{nodeColor}\n"
      "\\colorlet{depthC3}{nodeColor}\n"
      "\\colorlet{depthC4}{nodeColor}\n"
      "\\colorlet{depthC5}{nodeColor}\n"
      "\\colorlet{depthC6}{nodeColor}\n"
      "\\colorlet{depthC7}{nodeColor}\n"
      "\\colorlet{depthC8}{nodeColor}\n"
      "\\colorlet{depthC9}{nodeColor}\n"
      "\\colorlet{treenodeC}{Apricot}\n"
      "\\colorlet{arrowC}{black!70!white}\n"
      "\\colorlet{docC}{black!5}\n"
      "\\colorlet{textC}{black!80}\n"
      "\\tikzstyle{treenode}=[draw, very thin, anchor=south west, rounded corners=2pt, node distance=0pt, fill=treenodeC,shape=rectangle,minimum height=12pt, minimum width=0pt, inner sep=3pt]\n"
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
      "\\textnode at (0, 1) {\\huge\\bf\\sc tsvtree};\n";
   }

   std::cout << out << std::flush;

   if (op.oc.fmt == oconfig::format::tikz) {
      std::cout <<
      "\\end{tikzpicture}\n"
      "\\endpgfgraphicnamed\n"
      "\\end{document}\n";
   }

   return 0;
}

int check_min_depth_op(options const& op)
{
   auto const str = readfile(op.file);
   auto const cfg = op.make_tree_cfg(str, op.tsv);

   tree t {str, cfg};
   auto view = t.level_view(op.at());
   auto const out = check_min_depth(view, op.depth);

   if (std::empty(out)) {
      std::cout << "Ok" << std::endl;
      return 0;
   }

   std::cout << "Error on line: " << out << std::endl;
   return 1;
}

int impl(options const& op)
{
   switch (op.oc.fmt) {
     case oconfig::format::check_min_depth: return check_min_depth_op(op); 
     case oconfig::format::tree: return op1(op);
     case oconfig::format::comp: return op1(op);
     case oconfig::format::info: return op_info(op);
     case oconfig::format::tsv: return op_tsv(op);
     case oconfig::format::tree_deco: return op1(op);
     case oconfig::format::tikz: return op1(op);
     default: {
       throw std::runtime_error("Invalid input format.");
       return 1;
     }
   }
}

}

namespace po = boost::program_options;
using namespace tsvtree;

auto make_oformat(std::string const& s, bool deco, bool check_min_depth)
{
   if (check_min_depth) return oconfig::format::check_min_depth;
   if (s == "tree" && deco) return oconfig::format::tree_deco;
   if (s == "tree") return oconfig::format::tree;
   if (s == "comp") return oconfig::format::comp;
   if (s == "info") return oconfig::format::info;
   if (s == "tsv") return oconfig::format::tsv;
   if (s == "tikz") return oconfig::format::tikz;
   return oconfig::format::invalid;
}

auto parse_options(int argc, char* argv[])
{
   options op;
   std::string of = "tree";
   po::options_description desc("Options");
   desc.add_options()
   ( "help,h", "This help message.")
   ( "version,v", "Program version.")
   ( "tree,k", "Input file in tsv format.")
   ( "indent-with-tab,p", "Uses tab to represent the tree depth.")
   ( "at,a", po::value<std::string>(&op.at_coord)->default_value("0"), "Node coordinate.")
   ( "depth,d", po::value<int>(&op.depth)->default_value(std::numeric_limits<int>::max()), "Influences the output.")
   ( "file,f", po::value<std::string>(&op.file), "The file containing the tree.")
   ( "input-separator,e", po::value<char>(&op.in_field_sep), "Field separator in the input file.")
   ( "input-line-break,r", po::value<char>(&op.in_line_break), "Line break in the input file.")
   ( "output-line-break,b", po::value<char>(&op.out_line_break), "Line break character used in the output.")
   ( "output-separator,s", po::value<char>(&op.out_field_sep), "Output field separator.")
   ( "check-min-depth,c","Checks whether all leaf nodes have at least the depth specified in --depth.")
   ( "output,o"
   , po::value<std::string>(&of)->default_value("tree")
   , "Format used in the output file. Available options:\n"
     "• tree: \tNode depth with indentation.\n"
     "• comp: \tCompressed tree.\n"
     "• info: \tInfo of nodes at --depth.\n"
     "• tsv:  \tTSV format.\n"
     "• tikz:  \tTikZ format."
   )
   ( "tikz-x-step,x", po::value<int>(&op.oc.tikz_conf.x_step)->default_value(30), "Node horizontal distance in point units.")
   ( "tikz-y-step,y", po::value<int>(&op.oc.tikz_conf.y_step)->default_value(20), "Node vertical distance in point units.")
   ;

   po::positional_options_description pos;
   pos.add("file", -1);

   po::variables_map vm;        
   po::store(po::command_line_parser(argc, argv).
      options(desc).positional(pos).run(), vm);
   po::notify(vm);    

   op.decorate_tree = vm.count("indent-with-tab") == 0;
   auto const check_min_depth = vm.count("check-min-depth") > 0;
   op.oc.fmt = make_oformat(of, op.decorate_tree, check_min_depth);

   if (op.oc.fmt == oconfig::format::invalid) {
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

   op.tsv = vm.count("tree") == 0;

   if (op.tsv) {
      if (op.oc.fmt == oconfig::format::comp) op.out_indent = -1;
      if (op.oc.fmt == oconfig::format::info) op.out_indent = -1;
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

