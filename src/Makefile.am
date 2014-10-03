bin_PROGRAMS = powerbox_modbus

GIT_VERSION:=$(shell git describe --abbrev=10 --dirty --always)

powerbox_modbus_SOURCES = cmdline.h cmdline.c powerbox_modbus.c
powerbox_modbus_CFLAGS = $(MODBUS_CFLAGS) -DGIT_VERSION=\"$(GIT_VERSION)\"
powerbox_modbus_LDADD =  $(MODBUS_LIBS)




cmdline.c: $(top_srcdir)/powerbox_modbus.ggo
	gengetopt -C < $(top_srcdir)/powerbox_modbus.ggo
	sed "s/\"GIT_VERSION\"/GIT_VERSION/" $@ > $@.tmp
	mv $@.tmp $@