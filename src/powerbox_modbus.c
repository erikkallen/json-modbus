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
uint8_t tab_reg[8];
int rc;
int errno;
float modbus_get_float_cdab(const uint16_t *src);
bool running = true;
bool exit_now = false;

#define ADDR_TYPE_FLOAT  2 
#define ADDR_TYPE_WORD  1
#define ADDR_TYPE_SWORD  3
#define ADDR_TYPE_LONG  4
#define ADDR_TYPE_COIL  5

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
struct addr_type a_type_coil = {
	.type = ADDR_TYPE_COIL,
	.size = 1
};

struct a_list {
	char name[50];
	float fval;
	int32_t ival;
	uint8_t cval;
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
   		.name="camera",
   		.address=0,
   		.type = a_type_coil
   	},
   	{
   		.name="cec-a",
   		.address=1,
   		.type = a_type_coil
   	},
   	{
   		.name="cec-b",
   		.address=2,
   		.type = a_type_coil
   	},
   	{
   		.name="modem-b",
   		.address=3,
   		.type = a_type_coil
   	},
   	{
   		.name="rad_sensor",
   		.address=4,
   		.type = a_type_coil
   	},	
	{
   		.name="water_sensor",
   		.address=5,
   		.type = a_type_coil
   	}
   	,
   	{
   		.name="port_6",
   		.address=6,
   		.type = a_type_coil
   	}
   	,
   	{
   		.name="port_7",
   		.address=7,
   		.type = a_type_coil
   	}
   };
	
	
	fprintf(stderr, "Connected to: %s\n",args_info.host_arg);
	ctx = modbus_new_tcp(args_info.host_arg, 502);
	if (ctx == NULL) {
	    fprintf(stderr, "Unable to create the libmodbus context\n");
	    return -1;
	}
	//modbus_set_slave(ctx, 0);
	modbus_set_debug(ctx,args_info.debug_flag);
	
	struct timeval rt,rt2;
	
	rt2.tv_sec=1;
	rt2.tv_usec=0;
	modbus_set_byte_timeout(ctx, &rt);
	rt.tv_sec=10;
	rt.tv_usec=0;
	modbus_set_response_timeout(ctx, &rt);
	
	if (modbus_connect(ctx) == -1) {
	    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
	    modbus_free(ctx);
	    return -1;
	}

	
	fprintf(stderr, "addr size %d\n",address_list_size);
	//printf("Addr list %s: %f\n",address_list[0].name,address_list[0].fval);
	
	uint8_t data[] = {0,0,0,1,1,0,1,0};
	rc = modbus_write_bits(ctx,0,8,data);
	if (rc == -1) {
		fprintf(stderr, "Write: 0 failed\n");
	    fprintf(stderr, "%s\n", modbus_strerror(errno));
	    //return -1;
	}
	
	
	rc = modbus_read_bits(ctx,0,8,tab_reg);
	if (rc == -1) {
		fprintf(stderr, "Read: 0 failed\n");
	    fprintf(stderr, "%s\n", modbus_strerror(errno));
	    //return -1;
	}
	
	for (int i=0;i<address_list_size;i++) {
		address_list[i].cval = tab_reg[i];
		printf("Tab reg: %d %x\n",tab_reg[i],tab_reg[i]);
	}
	
	
	printf("{\n");
	printf("\t\"power_box\": {\n");
	for (int i=0;i<address_list_size;i++) {
		if (address_list[i].type.type==ADDR_TYPE_COIL) {
			printf("\t\t\"%s\":%d",address_list[i].name,address_list[i].cval);
		}
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