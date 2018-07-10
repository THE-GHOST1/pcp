#
# Copyright (c) prajwal
#
# This library is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation; either version 2.1 of the License, or
# (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License for more details.
#

TOPDIR = ../../..
include $(TOPDIR)/src/include/builddefs

CFILES = CommandKeys.c pmproxy2.c
HFILES = CommandKeys.h

LLDLIBS = $(PCPLIB) $(PCP_PMDALIB) $(LIB_FOR_MATH) $(PCP_WEBLIB)
LCFLAGS += -I$(TOPDIR)/src/libpcp_web/src


STATICLIBTARGET = libpcpweb.a

DSOVERSION = 1
LIBTARGET = libpmproxy2.$(DSOSUFFIX).$(DSOVERSION)
SYMTARGET = libpmproxy2.$(DSOSUFFIX)


LDIRT = pmproxy2.log

include $(BUILDRULES)

install: default

default_pcp:	default

install_pcp:	install


pmproxy2.o:	pmproxy2.c CommandKeys.h CommandKeys.c

CommandKeys.o pmproxy2.o:	$(TOPDIR)/src/include/pcp/libpcp.h
