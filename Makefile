ENVLAD := $(shell command -v ladcomp 2> /dev/null)
RUNLAD := $(shell command -v ladrun 2> /dev/null)

SDIR = ./src

CC = gcc
MPI = mpicc
MPIR = mpirun
LADC = ladcomp
LADR = ladrun

LADFLAGS = -env mpicc
CFLAGS = -Wall -g

all: sequential parallel optimized

sequential: $(SDIR)/sequential.c
	$(CC) -o $@ $< $(CFLAGS)

parallel: $(SDIR)/parallel.c
ifndef ENVLAD
	$(MPI) -o $@ $< $(CFLAGS)
else
	$(LADC) $(LADFLAGS) $< -o $@ $(CFLAGS)
endif

run: parallel
ifndef RUNLAD
ifndef NP
	$(MPIR) $<
else
	$(MPIR) -np ${NP} $<
endif
else
ifndef NP
	$(LADR) $<
else
	$(LADR) -np ${NP} $<
endif
endif

optimized: $(SDIR)/optimized.c
ifndef ENVLAD
	$(MPI) -o $@ $< $(CFLAGS)
else
	$(LADC) $(LADFLAGS) $< -o $@ $(CFLAGS)
endif

run_opt: optimized
ifndef RUNLAD
ifndef NP
	$(MPIR) $<
else
	$(MPIR) -np ${NP} $<
endif
else
ifndef NP
	$(LADR) $<
else
	$(LADR) -np ${NP} $<
endif
endif

tex: doc/report.tex
	latexmk -pvc -f doc/report.tex

run_seq: sequential
	./$<

.PHONY: clean

clean:
	rm -f sequential
	rm -f parallel
	rm -f optimized
	rm -f report.aux
	rm -f report.fdb_latexmk
	rm -f report.fls
	rm -f report.log
