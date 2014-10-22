bin_PROGRAMS = rtu_server_test http_server_test
rtu_server_test_SOURCES = rtu_server_test.c
rtu_server_test_CFLAGS = $(MODBUS_CFLAGS)
rtu_server_test_LDADD =  $(MODBUS_LIBS)

http_server_test_SOURCES = http_server_test.c
http_server_test_CFLAGS = $(MODBUS_CFLAGS) -ggdb
http_server_test_LDADD =  $(MODBUS_LIBS)