.nr PS 12
.nr FM 0.1i
.nr GROWPS 2
.nr PSINCR 3p
.nr FL \n[LL]
.nr FPS \n[PS]-2
.nr FGPS \n[PS]-2

.OH '-\En[%]-''\E*[TITLE]'
.EH '\E*[TITLE]''-\En[%]-'

.de PT
.ie \\n%=1 .if \\n[pg*P1] .tl \\*[pg*OH]
.el \{\
.	ie o .tl \\*[pg*OH]
.	el .tl \\*[pg*EH]
.\}
.sp -1
\l'6i\[ul]'
..

.nr FigCount 1

.de figure
. ps \n[FGPS]
. nop Figure \\n[FigCount]: \\$^
. ps \n[PS]
.nr FigCount +1
..

.EQ
delim $$
define forall '\[fa]'
define memof  '\[mo]'
.EN

.R1
discard BXYZ
no-accumulate
.R2
