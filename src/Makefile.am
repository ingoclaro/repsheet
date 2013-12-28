AUTOMAKE_OPTIONS = -Wno-override
lib_LTLIBRARIES = mod_repsheet.la
mod_repsheet_la_SOURCES = proxy.c mod_security.c repsheet.c mod_repsheet.c
mod_repsheet_la_LDFLAGS = -module -lhiredis
AM_CFLAGS = `../build/apache/bin/apr-1-config --cflags`
AM_CPPFLAGS = -I@APACHE_INCLUDE_DIR@ `../build/apache/bin/apr-1-config --cppflags --includes`

install:
	@APXS@ -i -a mod_repsheet.la
