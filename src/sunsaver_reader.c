#include <modbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include "cmdline.h"
#include <signal.h>

modbus_t *ctx;
uint16_t tab_reg[64];
int rc;
int errno;
float modbus_get_float_cdab(const uint16_t *src);
bool running = true;
bool exit_now = false;

#define ADDR_TYPE_FLOAT  2 
#define ADDR_TYPE_WORD  1
#define ADDR_TYPE_SWORD  3
#define ADDR_TYPE_LONG  4

struct gengetopt_args_info args_info;

struct addr_type {
	int type;
	int size;
};

struct addr_type a_type_float = {
	.type = ADDR_TYPE_FLOAT,
	.size = 2
};
struct addr_type a_type_word = {
	.type = ADDR_TYPE_WORD,
	.size = 1
};
struct addr_type a_type_sword = {
	.type = ADDR_TYPE_SWORD,
	.size = 1
};
struct addr_type a_type_long = {
	.type = ADDR_TYPE_LONG,
	.size = 2
};

struct a_list {
	char name[50];
	float fval;
	int32_t ival;
	struct addr_type type;
	char unit[10];
	double conversion;
	uint16_t address;
};
#define address_list_size (int)(sizeof(address_list)/sizeof(address_list[0]))


static void catch_function(int signo) {
	if (exit_now) {
		fprintf(stderr,"Exiting now!\n");
		modbus_close(ctx);
		modbus_free(ctx);
		exit(-1);
	} else {
	    if (signo == SIGINT) {
			fprintf(stderr,"Exiting nicely\n");
	    	running = false;
			exit_now = true;
	    }
	}
    
}


int main(int argc, char **argv) {
    if (signal(SIGINT, catch_function) == SIG_ERR) {
        fprintf(stderr,"An error occurred while setting a signal handler.\n");
        return EXIT_FAILURE;
    }
	/* let's call our cmdline parser */
    if (cmdline_parser (argc, argv, &args_info) != 0) exit(1);
	
    struct a_list address_list[] = {
   	{
   		.name="battery_volts",
   		.address=0x0008,
   		.unit="V",
   		.conversion=0.00305175781,
   		.type = a_type_word
   	},
   	{
   		.name="battery_array_volts",
   		.address=0x0009,
   		.unit="V",
   		.conversion=0.00305175781,
   		.type = a_type_word
   	},
   	{
   		.name="load_volts",
   		.address=0x000A,
   		.unit="V",
   		.conversion=0.00305175781,
   		.type = a_type_word
   	},
   	{
   		.name="charging_current",
   		.address=0x000B,
   		.unit="A",
   		.conversion=0.00241577148,
   		.type = a_type_word
   	},
   	{
   		.name="load_current",
   		.address=0x000C,
   		.unit="A",
   		.conversion=0.00241577148,
   		.type = a_type_word
   	}
   	,
   	{
   		.name="ambient_temp",
   		.address=0x000F,
   		.unit="°C",
   		.conversion=1,
   		.type = a_type_sword
   	}
   	,
   	{
   		.name="ah_charge",
   		.address=0x0015,
   		.unit="Ah",
   		.conversion=0.1,
   		.type = a_type_long
   	}
   	,
   	{
   		.name="ah_load",
   		.address=0x001D,
   		.unit="Ah",
   		.conversion=0.1,
   		.type = a_type_long
   	}
   };
	
	
	fprintf(stderr, "Connected to: %s\n",args_info.tty_arg);
	ctx = modbus_new_rtu(args_info.tty_arg, 9600, 'N', 8, 2);
	if (ctx == NULL) {
	    fprintf(stderr, "Unable to create the libmodbus context\n");
	    return -1;
	}
	modbus_set_slave(ctx, 1);
	modbus_set_debug(ctx,args_info.debug_flag);
	struct timeval rt;
	rt.tv_sec=5;
	rt.tv_usec=0;
	modbus_set_response_timeout(ctx, &rt);
	
	if (modbus_connect(ctx) == -1) {
	    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
	    modbus_free(ctx);
	    return -1;
	}
	
	
	fprintf(stderr, "addr size %d\n",address_list_size);
	//printf("Addr list %s: %f\n",address_list[0].name,address_list[0].fval);
	
	for (int i=0;i<address_list_size;i++) {
		rc = modbus_read_registers(ctx,address_list[i].address,address_list[i].type.size ,tab_reg);
		if (rc == -1) {
			fprintf(stderr, "Address: %d failed\n",address_list[i].address);
		    fprintf(stderr, "%s\n", modbus_strerror(errno));
		    //return -1;
		} else {
			fprintf(stderr, "Address: %d success\n",address_list[i].address);
			if (address_list[i].type.type==ADDR_TYPE_FLOAT) {
				address_list[i].fval = (float)modbus_get_float_cdab(tab_reg);
				fprintf(stderr, "%s: %f\n", address_list[i].name, address_list[i].fval);
			}
			if (address_list[i].type.type==ADDR_TYPE_WORD) {
				address_list[i].ival = (unsigned int)tab_reg[0];
				fprintf(stderr, "%s: %u\n", address_list[i].name, address_list[i].ival );
			}
			if (address_list[i].type.type==ADDR_TYPE_SWORD) {
				address_list[i].ival = (int)tab_reg[0];
				fprintf(stderr, "%s: %u\n", address_list[i].name, address_list[i].ival );
			}
			if (address_list[i].type.type==ADDR_TYPE_LONG) {
				address_list[i].ival = (((int32_t)tab_reg[1])<<16)|tab_reg[1];
				fprintf(stderr, "%s: %u\n", address_list[i].name, address_list[i].ival );
			}
		}
		sleep(1);
	}
	printf("{\n");
	printf("\t\"water_sensor\": {\n");
	for (int i=0;i<address_list_size;i++) {
		if (address_list[i].type.type==ADDR_TYPE_FLOAT) {
			printf("\t\t\"%s\":%f",address_list[i].name,address_list[i].fval * address_list[i].conversion);
		}
		if (address_list[i].type.type==ADDR_TYPE_WORD) {
			printf("\t\t\"%s\":%f",address_list[i].name,address_list[i].ival * address_list[i].conversion);
		}
		if (address_list[i].type.type==ADDR_TYPE_SWORD) {
			printf("\t\t\"%s\":%f",address_list[i].name,address_list[i].ival * address_list[i].conversion);
		}
		if (address_list[i].type.type==ADDR_TYPE_LONG) {
			printf("\t\t\"%s\":%f",address_list[i].name,address_list[i].ival * address_list[i].conversion);
		}
		// Last item in list
		if (i != address_list_size - 1) {
			printf(",\n");
		} else {
			printf("\n");
		}
	}
	printf("\t}\n");
	printf("}\n");
	modbus_close(ctx);
	modbus_free(ctx);

	return 0;
}

/* Get a float from 4 bytes in Modbus format (ABCD) */
float modbus_get_float_cdab(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = (((uint32_t)src[0]) << 16) + src[1];
    memcpy(&f, &i, sizeof(float));

    return f;
}