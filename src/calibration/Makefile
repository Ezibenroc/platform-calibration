CC=mpicc
CFLAGS=-I$(IDIR)

LDIR=

LIBS=-lrt

EXEC=calibrate


ifndef BLAS_INSTALLATION
	BLAS_INSTALLATION=/
endif
BLAS_INCLUDE=$(BLAS_INSTALLATION)/include
BLAS_LIB=$(BLAS_INSTALLATION)/lib

default: calibrate bp_search1 bp_search2

calibrate: calibrate.c experiments.c utils.c 
	$(CC) -o $@ $^ -I. $(LDIR) $(LIBS)

calibrate_blas: calibrate_blas.c utils.c
	gcc -O3 -o $@ $^ -lblas -I $(BLAS_INCLUDE) -L $(BLAS_LIB)

bp_search1: bp_search1.c
	$(CC) -o $@ $^ -I. $(LDIR) $(LIBS)

bp_search2: bp_search2.c
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

