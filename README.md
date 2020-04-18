## Description

`tsvtree` is a command line tool to analyse TSV (Tab Separated Value) files. Its main functionalities are

* Visualization of the TSV content in a tree-like format.
* Compression and serialization.
* Node information: The number leaf nodes are reachable from an entry and its coordinate in the tree.

It can be seen as a complement to text processing tools like `awk`, `cut`, `column`, `tac` etc.

## Showing the tree

Assume a TSV file like the one in `examples/cities.tsv` shown below

```bash
$ column examples/cities.tsv -t -s $'\t'
Earth  Europe   Western Europe  Germany  Baden-Württemberg    Karlsruhe     Karlsruhe                        Bruchsal
Earth  America  South America   Brazil   Sudeste              São Paulo     Macro Metropolitana Paulista     Bragança Paulista  Piracaia     Batatuba
Earth  America  South America   Brazil   Sudeste              São Paulo     Macro Metropolitana Paulista     Bragança Paulista  Atibaia      Alvinópolis
Earth  Europe   Western Europe  Germany  Baden-Württemberg    Karlsruhe     Rastatt                          Bühl
Earth  Europe   Western Europe  Germany  Nordrhein-Westfalen  Arnsberg      Ennepe-Ruhr-Kreis                Gevelsberg
Earth  America  South America   Brazil   Sudeste              São Paulo     Macro Metropolitana Paulista     Bragança Paulista  Atibaia      Jardim Siriema
Earth  Europe   Western Europe  Germany  Baden-Württemberg    Karlsruhe     Karlsruhe                        Karlsbad
Earth  America  South America   Brazil   Sudeste              São Paulo     Metropolitana de São Paulo       São Paulo          São Paulo    Mooca
Earth  America  South America   Brazil   Sudeste              Minas Gerais  Metropolitana de Belo Horizonte  Itabira            Alvinópolis  Alvinópolis
Earth  America  South America   Brazil   Sudeste              São Paulo     Macro Metropolitana Paulista     Bragança Paulista  Atibaia      Vila Santista
Earth  Europe   Western Europe  Germany  Nordrhein-Westfalen  Arnsberg      Ennepe-Ruhr-Kreis                Hattingen
Earth  Europe   Western Europe  France   Grand Est            Elsass        Bas-Rhin                         Strasbourg
Earth  America  South America   Brazil   Sudeste              Minas Gerais  Metropolitana de Belo Horizonte  Itabira            Alvinópolis  Major Ezequiel
Earth  Europe   Western Europe  Germany  Baden-Württemberg    Karlsruhe     Rastatt                          Bühlertal
Earth  Europe   Western Europe  France   Île-de-France        Val-de-Marne  Sucy-en-Brie
Earth  America  South America   Brazil   Sudeste              São Paulo     Metropolitana de São Paulo       São Paulo          São Paulo    Vila Leopoldina
Earth  Europe   Western Europe  France   Grand Est            Elsass        Bas-Rhin                         Haguenau
```
It is difficult to see the hierarchical nature of the data by looking at it in this form, also, the redundancy is distracting. `tsvtree` can display it in a much more comprehensible tree-like format as shown below

```bash
$ tsvtree examples/cities.tsv
Earth
├── America
│   └── South America
│       └── Brazil
│           └── Sudeste
│               ├── Minas Gerais
│               │   └── Metropolitana de Belo Horizonte
│               │       └── Itabira
│               │           └── Alvinópolis
│               │               ├── Alvinópolis
│               │               └── Major Ezequiel
│               └── São Paulo
│                   ├── Macro Metropolitana Paulista
│                   │   └── Bragança Paulista
│                   │       ├── Atibaia
│                   │       │   ├── Alvinópolis
│                   │       │   ├── Jardim Siriema
│                   │       │   └── Vila Santista
│                   │       └── Piracaia
│                   │           └── Batatuba
│                   └── Metropolitana de São Paulo
│                       └── São Paulo
│                           └── São Paulo
│                               ├── Mooca
│                               └── Vila Leopoldina
└── Europe
    └── Western Europe
        ├── France
        │   ├── Grand Est
        │   │   └── Elsass
        │   │       └── Bas-Rhin
        │   │           ├── Haguenau
        │   │           └── Strasbourg
        │   └── Île-de-France
        │       └── Val-de-Marne
        │           └── Sucy-en-Brie
        └── Germany
            ├── Baden-Württemberg
            │   └── Karlsruhe
            │       ├── Karlsruhe
            │       │   ├── Bruchsal
            │       │   └── Karlsbad
            │       └── Rastatt
            │           ├── Bühl
            │           └── Bühlertal
            └── Nordrhein-Westfalen
                └── Arnsberg
                    └── Ennepe-Ruhr-Kreis
                        ├── Gevelsberg
                        └── Hattingen
```
### Reading from stdin

To interact with other text processing tools like the ones mentioned above `tsvtree` can also read from standard input. Here are some illustrative examples.

* The authors of the last 50 commits on the linux kernel sorted by date
```bash
git log --oneline --pretty="%as%x09%an" -n 50 | tsvtree
Master
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
* Last 40 commits on the linux kernel excluding Linus Torvalds sorted by author.

```bash
git log --oneline --pretty="%ci%x09%h%x09%s%x09%an" -n 40 | grep -v Torvalds | awk -F $'\t' '{print $4"\t"$1" "$2" "$3}' | tsvtree
Master
├── Eric Biggers
│   ├── 2020-04-10 15:36:22 -0700 23756e551f35 selftests: kmod: test disabling module autoloading
│   ├── 2020-04-10 15:36:22 -0700 26c5d78c976c fs/filesystems.c: downgrade user-reachable WARN_ONCE() to pr_warn_once()
│   └── 2020-04-10 15:36:22 -0700 d7d27cfc5cf0 kmod: make request_module() return an error when autoloading is disabled
├── Logan Gunthorpe
│   ├── 2020-04-10 15:36:21 -0700 bfeb022f8fe4 mm/memory_hotplug: add pgprot_t to mhp_params
│   ├── 2020-04-10 15:36:21 -0700 c164fbb40c43 x86/mm: thread pgprot_t through init_memory_mapping()
│   └── 2020-04-10 15:36:21 -0700 f5637d3b42ab mm/memory_hotplug: rename mhp_restrictions to mhp_params
│   └── 2020-04-11 11:42:35 -0400 27d231c0c63b pNFS: Fix RCU lock leakage
├── Vasily Averin
│   ├── 2020-04-10 15:36:22 -0700 3bfa7e141b0b fs/seq_file.c: seq_read(): add info message about buggy .next functions
│   ├── 2020-04-10 15:36:22 -0700 89163f93c6f9 ipc/util.c: sysvipc_find_ipc() should increase position index
│   └── 2020-04-10 15:36:22 -0700 f4d74ef6220c kernel/gcov/fs.c: gcov_seq_next() should increase position index
└── Xiaoyao Li
    ├── 2020-04-11 16:40:55 +0200 9de6fe3c28d6 KVM: x86: Emulate split-lock access as a write in emulator
    └── 2020-04-11 16:42:41 +0200 e6f8b6c12f03 KVM: VMX: Extend VMXs #AC interceptor to handle split lock #AC in guest
```
* Last access time on each file of an arbitrary repository in th form `root:month:day`. 

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

For large files it is sometimes useful to restrict the output to a certain depth in the tree. As an example let us run `tsvtree` in the `examples/worldcities.comp` file that contains all cities in the world restricting the output depth to 2

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

