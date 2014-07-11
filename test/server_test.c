/*
 * Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the BSD License.
 */

#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <stdlib.h>
#include <errno.h>

#include <modbus.h>

int main(void)
{
    int s = -1;
    modbus_t *ctx;
    modbus_mapping_t *mb_mapping;

	ctx = modbus_new_rtu("/dev/tty.usbserial-FTFK3GHG", 9600, 'N', 8, 2);
	if (ctx == NULL) {
	    fprintf(stderr, "Unable to create the libmodbus context\n");
	    return -1;
	}
	modbus_set_slave(ctx, 1);
	modbus_set_debug(ctx,TRUE);
	struct timeval rt;
	rt.tv_sec=5;
	rt.tv_usec=0;
	modbus_set_response_timeout(ctx, &rt);
	
	
	
	
    /* modbus_set_debug(ctx, TRUE); */

    mb_mapping = modbus_mapping_new(500, 500, 500, 500);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

	if (modbus_connect(ctx) == -1) {
	    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
	    modbus_free(ctx);
	    return -1;
	}
	// battery_volts
	mb_mapping->tab_registers[8] =3660;
	// battery_array_volts
	mb_mapping->tab_registers[9] =3660;
	// load_volts
	mb_mapping->tab_registers[10] =3660;
	// charging_current
	mb_mapping->tab_registers[11] =3660;
	// load_current
	mb_mapping->tab_registers[12] =3660;
	// ambient_temp
	mb_mapping->tab_registers[15] =24;
	// ah_charge
	mb_mapping->tab_registers[21] =3660;
	mb_mapping->tab_registers[22] =3660;
	// ah_load
	mb_mapping->tab_registers[29] =3660;
	mb_mapping->tab_registers[30] =3660;

    for (;;) {
        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
        int rc;

        rc = modbus_receive(ctx, query);
        if (rc > 0) {
            /* rc is the query size */
            modbus_reply(ctx, query, rc, mb_mapping);
        } else if (rc == -1) {
            /* Connection closed by the client or error */
            break;
        }
    }

    printf("Quit the loop: %s\n", modbus_strerror(errno));

    if (s != -1) {
        close(s);
    }
    modbus_mapping_free(mb_mapping);
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}