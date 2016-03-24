#ifndef JSON_MODBUS_H
#define JSON_MODBUS_H
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
  mb_uint32,
  mb_int8,
  mb_int16,
  mb_int32,
  mb_coil,
  mb_coils
};

struct gengetopt_args_info args_info;

typedef struct reg_list {
	char name[50];
	float float_val;
    uint16_t * float_array;
    bool coil_array[255];
    uint8_t num_coils;
    int8_t int8_val;
    int16_t int16_val;
	int32_t int32_val;
	uint8_t uint8_val;
    uint16_t uint16_val;
    uint32_t uint32_val;
	enum mb_data_type type;
	char unit[10];
	double conversion;
	uint16_t address;
    char rw;
    bool convert;
} reg_list_t;

struct mb_util_ctx {
    reg_list_t * reg_list;
    uint16_t reg_index;
    modbus_t *modbus_ctx;
	char name[51];
	char rw;
  bool swap;
  bool include_date;
  int (*read_regs)(modbus_t *, int addr, int nb, uint16_t *dest);

};

#endif
