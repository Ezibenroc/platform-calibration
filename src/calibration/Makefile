CC=mpicc
CFLAGS=-I$(IDIR)

LDIR=-I/usr/include/libxml2

LIBS=-lxml2 -lrt

EXEC=calibrate

default: calibrate

calibrate: calibrate.c experiments.c 
	$(CC) -o $@ $^ -I. $(LDIR) $(LIBS)

.PHONY: clean

mpi_run:
	mpirun -np 2 ./$(EXEC)

Analyze.tex: Analyze.Rnw
	#Rscript -e "library(knitr); knit('Report.Rnw')"
	Rscript -e "library(knitr); knit('Analyze.Rnw')"

%.pdf : Analyze.tex
	pdflatex -jobname=$(@:.pdf=) Analyze.tex
	rm Analyze.tex

%.html: %.xml Analysis.Rmd
	Rscript -e "if(require(knitr)==0)suppressMessages(suppressWarnings(install.packages('knitr', repos = 'http://cran.r-project.org'))); \
	            require(knitr); \
                 input = '$<' ; outputhtml = '$@' ; \
                   outputRmd = gsub('.html$$','.Rmd',outputhtml); \
                   cat(outputhtml); \
                   knit('Analysis.Rmd',output=outputRmd) ; \
                   knitr::knit2html(outputRmd);"

run: calibrate Analyze.pdf



clean:
	rm -f *.o *~ $(EXEC)

