#include <modbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include "cmdline.h"
#include <signal.h>

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
#define ADDR_TYPE_CHAR  5

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
struct addr_type a_type_char = {
	.type = ADDR_TYPE_CHAR,
	.size = 1
};
struct addr_type a_type_coil = {
	.type = ADDR_TYPE_COIL,
	.size = 1
};

typedef struct reg_list {
	char name[50];
	float float_val;
	int32_t int32_val;
	uint8_t uint8_val;
    int8_t int8_val;
    uint16_t uint16_val;
    int16_t int16_val;
	char type[11];
	char unit[10];
	double conversion;
	uint16_t address;
    char rw;
} reg_list_t;

struct mb_util_ctx {
    reg_list_t * reg_list;
    uint16_t reg_index;
    modbus_t *modbus_ctx;
};

//#define address_list_size (int)(sizeof(address_list)/sizeof(address_list[0]))


static void catch_function(int signo) {
	if (exit_now) {
		fprintf(stderr,"Exiting now!\n");
		//modbus_close(ctx);
		//modbus_free(ctx);
		exit(-1);
	} else {
	    if (signo == SIGINT) {
			fprintf(stderr,"Exiting nicely\n");
	    	running = false;
			exit_now = true;
	    }
	}
    
}

void parse_def_string(char * def_str,struct mb_util_ctx * ctx) {
    char type[11];
    char name[51];
    uint16_t reg;
    char value[51];
    float conversion;
    char rw;
    // --reg "type name reg r/w conversion value"
    sscanf(def_str, "%10s%50s%hu %1c%f%10s",type,name,&reg,&rw,&conversion,value);
    
    ctx->reg_list[ctx->reg_index].address = reg;
    strncpy(ctx->reg_list[ctx->reg_index].name,name,50);
    ctx->reg_list[ctx->reg_index].conversion = conversion;
    ctx->reg_list[ctx->reg_index].rw = rw;
    strncpy(ctx->reg_list[ctx->reg_index].type,type,11);
    
    ctx->reg_index++;
    printf("Type: %s\nName: %s\nReg: %u\nValue: %s\nConversion: %f\nR/W: %c\n",type,name,reg,value,conversion,rw);
    
}

void process_registers(struct mb_util_ctx * ctx) {
    printf("Process registers: %d\n",ctx->reg_index);
    for (int i=0;i<ctx->reg_index;i++) {
        printf("Reg list rw: %c\n",ctx->reg_list[i].rw);
        if (ctx->reg_list[i].rw == 'r') {
            printf("Reading: reg: %hu\n",ctx->reg_list[i].address);
            if (strncmp(ctx->reg_list[ctx->reg_index].type,"uint16",10) == 0) {
                rc = modbus_read_input_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, &ctx->reg_list[i].uint16_val);
            }
    
            if (strncmp(ctx->reg_list[i].type,"int16",10) == 0) {
                rc = modbus_read_input_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, (uint16_t *)&ctx->reg_list[i].int16_val);
            }
    
            if (strncmp(ctx->reg_list[i].type,"int8",10) == 0) {
                rc = modbus_read_input_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, (uint16_t *)&ctx->reg_list[i].int8_val);
            }
    
            if (strncmp(ctx->reg_list[i].type,"float",10) == 0) {
                uint16_t tmp[2];
                rc = modbus_read_input_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 2, tmp);
                ctx->reg_list[i].float_val = modbus_get_float_cdab(tmp);
            }
    
            if (strncmp(ctx->reg_list[i].type,"coil",10) == 0) {
                rc = modbus_read_bits(ctx->modbus_ctx, ctx->reg_list[i].address, 1, &ctx->reg_list[i].uint8_val);
            }
        }
    }
}

void print_registers(struct mb_util_ctx * ctx) {
    printf("Process registers: %d\n",ctx->reg_index);
    for (int i=0;i<ctx->reg_index;i++) {
        printf("Reg list rw: %c\n",ctx->reg_list[i].rw);
        if (ctx->reg_list[i].rw == 'r') {
            printf("Reading: reg: %hu\n",ctx->reg_list[i].address);
            if (strncmp(ctx->reg_list[ctx->reg_index].type,"uint16",10) == 0) {
                printf("uint16: %hu\n", ctx->reg_list[i].uint16_val);
            }
    
            if (strncmp(ctx->reg_list[i].type,"int16",10) == 0) {
                printf("uint16: %hd\n", ctx->reg_list[i].int16_val);
            }
    
            if (strncmp(ctx->reg_list[i].type,"int8",10) == 0) {
                printf("uint16: %d\n", ctx->reg_list[i].int8_val);
            }
    
            if (strncmp(ctx->reg_list[i].type,"float",10) == 0) {
                printf("uint16: %f\n", ctx->reg_list[i].float_val);
            }
    
            if (strncmp(ctx->reg_list[i].type,"coil",10) == 0) {
                printf("uint16: %hhu\n", ctx->reg_list[i].uint8_val);
            }
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
    
    struct mb_util_ctx mb_instance;
    mb_instance.reg_list = (reg_list_t *) malloc(sizeof(reg_list_t) * args_info.reg_given);
    mb_instance.reg_index = 0;
    
    
    for (int i = 0; i < args_info.reg_given; ++i) {
        parse_def_string(args_info.reg_arg[i],&mb_instance);
        printf ("passed coil: %s\n", args_info.reg_arg[i]);
    }
    


	
	
	fprintf(stderr, "Connected to: %s\n",args_info.host_arg);
	mb_instance.modbus_ctx = modbus_new_tcp(args_info.host_arg, 502);
	if (mb_instance.modbus_ctx == NULL) {
	    fprintf(stderr, "Unable to create the libmodbus context\n");
	    return -1;
	}
	//modbus_set_slave(ctx, 0);
	modbus_set_debug(mb_instance.modbus_ctx,args_info.debug_flag);
	
	struct timeval rt,rt2;
	
	rt2.tv_sec=1;
	rt2.tv_usec=0;
	modbus_set_byte_timeout(mb_instance.modbus_ctx, &rt);
	rt.tv_sec=10;
	rt.tv_usec=0;
	modbus_set_response_timeout(mb_instance.modbus_ctx, &rt);
	
	if (modbus_connect(mb_instance.modbus_ctx) == -1) {
	    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
	    modbus_free(mb_instance.modbus_ctx);
	    return -1;
	}
    
    process_registers(&mb_instance);
    
    print_registers(&mb_instance);

	//fprintf(stderr, "addr size %d\n",address_list_size);
	//printf("Addr list %s: %f\n",address_list[0].name,address_list[0].fval);
	
	/*uint8_t data[] = {0,0,0,1,1,0,1,0};
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
	}*/
    /*
	
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
	*/
	modbus_close(mb_instance.modbus_ctx);
	modbus_free(mb_instance.modbus_ctx);
    free(mb_instance.reg_list);
    
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