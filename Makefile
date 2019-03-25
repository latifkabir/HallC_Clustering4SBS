SRCFILES = THcGEM.cc \
           GEMAnalyzer.cc \
	   GEMDataHandler.cc \
	   GEMMapping.cc \
	   GEMConfigure.cc \
	   GEMEvioParser.cc \
	   GEMRawDecoder.cc \
           PRadBenchMark.cc \
	   GEMEventAnalyzer.cc \
           GEMHit.cc \
           GEMCluster.cc \
           GEMRawPedestal.cc \
           GEMPedestal.cc \
           GEMPhysics.cc \
           GEMOnlineHitDecoder.cc \
           GEMTree.cc \
           GEMCoord.cc \
           EpicsEventAnalyzer.cc \
	   EpicsPhysics.cc \
	   TDCEventAnalyzer.cc \
	   EventUpdater.cc \
	   GEMSignalFitting.cc\
           THcGEMDataProcessor.cc\
           THcGEMCluster.cc
#	   main.cc GEMEvioParser.cc

SRC = $(addprefix src/,$(SRCFILES))
BUILD_DIR := lib obj

PACKAGE = GEM

LINKDEF = src/$(PACKAGE)_LinkDef.h

export DEBUG = 1

ARCH = linux

ifndef ANALYZER
  $(error $$ANALYZER environment variable not defined)
endif

INCDIRS  = $(wildcard $(addprefix $(HCANALYZER)/, src)) \
	$(wildcard $(addprefix $(ANALYZER)/, src hana_decode))

#------------------------------------------------------------------------------
# Do not change anything  below here unless you know what you are doing

ifeq ($(strip $(INCDIRS)),)
  $(error No Analyzer header files found. Check $$ANALYZER)
endif

ROOTCFLAGS   := $(shell root-config --cflags)
ROOTLIBS     := $(shell root-config --libs)
ROOTGLIBS    := $(shell root-config --glibs)
ROOTBIN      := $(shell root-config --bindir)
ROOTINC      := -I$(shell root-config --incdir)
CXX          := $(shell root-config --cxx)
CC           := $(shell root-config --cc)

INCLUDES      = $(addprefix -I, $(INCDIRS) ) -I$(shell pwd)

USERLIB       = lib/lib$(PACKAGE).so
USERDICT      = lib/$(PACKAGE)Dict

LIBS          =
GLIBS         =

ifeq ($(ARCH),linux)
# Linux with egcs (>= RedHat 5.2)
ifdef DEBUG
  CXXFLAGS    = -g -O0
  LDFLAGS     = -g -O0
else
  CXXFLAGS    = -O
  LDFLAGS     = -O
endif
CXXFLAGS     += -Wall -Woverloaded-virtual -fPIC
LD            = g++
SOFLAGS       = -shared
endif

CXXFLAGS     += $(ROOTCFLAGS) $(INCLUDES)
LIBS         += $(ROOTLIBS) $(SYSLIBS)
GLIBS        += $(ROOTGLIBS) $(SYSLIBS)

MAKEDEPEND    = gcc

ifdef WITH_DEBUG
CXXFLAGS     += -DWITH_DEBUG
endif

ifdef PROFILE
CXXFLAGS     += -pg
LDFLAGS      += -pg
endif

ifndef PKG
PKG           = lib$(PACKAGE)
LOGMSG        = "$(PKG) source files"
else
LOGMSG        = "$(PKG) Software Development Kit"
endif
DISTFILE      = $(PKG).tar.gz

#------------------------------------------------------------------------------
OBJFILES      = $(SRCFILES:.cc=.o)
OBJ	      = $(addprefix obj/,$(OBJFILES))
HDR           = $(SRC:.cc=.h)
DEP           = $(SRC:.cc=.d)
OBJS          = $(OBJ) $(USERDICT).o

all:		checkdirs $(USERLIB)

checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	@mkdir -p $@

$(USERLIB):	$(HDR) $(OBJS)
		$(LD) $(LDFLAGS) $(SOFLAGS) -o $@ $(OBJS)
		@echo "$@ done"

$(USERDICT).cc: $(HDR) $(LINKDEF)
	@echo "Generating dictionary $(USERDICT)..."
	$(ROOTSYS)/bin/rootcint -f $@ -c $(ROOTINC) $(INCLUDES) $(DEFINES) $^

install:	all
		$(error Please define install yourself)
# for example:
#		cp $(USERLIB) $(LIBDIR)

clean:
		rm -f obj/*.o  *~ $(USERLIB) $(USERDICT).*

realclean:	clean
		rm -f *.d

srcdist:
		rm -f $(DISTFILE)
		rm -rf $(PKG)
		mkdir $(PKG)
		cp -p $(SRC) $(HDR) $(LINKDEF) db*.dat README Makefile $(PKG)
		gtar czvf $(DISTFILE) --ignore-failed-read \
		 -V $(LOGMSG)" `date -I`" $(PKG)
		rm -rf $(PKG)

.PHONY: checkdirs all clean realclean srcdist

.SUFFIXES:
.SUFFIXES: .c .cc .cpp .cxx .C .o .d

obj/%.o: src/%.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $<

# FIXME: this only works with gcc
%.d:	%.cxx
	@echo Creating dependencies for $<
	@$(SHELL) -ec '$(MAKEDEPEND) -MM $(ROOTINC) $(INCLUDES) -c $< \
		| sed '\''s%^.*\.o%$*\.o%g'\'' \
		| sed '\''s%\($*\)\.o[ :]*%\1.o $@ : %g'\'' > $@; \
		[ -s $@ ] || rm -f $@'

###

-include $(DEP)
