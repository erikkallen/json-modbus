#include <modbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include "cmdline.h"
#include <signal.h>
#include <regex.h>

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
  mb_float_cdab,
  mb_float_array,
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
    uint16_t * float_array;
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
    char type[16];
    char name[51];
    uint16_t reg;
    char value[51];
    float conversion;
    char rw;
    // --reg "type name reg conversion value"
    sscanf(def_str, "%15s%50s%hu %f%10s",type,name,&reg,&conversion,value);
    
    DEBUG_MSG("Parsing string\n");
    int reti, num_matches=0;
    regex_t p;
    regmatch_t *pm = malloc(sizeof(regmatch_t)*5);
    memset(pm, 0,sizeof(regmatch_t)*5);
    reti = regcomp(&p, "([a-zA-Z]+) ([a-zA-Z]+) ([0-9.]*) ?([0-9.]*)", REG_EXTENDED);
    if (reti) {
        DEBUG_MSG(stderr, "Could not compile regex\n");
        exit(1);
    }
    reti = regexec(&p, def_str, 5, pm, 0);
    for (int i=1;i<5;i++ ) {
        char s[100];
        memset(s,0,100);
        strncpy(s,&def_str[(pm + i)->rm_so], (pm+i)->rm_eo - (pm+i)->rm_so);
        DEBUG_MSG(" %s start=%lld end=%lld\n",s,(pm+i)->rm_so, (pm+i)->rm_eo);
        if ((pm+i)->rm_so == (pm+i)->rm_eo) {
            
            num_matches = i -1;
        }
        //last_match += pm.rm_so+1;
    }
    regfree(&p);
    free(pm);
    
    DEBUG_MSG("Num matches: %d\n", num_matches);
    
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
    
    if (strncmp(type,"float_cdab",15) == 0) {
        ctx->reg_list[ctx->reg_index].type = mb_float_cdab;
        //fprintf(stderr,"writing float not implemented");
        //exit(1);
    }
    
    if (strncmp(type,"float_array",15) == 0) {
        ctx->reg_list[ctx->reg_index].type = mb_float_array;
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
                    rc = modbus_read_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, &ctx->reg_list[i].uint16_val);
                break;
                case mb_int16:
                    rc = modbus_read_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, (uint16_t *)&ctx->reg_list[i].int16_val);
                break;
                case mb_int8:
                    rc = modbus_read_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, (uint16_t *)&ctx->reg_list[i].int8_val);
                break;
                case mb_uint8:
                    rc = modbus_read_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, (uint16_t *)&ctx->reg_list[i].int8_val);
                break;
                case mb_float:
    	            rc = modbus_read_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 2, tmp);
    	            ctx->reg_list[i].float_val = modbus_get_float(tmp);
                break;
                case mb_float_cdab:
    	            rc = modbus_read_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 2, tmp);
    	            ctx->reg_list[i].float_val = modbus_get_float_cdab(tmp);
                break;
                case mb_float_array:
                    // Allocate room for float array
                    ctx->reg_list[i].float_array = (uint16_t *)malloc(sizeof(uint16_t) * ctx->reg_list[i].conversion);
    	            rc = modbus_read_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 2 * ctx->reg_list[i].conversion, ctx->reg_list[i].float_array);
    	            //ctx->reg_list[i].float_val = modbus_get_float_cdab(tmp);
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
                case mb_float_cdab:
                case mb_float_array:
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
            case mb_float_cdab:
                printf("\t\t\"%s\":%f", ctx->reg_list[i].name, ctx->reg_list[i].float_val * ctx->reg_list[i].conversion);
            break;
            case mb_float_array:
                printf("\t\t\"%s\":[", ctx->reg_list[i].name);
                for (int j=0;j<ctx->reg_list[i].conversion;j++) {
                    printf("%f",modbus_get_float(ctx->reg_list[i].float_array+(j*2)));
                    if (j != ctx->reg_list[i].conversion - 1) {
            			printf(",");
            		}
                }
                printf("]\n");
                // Cleanup memory
                free(ctx->reg_list[i].float_array);
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
	params->override = 1;
	params->check_required = 0;
	
	/* let's call our cmdline parser */
    if (cmdline_parser_ext (argc, argv, &args_info, params) != 0) exit(1);
	
	params->initialize = 0;
	params->override = 0;
	params->check_required = 1;
	
	if (args_info.conf_file_given) {
		printf("Reading config\n");
     	/* call the config file parser */
    	if (cmdline_parser_config_file(args_info.conf_file_arg, &args_info, params) != 0) exit(1);
	}
    debug_mode = args_info.debug_flag;
    
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
	
	DEBUG_MSG("Connecting to: %s\n",args_info.host_arg);
	mb_instance.modbus_ctx = modbus_new_tcp(args_info.host_arg, args_info.port_arg);
	if (mb_instance.modbus_ctx == NULL) {
	    DEBUG_MSG("Unable to create the libmodbus context\n");
	    return -1;
	}
	modbus_set_slave(mb_instance.modbus_ctx, 1);
	modbus_set_debug(mb_instance.modbus_ctx,args_info.debug_flag);
    
	
	struct timeval rt,rt2;
	
	/*rt2.tv_sec=1;
	rt2.tv_usec=0;
	modbus_set_byte_timeout(mb_instance.modbus_ctx, &rt);
	rt.tv_sec=10;
	rt.tv_usec=0;
	modbus_set_response_timeout(mb_instance.modbus_ctx, &rt);
	*/
	rt.tv_sec=args_info.timeout_arg;
	rt.tv_usec=0;
    modbus_set_response_timeout(mb_instance.modbus_ctx, &rt);
    
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