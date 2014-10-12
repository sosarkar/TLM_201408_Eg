# -----------------------------------------------------------------------------
#  Makefile
# 
#  A makefile for SystemC projects
# 
#  Copyright (C) 2012 imec, glasseem@imec.be 
#  For copyright and disclaimer notice, see "COPYRIGHT" 
#  ----------------------------------------------------------------------------
TARGET= example6

SOURCES= main.cpp top.cpp slave.cpp master.cpp 

OBJECTS=$(SOURCES:.cpp=.o)

DEPFILES=$(SOURCES:.cpp=.d) 

SUFFIXES+= .d

CXXFLAGS= \
		  -g -DSC_INCLUDE_DYNAMIC_PROCESSES -DBOOST_NO_0X_HDR_INITIALIZER_LIST \
		  -I${SYSTEMC_HOME}/include

LDFLAGS= \
		 -L${SYSTEMC_HOME}/lib-${LIB_ARCH} \
		 -Wl,-rpath,${SYSTEMC_HOME}/lib-${LIB_ARCH} \
		 -lsystemc -lm 

build: $(TARGET)

run: $(TARGET)
	$<

$(TARGET): $(OBJECTS)
	g++ -g -o $@ $(LDFLAGS) $^

clean:
	rm -rf $(OBJECTS) $(TARGET) 

very_clean: clean
	rm -f $(DEPFILES)

.PHONY: clean very_clean

#dependency generation and inclusion

%.d: %.cpp
	$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$;                 \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

#include dependency files for targets not including clean
ifeq (, $(findstring clean, $(MAKECMDGOALS)))
-include $(DEPFILES)
endif

