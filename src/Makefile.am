bin_PROGRAMS = sunsaver_reader
sunsaver_reader_SOURCES = cmdline.h cmdline.c sunsaver_reader.c
sunsaver_reader_CFLAGS = $(MODBUS_CFLAGS)
sunsaver_reader_LDADD =  $(MODBUS_LIBS)


cmdline.c: $(top_srcdir)/sunsaver_reader.ggo
	gengetopt < $(top_srcdir)/sunsaver_reader.ggo