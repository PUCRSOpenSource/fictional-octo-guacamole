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

tex: doc/base_report.tex
	latexmk -pvc -f doc/base_report.tex

run_seq: sequential
	./$<

.PHONY: clean

clean:
	rm -f sequential
	rm -f parallel
	rm -f optimized
	rm -f base_report.aux
	rm -f base_report.fdb_latexmk
	rm -f base_report.fls
	rm -f base_report.log
	rm -f base_report.pdf
