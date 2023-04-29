.ds TITLE SCC Coloring Algorithm - C vs Rust

.\" cover
.TL
\*[TITLE]
.AU
Ioannis Stefanidis - 9587
.AI
Aristotel University of Thessaloniki
.sp 6p
.C
28 April 2023
.AB no
In this paper I'm going to compare the \fBperformance of the Coloring
Algorithm\fR for finding the Strongly Connected Components (SCC) in a directed
graph, written in \fBC and Rust\fR.
.AE

.\" beginning of document
.SH 1
Introduction
.PP
In a few words, the coloring algorithm for finding strongly connected
components (SCC) is a depth-first search-based algorithm that assigns unique
colors to vertices during the search, and then identifies SCCs based on the
colors assigned.
.LP
The data structure of the graphs and the method of parallelization is discussed
in a previous paper for this assignment \fISCC Coloring Algorithm in parallel
with C\fR
.[
  scc_parallel
.]
\&. In this paper I'm going to compare the fastest implementation of the C code
(OpenCilk) with both safe and unsafe Rust code.

.SH 1
Parallelism in Rust
.PP
Rust has a very good support for parallelism. It has a built-in library for
parallelism called \fCstd::thread\fR and a third party library for parallel
iterators called \fCrayon\fR. In this paper I'm using rayon which uses a
work-stealing scheduler similar to OpenCilk.

.SH 1
Why use unsafe Rust?
.PP
In the C implementation, there are some benign race conditions that do not affect
the result of the algorithm, but are necessary for its speed. These race conditions
were discussed in the \fIColoring\fR section of the original algorithm paper.
.[
  bfs coloring
.]
.LP
In Rust, the borrow checker prevents mutably accessing the same data across
threads without synchronization mechanisms such as locks or atomics. To ensure a
fair comparison between the C and Rust implementations, I first implemented the
algorithm using only safe Rust and then added unsafe blocks where necessary for
performance reasons.

.SH 1
Results
.PP
At the following table we can see all the graphs that were used for testing and
the number of vertices, edges and SCC for each one.
.DS C
.TS
tab(|);
|l|c|c|c|
|l|r|r|r|.
_
Graph's Name          | Vertices |    Edges   |   SCC  
_
_
celegansneural        |      297 |       2345 |      57
foldoc                |    13356 |     120238 |      71
language              |   399130 |    1216334 |    2456
eu-2005               |   862664 |   19235140 |   90768
wiki-topcats          |  1791489 |   28511807 |       1
sx-stackoverflow      |  2601977 |   36233450 |  953658
wikipedia-20060925    |  2983494 |   37269096 |  975731
wikipedia-20061104    |  3148440 |   39383235 | 1040035
wikipedia-20070206    |  3566907 |   45030389 | 1203340
wb-edu                |  9845725 |   57156537 | 4269022
indochina-2004        |  7414866 |  194109311 | 1749052
uk-2002               | 18520486 |  298113762 | 3887634
arabic-2005           | 22744080 |  639999458 | 4000414
uk-2005               | 39459925 |  936364282 | 5811041
_
.TE
.figure Table of graphs used for testing (sorted by the number of edges).
.DE

Now let's take a look for each implementation of the coloring algorithm how much
time was needed to find all the Strongly Connected Components:
.DS C
.TS
decimalpoint(.) nospaces tab(|);
|l|c|c|c|
|l|n|n|n|.
_
Graph's Name       | OpenCilk | Safe Rust | Unsafe Rust
=
celegansneural     |     1.46 |      2.55 |        1.39
foldoc             |     2.73 |      7.16 |        2.58
language           |    29.37 |     93.56 |       44.02
eu-2005            |   192.66 |    483.15 |      208.97
wiki-topcats       |  1601.57 |      5.73 |     1993.44
sx-stackoverflow   |   654.95 |    938.18 |      627.43
wikipedia-20060925 |   2674.3 |  37205.44 |     3584.63
wikipedia-20061104 |  2038.97 |  42293.73 |     2558.36
wikipedia-20070206 |  2314.81 |  63242.29 |     2650.71
wb-edu             | 19273.45 |  37350.83 |    16305.11
indochina-2004     |  8448.95 |  27135.79 |    11044.77
uk-2002            | 20127.72 |  29735.58 |    13231.55
arabic-2005        | 17951.26 |  30642.27 |    13168.97
uk-2005            | 62696.62 | 123428.41 |    51449.39
_
.TE
.figure Table of execution time in milliseconds of each implementation for each
\s[\n[FGPS]]graph (machine used: Macbook Pro M1 16GB 1TB).\s[\n[PS]]
.DE

.LP
In Fig.\n[FigCount] the graph shows the relative execution time of each
implementation (relative to the slowest one) for each graph. We can see that
the slowest implementation is always the safe Rust one and C and unsafe Rust are
competing very closely, with both of them in average finish in 1/3 of the time
that safe Rust needs.
.LP
In graphs like the wikipedia-20060925, wikipedia-20061104 and wikipedia-20070206
the safe Rust implementation is 10+ times slower than the other two. This
implies that a lot of coloring cycles are happening in these graphs and the safe
Rust implementation is paying the cost of the atomic operations and the cloning
of some vectors to avoid race conditions that are happening.

.G1
frame invis ht 2 wid 5 left solid bot solid
#ticks bottom off
label left "Relative Time" left 0.2
#label bottom "Number of edges" left 0.2
grid bottom ticks off from 1 to 14 by 1

draw openCilk solid  color "red"
draw safe     dashed color "brown"
draw unsafe   dashed color "blue"

copy "data/times.d" thru {
  pic A:Frame.Origin + (x_gg($1),0)
  pic B:Frame.Origin + (x_gg($1+0.5), -0.5/3)
  pic line from A to B "$2" ljust aligned below

  maxx = max($3, $4)
  maxx = max(maxx, $5)
  next openCilk at $1, $3/maxx
  next safe   at $1, $4/maxx
  next unsafe  at $1, $5/maxx
}

# legend
lx  = 1.5   # legend x
ly  = 0.9  # legend y
lyg = 0.1  # legend gap
lxg = 0.5   # legend gap

copy until "DONE" thru {
  line color "$3" $2 from lx, $1 to lx+lxg, $1
  $4 size -2 ljust at lx+lxg*1.1, $1
}
ly       solid  red   "OpenCilk"
ly-lyg   dashed brown "SafeRust"
ly-2*lyg dashed blue  "UnsafeRust"
DONE
.G2
.sp 3.5
.DS C
.figure Relative time graph for the parallel implementations.
.DE

.SH 1
Conclusion
.PP
I really wasn't expecting Rust to perform so close to the OpenCilk
implementation. Writing Rust takes more time than writing C, especially when you
need to do something unsafe. But knowing that your code is safe anywhere else
except inside the unsafe blocks is a great feeling. Having all that safety
without sacrificing any performance is even better.

.B1
.CD
The source code for this assignment is available at the rust branch in this repo:
.br
.I "https://github.com/johnstef99/scc_madness"
.DE
.B2
