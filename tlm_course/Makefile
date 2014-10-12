# -----------------------------------------------------------------------------
#  Makefile
#
#  Copyright (C) 2012 imec, glasseem@imec.be 
#  For copyright and disclaimer notice, see "COPYRIGHT" 
#  ----------------------------------------------------------------------------
# Edit this if you add other subdirectories
SUBDIRS = \
		  example1 \
		  example2 \
		  example3 \
		  example4 \
		  example5 \
		  example6 \
		  example7 \
		  example8 \
		  example9 \
		  exercise1 \
		  exercise2 \
		  exercise3 

build:

#default rule for all non-local targets: run make in subdirs
%:
	list='$(SUBDIRS)'; \
	for tst in $$list; do (\
		(cd $$tst && $(MAKE) $@)\
	) done;

%-local:
	#dummy recipe to prevent make local targets in subdirs

