# test parser
# make clean; make DPARSER=1 c_parser
# make clean; make DPARSER=1 
# make clean; make DPARSER=1 parser_4op

# test lexer
# make clean; make DLEXER=1 lexer

CXXFLAGS=-g -std=c++14 -Wall -m32
ifdef DLEXER
CXXFLAGS+= -DDEBUG_LEXER
endif

ifdef DPARSER
CXXFLAGS+= -DDEBUG_PARSER
endif


CXX=g++

OBJS = m.o section.o

#all: 
#	make DLEXER=1 lexer

simple_as: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

lexer: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	

dep: $(OBJS:.o=.d)
include $(OBJS:.o=.d)

%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM -MF $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

	

doc_html:
	make -C doc
clean:
	rm parser lexer *.o *.d ; make -C doc clean
