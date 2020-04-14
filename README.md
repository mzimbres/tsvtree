## Description

`tsvtree` is a command line tool to analyse TSV (Tab Separated Value) files. Its main functionalities are

* Visualization of the TSV content in a tree-like format.
* Compression and serialization.
* Node information: The number leaf nodes are reachable from an entry and its coordinate in the tree.

The project tries to offer only functionality that is not already available on other command line tools like `awk`, `cut`, `column`, tac etc. It can be seen as a complement to them.

## Showing the tree

Assume a TSV file like the one in `examples/cities.tsv` shown below

```bash
$ column examples/cities.tsv -t -s $'\t'
Brazil   Sao Paulo             Atibaia
Germany  Nordrhein-Westphalen  Düsseldorf
Brazil   Sao Paulo             Sao Paulo
France   Alsacia               Hagenau
Brazil   Rio de Janeiro        Teresópolis
Germany  Baden-würtemberg      Stuttgart
Germany  Baden-würtemberg      Karlsruhe
France   Alsacia               Strasburg
Brazil   Rio de Janeiro        Rio de Janeiro
France   Île-de-France         Sucy-en-Brie
Germany  Nordrhein-Westphalen  Köln
France   Île-de-France         Paris
```
It is difficult to see the hierarchical nature of the data when we look at it in this form, specially if it is not sorted, also, the redundancy is distracting. `tsvtree` can display it in a much more comprehensible tree-like format as shown below

```bash
$ tsvtree examples/cities.tsv
Root
├── Brazil
│   ├── Rio de Janeiro
│   │   ├── Rio de Janeiro
│   │   └── Teresópolis
│   └── Sao Paulo
│       ├── Atibaia
│       └── Sao Paulo
├── France
│   ├── Alsacia
│   │   ├── Hagenau
│   │   └── Strasburg
│   └── Île-de-France
│       ├── Paris
│       └── Sucy-en-Brie
└── Germany
    ├── Baden-würtemberg
    │   ├── Karlsruhe
    │   └── Stuttgart
    └── Nordrhein-Westphalen
        ├── Düsseldorf
        └── Köln
```
For large files it is sometimes useful to restrict the output to a certain depth in the tree. As an example let us run `tsvtree` in the `examples/worldcities.comp` file that contains all cities in the world restricting the output depth to 2

### Reading from stdin

When a file is not provided `tsvtree` reads data from standard input, which is very convenient to process the output of standard command line tools. Here are some illustrative examples.

The authors of the last 50 commits on the linux kernel sorted by date
```bash
git log --oneline --pretty="%as%x09%an" -n 50 | tsvtree
Root
├── 2020-04-09
│   └── Masahiro Yamada
├── 2020-04-10
│   ├── Anshuman Khandual
│   ├── Arjun Roy
│   ├── Eric Biggers
│   ├── Jaewon Kim
│   ├── Linus Torvalds
│   ├── Logan Gunthorpe
│   ├── Pali Rohár
│   ├── Roman Gushchin
│   ├── Thomas Gleixner
│   ├── Vasily Averin
│   ├── Xiaoyao Li
│   └── kbuild test robot
├── 2020-04-11
│   ├── Linus Torvalds
│   ├── Sedat Dilek
│   └── Trond Myklebust
└── 2020-04-12
    └── Linus Torvalds
```
Last access time on an arbitrary repository.
```bash
$ find  . -name *.cpp -printf "%Tm\t%Td\t%f\n" | tsvtree
Root
├── 01
│   ├── 06
│   │   ├── redis.cpp
│   │   └── test_clients.cpp
│   └── 09
│       ├── notify-test.cpp
│       └── mms.cpp
├── 02
│   ├── 01
│   │   ├── db.cpp
│   │   └── key-gen.cpp
│   ├── 11
│   │   └── crypto.cpp
│   └── 12
│       ├── mms_session.cpp
│       └── post.cpp
├── 04
│   └── 06
│       └── json.cpp
└── 12
    ├── 18
    │   ├── db_tests.cpp
    │   ├── logger.cpp
    │   ├── net.cpp
    │   └── system.cpp
    ├── 28
    │   ├── notifier.cpp
    │   ├── ntf_session.cpp
    │   └── notify.cpp
    └── 31
        └── sim.cpp

```

### Restricting the depth

```bash
$ tsvtree --tree --depth 2  examples/worldcities.comp
Places
├── Africa
│   ├── Northern Africa
│   ├── Eastern Africa
│   ├── Middle Africa
│   ├── Southern Africa
│   └── Western Africa
├── America
│   ├── Caribbean
│   ├── Central America
│   ├── South America
│   └── Northern America
├── Asia
│   ├── Central Asia
│   ├── Eastern Asia
│   ├── South-eastern Asia
│   ├── Southern Asia
│   └── Western Asia
├── Europe
│   ├── Northern Europe
│   ├── Eastern Europe
│   ├── Southern Europe
│   └── Western Europe
└── Oceania
    ├── Autralia and New Zealand
    ├── Melanesia
    ├── Micronesia
    └── Polynesia
```
The `.comp` format will be explained below.

## Leaf counters and node coordinate

Once the tree in `.comp` format is available it is possible to show some useful information about the nodes at a specific depth, such as the number of leaf nodes that are reachable from each node in the tree and their coordinate in the tree. For example

```bash
$ tsvtree --tree --depth 2 --output info examples/worldcities.comp | column -t -s $'\t'
Northern Africa           000.000  99
Eastern Africa            000.001  109
Middle Africa             000.002  70
Southern Africa           000.003  219
Western Africa            000.004  143
Caribbean                 001.000  102
Central America           001.001  226
South America             001.002  11444
Northern America          001.003  366
Central Asia              002.000  42
Eastern Asia              002.001  1358
South-eastern Asia        002.002  380
Southern Asia             002.003  437
Western Asia              002.004  206
Northern Europe           003.000  829
Eastern Europe            003.001  2039
Southern Europe           003.002  1215
Western Europe            003.003  46532
Autralia and New Zealand  004.000  111
Melanesia                 004.001  77
Micronesia                004.002  10
Polynesia                 004.003  14
```
If your TSV file does not have the root node on the left and the leaf nodes on the right, you will have to reorder it. This can be easily achieved with `awk`, for example

```bash
awk -F$'\t' '{print $3":"$2":"$1}' file.tsv
```
will select columns 1, 2, and 3 of `file.tsv` and reverse them.

## Compressing the tree

TSV files like the one above have a lot of redundancy since the nodes at small depths appear many times in the file. With `tsvtree` it is possible to compress the file by using numbers to represent the depth of the tree. The resulting file can be more than five times smaller then the original TSV

```bash
$ tsvtree --output comp examples/cities.tsv 
0	Root
1	Brazil
2	Rio de Janeiro
3	Rio de Janeiro
3	Teresópolis
2	Sao Paulo
3	Atibaia
3	Sao Paulo
1	France
2	Alsacia
3	Hagenau
3	Strasburg
2	Île-de-France
3	Paris
3	Sucy-en-Brie
1	Germany
2	Baden-würtemberg
3	Karlsruhe
3	Stuttgart
2	Nordrhein-Westphalen
3	Düsseldorf
3	Köln
```

The file `examples/worldcities.comp` in the compressed format has the 456 kbytes in size whereas the same data in TSV format has 2064 kbytes, namely more that five times larger. To decompress it again to either TSV or tree format run

```bash
$ tsvtree --tree --output tsv examples/worldcities.comp
$ tsvtree --tree --output tree examples/worldcities.comp
```
Compressing the data in this form with `gzip` or other compressing tools also produces better results than compressing the original TSV directly. 

