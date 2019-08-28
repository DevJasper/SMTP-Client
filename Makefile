ARGS=4321
CXX=g++
# CFLAGS= -g -Wall -std=c++11
CXXFLAGS=  -O2 -g -Wall -std=c++17 -lresolv
BINARYDIR=binary
LDLIBS=-lhpdf
# OBJECTDIR=object
# # OBJECTFILES= $(OBJECTDIR)/server.o $(OBJECTDIR)/main.o
# OBJECTFILES=$(addprefix $(OBJECTDIR)/,main.o server.o)
SOURCEDIR=source
SOURCEFILES=$(addprefix $(SOURCEDIR)/,*.$.cpp)

#SOURCEEXT=cpp
# SRCDIR=$(shell find $(SOURCEDIR) -type f -name *.$(SOURCEEXT))


# all: $(OBJECTFILES)
#
# $(OBJECTFILES): $(SOURCEFILES)
# 									${CXX} ${CXXFLAGS} ${HEADERDIR} -c $< -o $@

all: hive

hive:
	${CXX} ${CXXFLAGS} -o ${BINARYDIR}/hive ${SOURCEFILES}

run:
		${BINARYDIR}/hive $(ARGS)
