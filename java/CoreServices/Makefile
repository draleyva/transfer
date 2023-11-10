include $(SFTDEVELOPMENT)/library/core/build/makesft
PROJECTNAME=CoreServices
S=$(SFTDEVELOPMENT)/library/core/src/$(PROJECTNAME)

CXXSRCS = $(S)/BaseCoreServices.cpp $(S)/Tool.cpp $(S)/ToolToken.cpp $(S)/Option.cpp
LIBOBJS = BaseCoreServices.o Tool.o ToolToken.o Option.o
LIBMODE = static

# STATIC
LIBRARY= $(LIB)/lib$(PROJECTNAME).$(SYS_ARCHIVELIBRARY)

all : $(LIBRARY)
lib : $(LIBRARY)

# STATIC
$(LIBRARY) : $(LIBOBJS)
	ar $(ARCHS) rv $@ $?
	ranlib $(LIBRARY)

rebuildlib : $(LIBOBJS)
	rm -f $(LIBRARY)
	ar $(ARCHS) rcv $(LIBRARY) $(LIBOBJS)
	ranlib $(LIBRARY)
