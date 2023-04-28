.SUFFIXES: .jpg .eps .ms .ps .pdf .grap .ref

REFER="$(CURDIR)/bibliography.ref"


all: scc_rust_vs_c.pdf

.jpg.eps:
	convert $< $@

.ms.ps:
	REFER=$(REFER) groff -GeptkR -P-pa4 -ms -Tps $< > $@

.ps.pdf:
	ps2pdf $< > $@

clean:
	rm *.pdf *.eps *.ps
