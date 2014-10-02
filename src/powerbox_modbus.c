#include <modbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include "cmdline.h"
#include <signal.h>

#define DEBUG_MSG(msg,...) if (debug_mode) fprintf(stderr, msg, ##__VA_ARGS__);

uint8_t tab_reg[8];
int rc;
int errno;
float modbus_get_float_cdab(const uint16_t *src);
bool running = true;
bool exit_now = false;
bool debug_mode = false;

#define MB_TYPE_FLOAT  2 
#define MB_TYPE_WORD  1
#define MB_TYPE_SWORD  3
#define MB_TYPE_LONG  4
#define MB_TYPE_COIL  5
#define MB_TYPE_CHAR  5

enum mb_data_type {
  mb_float,
  mb_uint8,
  mb_uint16,
  mb_int8,
  mb_int16,
  mb_coil  
};

struct gengetopt_args_info args_info;

typedef struct reg_list {
	char name[50];
	float float_val;
	int32_t int32_val;
	uint8_t uint8_val;
    int8_t int8_val;
    uint16_t uint16_val;
    int16_t int16_val;
	enum mb_data_type type;
	char unit[10];
	double conversion;
	uint16_t address;
    char rw;
} reg_list_t;

struct mb_util_ctx {
    reg_list_t * reg_list;
    uint16_t reg_index;
    modbus_t *modbus_ctx;
	char name[51];
	char rw;
};

//#define address_list_size (int)(sizeof(address_list)/sizeof(address_list[0]))


static void catch_function(int signo) {
	if (exit_now) {
		DEBUG_MSG("Exiting now!\n");
		//modbus_close(ctx);
		//modbus_free(ctx);
		exit(-1);
	} else {
	    if (signo == SIGINT) {
			DEBUG_MSG("Exiting nicely\n");
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
    sscanf(def_str, "%10s%50s%hu %f%10s",type,name,&reg,&conversion,value);
    
    ctx->reg_list[ctx->reg_index].address = reg;
    strncpy(ctx->reg_list[ctx->reg_index].name,name,50);
    ctx->reg_list[ctx->reg_index].conversion = conversion;
    ctx->reg_list[ctx->reg_index].rw = rw;
	
    if (strncmp(type,"uint16",10) == 0) {
        ctx->reg_list[ctx->reg_index].type = mb_uint16;
		ctx->reg_list[ctx->reg_index].uint16_val = (uint16_t)atoi(value);
    }

    if (strncmp(type,"int16",10) == 0) {
        ctx->reg_list[ctx->reg_index].type = mb_int16;
		ctx->reg_list[ctx->reg_index].int16_val = (int16_t)atoi(value);
    }

    if (strncmp(type,"int8",10) == 0) {
        ctx->reg_list[ctx->reg_index].type = mb_int8;
		ctx->reg_list[ctx->reg_index].int8_val = (int8_t)atoi(value);
    }

    if (strncmp(type,"float",10) == 0) {
        ctx->reg_list[ctx->reg_index].type = mb_float;
        //fprintf(stderr,"writing float not implemented");
        //exit(1);
    }

    if (strncmp(type,"coil",10) == 0) {
        ctx->reg_list[ctx->reg_index].type = mb_coil;
		ctx->reg_list[ctx->reg_index].uint8_val = (uint8_t)atoi(value);
    }
	
	
    
    ctx->reg_index++;
    DEBUG_MSG("Type: %s\nName: %s\nReg: %u\nValue: %s\nConversion: %f\nR/W: %c\n",type,name,reg,value,conversion,rw);
    
}

void process_registers(struct mb_util_ctx * ctx) {
    uint16_t tmp[2];
    DEBUG_MSG("Process registers: %d\n",ctx->reg_index);
	DEBUG_MSG("Reg list rw: %c\n",ctx->rw);
    
	// Read registers
	if (ctx->rw == 'r') {
    	for (int i=0;i<ctx->reg_index;i++) {
	        DEBUG_MSG("Reading: reg: %hu\n",ctx->reg_list[i].address);
            switch(ctx->reg_list[i].type) {
                case mb_uint16:
                    rc = modbus_read_input_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, &ctx->reg_list[i].uint16_val);
                break;
                case mb_int16:
                    rc = modbus_read_input_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, (uint16_t *)&ctx->reg_list[i].int16_val);
                break;
                case mb_int8:
                    rc = modbus_read_input_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, (uint16_t *)&ctx->reg_list[i].int8_val);
                break;
                case mb_uint8:
                    rc = modbus_read_input_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, (uint16_t *)&ctx->reg_list[i].int8_val);
                break;
                case mb_float:
    	            rc = modbus_read_input_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 2, tmp);
    	            ctx->reg_list[i].float_val = modbus_get_float_cdab(tmp);
                break;
                case mb_coil:
                    rc = modbus_read_bits(ctx->modbus_ctx, ctx->reg_list[i].address, 1, &ctx->reg_list[i].uint8_val);
                break;
                default:
                break;
            }
        }
    } else { // Write registers
    	for (int i=0;i<ctx->reg_index;i++) {
	        DEBUG_MSG("Writing: reg: %hu\n",ctx->reg_list[i].address);
            switch(ctx->reg_list[i].type) {
                case mb_uint16:
                    rc = modbus_write_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, &ctx->reg_list[i].uint16_val);
                break;
                case mb_int16:
                    rc = modbus_write_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, (uint16_t *)&ctx->reg_list[i].int16_val);
                break;
                case mb_int8:
                    rc = modbus_write_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, (uint16_t *)&ctx->reg_list[i].int8_val);
                break;
                case mb_uint8:
                    rc = modbus_write_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, (uint16_t *)&ctx->reg_list[i].int8_val);
                break;
                case mb_float:
                DEBUG_MSG("writing float not implemented");
                exit(1);
                break;
                case mb_coil:
                    rc = modbus_write_bit(ctx->modbus_ctx, ctx->reg_list[i].address, ctx->reg_list[i].uint8_val);
                break;
                default:
                break;
            }
        }
    }
}

