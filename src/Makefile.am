bin_PROGRAMS = powerbox_modbus
powerbox_modbus_SOURCES = cmdline.h cmdline.c powerbox_modbus.c
powerbox_modbus_CFLAGS = $(MODBUS_CFLAGS)
powerbox_modbus_LDADD =  $(MODBUS_LIBS)


cmdline.c: $(top_srcdir)/powerbox_modbus.ggo
	gengetopt -C < $(top_srcdir)/powerbox_modbus.ggo