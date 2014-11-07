bin_PROGRAMS = json_modbus

GIT_VERSION:=$(shell git describe --abbrev=10 --dirty --always)

json_modbus_SOURCES = cmdline.h cmdline.c json_modbus.c
json_modbus_CFLAGS = $(MODBUS_CFLAGS) -DGIT_VERSION=\"$(GIT_VERSION)\"
json_modbus_LDADD =  $(MODBUS_LIBS)

cmdline.c: $(top_srcdir)/json_modbus.ggo
	gengetopt -C < $(top_srcdir)/json_modbus.ggo
	sed "s/\"GIT_VERSION\"/GIT_VERSION/" $@ > $@.tmp
	mv $@.tmp $@
