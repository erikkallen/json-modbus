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
#include "dbg.h"
#include "json_modbus.h"

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
    float conversion = 1;
    int last_pos = 0;
    ctx->reg_list[ctx->reg_index].convert = false;
    // --reg "type name reg [value] conversion"
  
#define STRLEN(s) ((sizeof(s)/sizeof(s[0]))-1)
#define CHECK_STR(str,sstr) (strncmp(str,sstr,STRLEN(sstr)) == 0)
    
    if (CHECK_STR(def_str,"int") || CHECK_STR(def_str,"uint") || CHECK_STR(def_str,"float") || CHECK_STR(def_str,"float_cdab")) {
        if (ctx->rw == 'r') {
            sscanf(def_str, "%15s%50s%hu%n",type,name,&reg,&last_pos);
            DEBUG_MSG("Lst pos: %d size: %lu\n",last_pos, strlen(def_str));
        } else {
            sscanf(def_str, "%15s%50s%hu%20s%n",type,name,&reg,value,&last_pos);
        }
        
        if (last_pos != strlen(def_str)) {
            sscanf(&def_str[last_pos], "%f",&conversion);
            ctx->reg_list[ctx->reg_index].convert = true;
        }
    }
    
    if (CHECK_STR(def_str,"int8")) {
        ctx->reg_list[ctx->reg_index].type = mb_int8;
    	ctx->reg_list[ctx->reg_index].int8_val = (int8_t)atoi(value);
    } else if (CHECK_STR(def_str,"int16")) {
        ctx->reg_list[ctx->reg_index].type = mb_int16;
		ctx->reg_list[ctx->reg_index].int16_val = (int16_t)atoi(value);
    } else if (CHECK_STR(def_str,"int32")) {
        ctx->reg_list[ctx->reg_index].type = mb_int32;
		ctx->reg_list[ctx->reg_index].uint32_val = (int32_t)atoi(value);
    } else if (CHECK_STR(def_str,"uint8")) {
        ctx->reg_list[ctx->reg_index].type = mb_uint8;
		ctx->reg_list[ctx->reg_index].uint8_val = (uint8_t)atoi(value);
    } else if (CHECK_STR(def_str,"uint16")) {
        ctx->reg_list[ctx->reg_index].type = mb_uint16;
		ctx->reg_list[ctx->reg_index].uint16_val = (uint16_t)atoi(value);
    } else if (CHECK_STR(def_str,"uint32")) {
        ctx->reg_list[ctx->reg_index].type = mb_uint32;
		ctx->reg_list[ctx->reg_index].uint32_val = (uint32_t)atoi(value);
    } else if (CHECK_STR(def_str,"float")) {
        ctx->reg_list[ctx->reg_index].type = mb_float;
		ctx->reg_list[ctx->reg_index].float_val = atof(value);
    } else if (CHECK_STR(def_str,"float_cdab")) {
        ctx->reg_list[ctx->reg_index].type = mb_float_cdab;
		ctx->reg_list[ctx->reg_index].float_val = atof(value);
    } else if (CHECK_STR(def_str,"float_array")) {
        ctx->reg_list[ctx->reg_index].type = mb_float_array;
    } else if (CHECK_STR(def_str,"coils")) {
        sscanf(def_str, "%15s%50s%hu",type,name,&reg);
        ctx->reg_list[ctx->reg_index].type = mb_coils;
        char *tok = NULL;
        tok = strtok(&def_str[5], " ");
        int i;
        DEBUG_MSG("str %s\n",def_str);
        while (tok) {
            DEBUG_MSG("Token: %s\n", tok);
            ctx->reg_list[ctx->reg_index].coil_array[i++] = (bool)atoi(tok);
            tok = strtok(NULL, " ");
        }
        DEBUG_MSG("NUM coils = %d\n",i);
        ctx->reg_list[ctx->reg_index].num_coils = i;
        
    } else if (CHECK_STR(def_str,"coil")) {
        sscanf(def_str, "%15s%50s%hu%10s",type,name,&reg,value);
        ctx->reg_list[ctx->reg_index].type = mb_coil;
		ctx->reg_list[ctx->reg_index].uint8_val = (uint8_t)atoi(value);
    }
    
    ctx->reg_list[ctx->reg_index].address = reg;
    strncpy(ctx->reg_list[ctx->reg_index].name,name,50);
    ctx->reg_list[ctx->reg_index].conversion = conversion;
    
    ctx->reg_index++;
    DEBUG_MSG("Type: %s\nName: %s\nReg: %u\nValue: %s\nConversion: %f\n",type,name,reg,value,conversion);
    
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
                case mb_int8:
                    rc = modbus_read_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, (uint16_t *)&ctx->reg_list[i].int8_val);
                break;
                case mb_int16:
                    rc = modbus_read_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, (uint16_t *)&ctx->reg_list[i].int16_val);
                break;
                case mb_int32:
                    rc = modbus_read_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 2, (uint16_t *)&ctx->reg_list[i].int32_val);
                break;
                case mb_uint8:
                    rc = modbus_read_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, (uint16_t *)&ctx->reg_list[i].int8_val);
                break;
                case mb_uint16:
                    rc = modbus_read_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 1, &ctx->reg_list[i].uint16_val);
                break;
                case mb_uint32:
                    rc = modbus_read_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 2, (uint16_t *)&ctx->reg_list[i].uint32_val);
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
			uint16_t tmp_float[2];
	        DEBUG_MSG("Writing: reg: %hu\n",ctx->reg_list[i].address);
            switch(ctx->reg_list[i].type) {
                case mb_uint16:
                    rc = modbus_write_register(ctx->modbus_ctx, ctx->reg_list[i].address, ctx->reg_list[i].uint16_val);
                break;
                case mb_uint32:
                    rc = modbus_write_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 2,(uint16_t*)&ctx->reg_list[i].uint32_val);
                break;
                case mb_int16:
                    rc = modbus_write_register(ctx->modbus_ctx, ctx->reg_list[i].address, (uint16_t)ctx->reg_list[i].int16_val);
                break;
                case mb_int8:
                    rc = modbus_write_register(ctx->modbus_ctx, ctx->reg_list[i].address, (uint16_t)ctx->reg_list[i].int8_val);
                break;
                case mb_uint8:
                    rc = modbus_write_register(ctx->modbus_ctx, ctx->reg_list[i].address, (uint16_t)ctx->reg_list[i].int8_val);
                break;
                case mb_float:
                    modbus_set_float(ctx->reg_list[i].float_val,tmp_float);
                    rc = modbus_write_registers(ctx->modbus_ctx, ctx->reg_list[i].address, 2, tmp_float);
                break;
                case mb_float_cdab:
                case mb_float_array:
                DEBUG_MSG("writing float not implemented");
                exit(1);
                break;
                case mb_coil:
                    rc = modbus_write_bit(ctx->modbus_ctx, ctx->reg_list[i].address, ctx->reg_list[i].uint8_val);
                break;
                case mb_coils:
                    rc = modbus_write_bits(ctx->modbus_ctx, ctx->reg_list[i].address, ctx->reg_list[i].num_coils,(uint8_t *)ctx->reg_list[i].coil_array);
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
        if (ctx->reg_list[i].convert && ctx->reg_list[i].type != mb_float_array) {
            ctx->reg_list[i].float_val *= ctx->reg_list[i].conversion;
            printf("\t\t\"%s\":%f", ctx->reg_list[i].name, ctx->reg_list[i].float_val);
        } else {
            switch(ctx->reg_list[i].type) {
                case mb_int8:
                    printf("\t\t\"%s\":%hhd", ctx->reg_list[i].name, ctx->reg_list[i].int8_val);
                break;
                case mb_int16:
                    //if (ctx->reg_list[i].convert) ctx->reg_list[i].int16_val *= ctx->reg_list[i].conversion;
                    printf("\t\t\"%s\":%hd", ctx->reg_list[i].name, ctx->reg_list[i].int16_val);
                break;
                case mb_int32:
                    //if (ctx->reg_list[i].convert) ctx->reg_list[i].int32_val *= ctx->reg_list[i].conversion;
                    printf("\t\t\"%s\":%d", ctx->reg_list[i].name, ctx->reg_list[i].int32_val);
                break;
                case mb_uint8:
                    //if (ctx->reg_list[i].convert) ctx->reg_list[i].uint8_val *= ctx->reg_list[i].conversion;
                    printf("\t\t\"%s\":%hhu", ctx->reg_list[i].name, ctx->reg_list[i].uint8_val );
                break;
                case mb_uint16:
                    //if (ctx->reg_list[i].convert) ctx->reg_list[i].uint16_val *= ctx->reg_list[i].conversion;
                    printf("\t\t\"%s\":%hu", ctx->reg_list[i].name, ctx->reg_list[i].uint16_val);
                break;
                case mb_uint32:
                    //if (ctx->reg_list[i].convert) ctx->reg_list[i].uint32_val *= ctx->reg_list[i].conversion;
                    printf("\t\t\"%s\":%u", ctx->reg_list[i].name, ctx->reg_list[i].uint32_val);
                break;
                case mb_float:
                case mb_float_cdab:
                    //if (ctx->reg_list[i].convert) ctx->reg_list[i].float_val *= ctx->reg_list[i].conversion;
                    printf("\t\t\"%s\":%f", ctx->reg_list[i].name, ctx->reg_list[i].float_val);
                break;
                case mb_float_array:
                    printf("\t\t\"%s\":[", ctx->reg_list[i].name);
                    for (int j=0;j<ctx->reg_list[i].conversion;j++) {
                        printf("%f",modbus_get_float(ctx->reg_list[i].float_array+(j*2)));
                        if (j != ctx->reg_list[i].conversion - 1) {
                			printf(",");
                		}
                    }
                    printf("]");
                    // Cleanup memory
                    free(ctx->reg_list[i].float_array);
                break;
                case mb_coil:
                    printf("\t\t\"%s\":%hhu", ctx->reg_list[i].name, ctx->reg_list[i].uint8_val);
                break;
                case mb_coils:
                    printf("\t\t\"%s\":[", ctx->reg_list[i].name);
                    for (int j=0;j<ctx->reg_list[i].num_coils;j++) {
                        printf("%hhu",(uint8_t)ctx->reg_list[i].coil_array[j]);
                        if (j != ctx->reg_list[i].num_coils - 1) {
                			printf(",");
                		}
                    }
                    printf("]");
                break;
                default:
                break;
            }
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
		DEBUG_MSG("Reading config\n");
     	/* call the config file parser */
    	if (cmdline_parser_config_file(args_info.conf_file_arg, &args_info, params) != 0) exit(1);
	}
    
    if (args_info.inputs_num != 1) {
        cmdline_parser_print_help();
        exit(1);
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
	
	DEBUG_MSG("Connecting to: %s\n",args_info.inputs[0]);
	mb_instance.modbus_ctx = modbus_new_tcp(args_info.inputs[0], args_info.port_arg);
	if (mb_instance.modbus_ctx == NULL) {
	    DEBUG_MSG("Unable to create the libmodbus context\n");
	    return -1;
	}
	modbus_set_slave(mb_instance.modbus_ctx, 1);
	modbus_set_debug(mb_instance.modbus_ctx,args_info.debug_flag);
    
	
	struct timeval rt;
	
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