void print_registers(struct mb_util_ctx * ctx) {
    //printf("Process registers: %d\n",ctx->reg_index);
	printf("{\n");
	printf("\t\"%s\": {\n", ctx->name);
    for (int i=0;i<ctx->reg_index;i++) {
        //printf("Reg list rw: %c\n",ctx->reg_list[i].rw);
        //printf("Reading: reg: %hu\n",ctx->reg_list[i].address);
        switch(ctx->reg_list[i].type) {
            case mb_uint16:
                printf("\t\t\"%s\":%f", ctx->reg_list[i].name, ctx->reg_list[i].uint16_val * ctx->reg_list[i].conversion);
            break;
            case mb_int16:
                printf("\t\t\"%s\":%f", ctx->reg_list[i].name, ctx->reg_list[i].int16_val * ctx->reg_list[i].conversion);
            break;
            case mb_int8:
                printf("\t\t\"%s\":%f", ctx->reg_list[i].name, ctx->reg_list[i].int8_val * ctx->reg_list[i].conversion);
            break;
            case mb_uint8:
                printf("\t\t\"%s\":%f", ctx->reg_list[i].name, ctx->reg_list[i].uint8_val * ctx->reg_list[i].conversion);
            break;
            case mb_float:
                printf("\t\t\"%s\":%f", ctx->reg_list[i].name, ctx->reg_list[i].float_val * ctx->reg_list[i].conversion);
            break;
            case mb_coil:
                printf("\t\t\"%s\":%hhu", ctx->reg_list[i].name, ctx->reg_list[i].uint8_val);
            break;
            default:
            break;
        }
        
		// Last item in list
		if (i != ctx->reg_index - 1) {
			printf(",\n");
		} else {
			printf("\n");
		}
    }
	printf("\t}\n");
	printf("}\n");
}

int main(int argc, char **argv) {
    if (signal(SIGINT, catch_function) == SIG_ERR) {
        DEBUG_MSG("An error occurred while setting a signal handler.\n");
        return EXIT_FAILURE;
    }
    
    struct cmdline_parser_params *params;
     
    /* initialize the parameters structure */
    params = cmdline_parser_params_create();
    
    params->initialize = 1;
    
     /* call the config file parser */
    if (cmdline_parser_config_file(args_info.conf_file_arg, &args_info, params) != 0) exit(1);
    
	/* let's call our cmdline parser */
    if (cmdline_parser (argc, argv, &args_info) != 0) exit(1);
    
    struct mb_util_ctx mb_instance;
    mb_instance.reg_list = (reg_list_t *) malloc(sizeof(reg_list_t) * args_info.reg_given);
    mb_instance.reg_index = 0;
	if (args_info.write_flag == true) {
		DEBUG_MSG("Write registers\n");
		mb_instance.rw = 'w';
	} else {
		DEBUG_MSG("Read registers\n");
		mb_instance.rw = 'r';
	}
	// Copy system name
	strncpy(mb_instance.name,args_info.name_arg,49);
    
    
    for (int i = 0; i < args_info.reg_given; ++i) {
        parse_def_string(args_info.reg_arg[i],&mb_instance);
        DEBUG_MSG("passed coil: %s\n", args_info.reg_arg[i]);
    }
	
	DEBUG_MSG("Connected to: %s\n",args_info.host_arg);
	mb_instance.modbus_ctx = modbus_new_tcp(args_info.host_arg, 502);
	if (mb_instance.modbus_ctx == NULL) {
	    DEBUG_MSG("Unable to create the libmodbus context\n");
	    return -1;
	}
	//modbus_set_slave(ctx, 0);
	modbus_set_debug(mb_instance.modbus_ctx,args_info.debug_flag);
    debug_mode = args_info.debug_flag;
	
	struct timeval rt,rt2;
	
	/*rt2.tv_sec=1;
	rt2.tv_usec=0;
	modbus_set_byte_timeout(mb_instance.modbus_ctx, &rt);
	rt.tv_sec=10;
	rt.tv_usec=0;
	modbus_set_response_timeout(mb_instance.modbus_ctx, &rt);
	*/
	if (modbus_connect(mb_instance.modbus_ctx) == -1) {
	    DEBUG_MSG("Connection failed: %s\n", modbus_strerror(errno));
	    modbus_free(mb_instance.modbus_ctx);
	    return -1;
	}
    
    process_registers(&mb_instance);
    
    print_registers(&mb_instance);

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