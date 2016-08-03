CFLAGS=-I$(IDIR)




.PHONY: clean

%.tex: %.Rnw
	#Rscript -e "library(knitr); knit('Report.Rnw')"
	Rscript -e "library(knitr); knit('$<')"

%.pdf : %.tex
	pdflatex $<

run: LoadAnalyze.pdf



clean:
	rm -f *.o *~ $(EXEC)
	
