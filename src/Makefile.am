# Copyright 2023 MarcosHCK
# This file is part of WebServer.
#
# WebServer is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# WebServer is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with WebServer. If not, see <http://www.gnu.org/licenses/>.
#

VOID=

#
# Binaries and libraries
# - declare
#

bin_PROGRAMS=webserver
noinst_HEADERS=webserver.h

#
# Binaries and libraries
# - sources
#

webserver_SOURCES=webserver.c 
webserver_CFLAGS=-flto \
	-DG_LOG_DOMAIN=\"Jash\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(GIO_CFLAGS)
webserver_LDFLAGS=-flto
webserver_LDADD=$(GIO_LIBS)
