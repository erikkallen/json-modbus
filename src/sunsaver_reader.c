#include <modbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

modbus_t *ctx;

modbus_t *ctx;
uint16_t tab_reg[64];
int rc;
int errno;
float modbus_get_float_cdab(const uint16_t *src);


#define ADDR_TYPE_FLOAT  2 
#define ADDR_TYPE_WORD  1


struct {
	char name[50];
	float fval;
	int ival;
	int type;
	uint16_t address;
} address_list[] = {
	{
		.name="rtcmilliseconds",
		.address=1001,
		.type = ADDR_TYPE_WORD
	},
	{
		.name="rtcseconds",
		.address=1002,
		.type = ADDR_TYPE_WORD
	},
	{
		.name="rtcminutes",
		.address=1003,
		.type = ADDR_TYPE_WORD
	},
	{
		.name="rtchours",
		.address=1004,
		.type = ADDR_TYPE_WORD
	},
	{
		.name="rtcdays",
		.address=1005,
		.type = ADDR_TYPE_WORD
	}
	,
	{
		.name="rtcmonths",
		.address=1007,
		.type = ADDR_TYPE_WORD
	}
	,
	{
		.name="rtcyears",
		.address=1008,
		.type = ADDR_TYPE_WORD
	}
	,
	{
		.name="temperature",
		.address=6668,
		.type = ADDR_TYPE_FLOAT
	}
	,
	{
		.name="flow",
		.address=6686,
		.type = ADDR_TYPE_FLOAT
	},
	{
		.name="reverse_flow",
		.address=6688,
		.type = ADDR_TYPE_FLOAT
	},
	{
		.name="pressure-mv_ext_input",
		.address=6692,
		.type = ADDR_TYPE_FLOAT
	},
	{
		.name="ph",
		.address=6698,
		.type = ADDR_TYPE_FLOAT
	},
	{
		.name="orp",
		.address=6704,
		.type = ADDR_TYPE_FLOAT
	},
	{
		.name="conductivity",
		.address=6710,
		.type = ADDR_TYPE_FLOAT
	},
	{
		.name="turbidity",
		.address=6716,
		.type = ADDR_TYPE_FLOAT
	},
	{
		.name="colour",
		.address=6722,
		.type = ADDR_TYPE_FLOAT
	}	,
	{
		.name="free_chlorine",
		.address=6728,
		.type = ADDR_TYPE_FLOAT
	}
	,
	{
		.name="spare_chlorine",
		.address=6734,
		.type = ADDR_TYPE_FLOAT
	}
	,
	{
		.name="mono-chloramine",
		.address=6740,
		.type = ADDR_TYPE_FLOAT
	}
	,
	{
		.name="dissolved_oxygen",
		.address=6746,
		.type = ADDR_TYPE_FLOAT
	}
	,
	{
		.name="ise",
		.address=6752,
		.type = ADDR_TYPE_FLOAT
	}
	,
	{
		.name="external_flow",
		.address=6776,
		.type = ADDR_TYPE_FLOAT
	}
	,
	{
		.name="minutes",
		.address=6648,
		.type = ADDR_TYPE_FLOAT
	}
	,
	{
		.name="hours",
		.address=6650,
		.type = ADDR_TYPE_FLOAT
	}
	,
	{
		.name="day",
		.address=6652,
		.type = ADDR_TYPE_FLOAT
	}
	,
	{
		.name="month",
		.address=6656,
		.type = ADDR_TYPE_FLOAT
	}
	,
	{
		.name="year",
		.address=6658,
		.type = ADDR_TYPE_FLOAT
	}
};
#define address_list_size (int)(sizeof(address_list)/sizeof(address_list[0]))

int main() {
	ctx = modbus_new_rtu("/dev/tty.usbserial-FTVXKTF3", 19200, 'N', 8, 1);
	if (ctx == NULL) {
	    fprintf(stderr, "Unable to create the libmodbus context\n");
	    return -1;
	}
	modbus_set_slave(ctx, 1);
	modbus_set_debug(ctx,FALSE);
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
		rc = modbus_read_registers(ctx,address_list[i].address,address_list[i].type ,tab_reg);
		if (rc == -1) {
			fprintf(stderr, "Address: %d failed\n",address_list[i].address);
		    fprintf(stderr, "%s\n", modbus_strerror(errno));
		    //return -1;
		} else {
			fprintf(stderr, "Address: %d success\n",address_list[i].address);
			if (address_list[i].type==ADDR_TYPE_FLOAT) {
				address_list[i].fval = (float)modbus_get_float_cdab(tab_reg);
				fprintf(stderr, "%s: %f\n", address_list[i].name, address_list[i].fval);
			}
			if (address_list[i].type==ADDR_TYPE_WORD) {
				address_list[i].ival = (unsigned int)tab_reg[0];
				fprintf(stderr, "%s: %u\n", address_list[i].name, address_list[i].ival );
			}
		}
		sleep(1);
	}
	printf("{\n");
	printf("\t\"water_sensor\": {\n");
	for (int i=0;i<address_list_size;i++) {
		if (address_list[i].type==ADDR_TYPE_FLOAT) {
			printf("\t\t\"%s\":%f",address_list[i].name,address_list[i].fval);
		}
		if (address_list[i].type==ADDR_TYPE_WORD) {
			printf("\t\t\"%s\":%u",address_list[i].name,address_list[i].ival);
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