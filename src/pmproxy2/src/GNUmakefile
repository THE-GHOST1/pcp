
TOPDIR = ../../..
include $(TOPDIR)/src/include/builddefs

CMDTARGET = pmproxy2$(EXECSUFFIX)
CFILES = CommandKeys.c pmproxy2.c
HFILES = CommandKeys.h


LLDLIBS = $(PCP_WEBLIB)
LCFLAGS += -I$(TOPDIR)/src/libpcp_web/src

LDIRT = pmproxy2.log

default: $(CMDTARGET)

include $(BUILDRULES)

install: default

default_pcp:    default

install_pcp:	install


pmproxy2.o:	pmproxy2.c CommandKeys.h CommandKeys.c

CommandKeys.o pmproxy2.o:	$(TOPDIR)/src/include/pcp/libpcp.h
