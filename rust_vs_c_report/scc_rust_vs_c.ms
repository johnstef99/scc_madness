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
In the C implementation, there are some benign race conditions that do not
affect the result of the algorithm, but are necessary for its speed. These race
conditions were discussed in the \fIColoring\fR section of the original
algorithm paper.
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
.ps \n[PS]-2
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
twitter7              | 41652230 | 1468365182 | 8044728
_
.TE
.figure Table of graphs used for testing (sorted by the number of edges).
.DE

Now let's take a look for each implementation of the coloring algorithm how much
time was needed to find all the Strongly Connected Components:
.DS C
.ps \n[PS]-2
.TS
decimalpoint(.) nospaces tab(|);
|l|c|c|c|c|
|l|n|n|n|n|.
_
Graph's Name       | Serial Rust | Safe Rust | Unsafe Rust | OpenCilk
=
celegansneural     |       0.046 |      5.01 |        1.94 |     8.43
foldoc             |        3.33 |      8.29 |        5.17 |    17.20
language           |      102.21 |     78.93 |       34.45 |    39.00
eu-2005            |      697.36 |    243.12 |      184.03 |   597.93
wiki-topcats       |    10089.68 |    976.82 |      838.79 |   699.20
sx-stackoverflow   |     1641.96 |    687.07 |      597.65 |  1010.50
wikipedia-20060925 |     9495.09 |   1679.05 |     1527.14 |  3003.81
wikipedia-20061104 |     6851.03 |   1415.65 |     1262.04 |  2840.85
wikipedia-20070206 |     6662.01 |   1573.03 |     1453.00 |  3395.13
wb-edu             |   101018.82 |   8287.11 |     5571.10 | 42638.20
indochina-2004     |    35034.07 |   9237.19 |     7390.41 | 21669.90
uk-2002            |    74700.83 |   7814.78 |     5692.67 | 44417.30
arabic-2005        |    64904.20 |   8340.29 |     6905.64 | 35089.60
uk-2005            |   255174.98 |  30235.00 |    23029.56 | 85217.30
twitter7           |   348015.33 |  49281.95 |    48548.47 | 52441.90
_
.TE
.figure Table of execution time in milliseconds of each implementation for each
\s[\n[FGPS]]graph (CPU: AMD EPYC 7662, Threads: 32).\s[\n[PS]]
.DE

.LP
In Figure \n[FigCount] we can see the speedup for parallel Rust implementations
compared to the serial one. Both safe and unsafe implementations for large
graphs provide more than 5x speedup. Also the safe implementation where we
ensure there are no race conditions isn't that far behind the unsafe one.
Compering Rust with OpenCilk, we can see that the Rust is almost always faster.

.G1
frame invis ht 2 wid 5 left solid bot solid
label left "Speed up" left 0.2
grid bottom ticks off from 2 to 30 by 2

define bar { #(color, x, y, width)
  barx  = $2
  bary  = $3
  width = $4

  line color "$1" from barx, 0 to barx, bary
  line color "$1" from barx, bary to barx+width, bary
  line color "$1" from barx+width, bary to barx+width, 0
}

n_line = 0

copy "data/times.d" thru {
  n_line = n_line + 1
  if n_line > 1 then {
    pic A:Frame.Origin + (x_gg($1*2),0)
    pic B:Frame.Origin + (x_gg($1*2+0.5), -0.5/8)
    pic line from A to B "$2" ljust aligned below

    x = $1*2
    barw = 0.4
    serial = $3
    safe_speedup = serial / $4
    unsafe_speedup = serial / $5
    open_cilk_speedup = serial / $6

    bar(orange, x-barw*1.5 - 0.1, unsafe_speedup,    barw)
    bar(blue,   x-barw*0.5 + 0.0, safe_speedup,      barw)
    bar(green,  x+barw*0.5 + 0.1, open_cilk_speedup, barw)
  }
}

# legend
lx  = 0.5   # legend x
ly  = 16  # legend y
lyg = 1.4 # legend gap
lxg = 0.5 # legend gap

copy until "DONE" thru {
  line color "$3" $2 from lx, $1 to lx+lxg, $1
  $4 size -2 ljust at lx+lxg*1.1, $1
}

ly       solid orange "UnsafeRust"
ly-lyg   solid blue   "SafeRust"
ly-2*lyg   solid green  "OpenCilk"
DONE
.G2
.sp 3.6
.DS C
.figure Speedup for parallel implementations relative to the serial Rust.
.DE

.SH 1
Conclusion
.PP
I really wasn't expecting Rust to outperform the OpenCilk implementation.
Writing Rust takes more time than writing C, especially when you need to do
something unsafe. But knowing that your code is safe anywhere else except inside
the unsafe blocks is a great feeling. Having all that safety without sacrificing
any performance is even better.

.B1
.CD
The source code for this assignment is available at the rust branch in this repo:
.br
.I "https://github.com/johnstef99/scc_madness"
.DE
.B2
