/** @file cmdline.h
 *  @brief The header file for the command line option parser
 *  generated by GNU Gengetopt version 2.22.6
 *  http://www.gnu.org/software/gengetopt.
 *  DO NOT modify this file, since it can be overwritten
 *  @author GNU Gengetopt by Lorenzo Bettini */

#ifndef CMDLINE_H
#define CMDLINE_H

/* If we use autoconf.  */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h> /* for FILE */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef CMDLINE_PARSER_PACKAGE
/** @brief the program name (used for printing errors) */
#define CMDLINE_PARSER_PACKAGE "json_modbus"
#endif

#ifndef CMDLINE_PARSER_PACKAGE_NAME
/** @brief the complete program name (used for help and version) */
#define CMDLINE_PARSER_PACKAGE_NAME "json_modbus"
#endif

#ifndef CMDLINE_PARSER_VERSION
/** @brief the program version */
#define CMDLINE_PARSER_VERSION "1.0.1"
#endif

/** @brief Where the command line options are stored */
struct gengetopt_args_info
{
  const char *help_help; /**< @brief Print help and exit help description.  */
  const char *version_help; /**< @brief Print version and exit help description.  */
  char * host_arg;	/**< @brief IP adress of modbus device. (default='10.0.0.5').  */
  char * host_orig;	/**< @brief IP adress of modbus device. original value given at command line.  */
  const char *host_help; /**< @brief IP adress of modbus device. help description.  */
  int port_arg;	/**< @brief Port of modbus device. (default='502').  */
  char * port_orig;	/**< @brief Port of modbus device. original value given at command line.  */
  const char *port_help; /**< @brief Port of modbus device. help description.  */
  int interval_arg;	/**< @brief Time between measurements in seconds (default='0').  */
  char * interval_orig;	/**< @brief Time between measurements in seconds original value given at command line.  */
  const char *interval_help; /**< @brief Time between measurements in seconds help description.  */
  int debug_flag;	/**< @brief Show protocol debug information (default=off).  */
  const char *debug_help; /**< @brief Show protocol debug information help description.  */
  char * name_arg;	/**< @brief Name of the application (default='modbus').  */
  char * name_orig;	/**< @brief Name of the application original value given at command line.  */
  const char *name_help; /**< @brief Name of the application help description.  */
  int include_date_flag;	/**< @brief add a date to the output (default=off).  */
  const char *include_date_help; /**< @brief add a date to the output help description.  */
  char * conf_file_arg;	/**< @brief Configuration file.  */
  char * conf_file_orig;	/**< @brief Configuration file original value given at command line.  */
  const char *conf_file_help; /**< @brief Configuration file help description.  */
  char ** reg_arg;	/**< @brief Define a register to read or write.  */
  char ** reg_orig;	/**< @brief Define a register to read or write original value given at command line.  */
  unsigned int reg_min; /**< @brief Define a register to read or write's minimum occurreces */
  unsigned int reg_max; /**< @brief Define a register to read or write's maximum occurreces */
  const char *reg_help; /**< @brief Define a register to read or write help description.  */
  int timeout_arg;	/**< @brief Set the response timeout (default='1').  */
  char * timeout_orig;	/**< @brief Set the response timeout original value given at command line.  */
  const char *timeout_help; /**< @brief Set the response timeout help description.  */
  int read_flag;	/**< @brief Read registers (default=on).  */
  const char *read_help; /**< @brief Read registers help description.  */
  int write_flag;	/**< @brief Write registers (default=off).  */
  const char *write_help; /**< @brief Write registers help description.  */
  
  unsigned int help_given ;	/**< @brief Whether help was given.  */
  unsigned int version_given ;	/**< @brief Whether version was given.  */
  unsigned int host_given ;	/**< @brief Whether host was given.  */
  unsigned int port_given ;	/**< @brief Whether port was given.  */
  unsigned int interval_given ;	/**< @brief Whether interval was given.  */
  unsigned int debug_given ;	/**< @brief Whether debug was given.  */
  unsigned int name_given ;	/**< @brief Whether name was given.  */
  unsigned int include_date_given ;	/**< @brief Whether include-date was given.  */
  unsigned int conf_file_given ;	/**< @brief Whether conf-file was given.  */
  unsigned int reg_given ;	/**< @brief Whether reg was given.  */
  unsigned int timeout_given ;	/**< @brief Whether timeout was given.  */
  unsigned int read_given ;	/**< @brief Whether read was given.  */
  unsigned int write_given ;	/**< @brief Whether write was given.  */

  int read_mode_counter; /**< @brief Counter for mode read */
  int write_mode_counter; /**< @brief Counter for mode write */
} ;

/** @brief The additional parameters to pass to parser functions */
struct cmdline_parser_params
{
  int override; /**< @brief whether to override possibly already present options (default 0) */
  int initialize; /**< @brief whether to initialize the option structure gengetopt_args_info (default 1) */
  int check_required; /**< @brief whether to check that all required options were provided (default 1) */
  int check_ambiguity; /**< @brief whether to check for options already specified in the option structure gengetopt_args_info (default 0) */
  int print_errors; /**< @brief whether getopt_long should print an error message for a bad option (default 1) */
} ;

/** @brief the purpose string of the program */
extern const char *gengetopt_args_info_purpose;
/** @brief the usage string of the program */
extern const char *gengetopt_args_info_usage;
/** @brief the description string of the program */
extern const char *gengetopt_args_info_description;
/** @brief all the lines making the help output */
extern const char *gengetopt_args_info_help[];

/**
 * The command line parser
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser (int argc, char **argv,
  struct gengetopt_args_info *args_info);

/**
 * The command line parser (version with additional parameters - deprecated)
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @param override whether to override possibly already present options
 * @param initialize whether to initialize the option structure my_args_info
 * @param check_required whether to check that all required options were provided
 * @return 0 if everything went fine, NON 0 if an error took place
 * @deprecated use cmdline_parser_ext() instead
 */
int cmdline_parser2 (int argc, char **argv,
  struct gengetopt_args_info *args_info,
  int override, int initialize, int check_required);

/**
 * The command line parser (version with additional parameters)
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @param params additional parameters for the parser
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_ext (int argc, char **argv,
  struct gengetopt_args_info *args_info,
  struct cmdline_parser_params *params);

/**
 * Save the contents of the option struct into an already open FILE stream.
 * @param outfile the stream where to dump options
 * @param args_info the option struct to dump
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_dump(FILE *outfile,
  struct gengetopt_args_info *args_info);

/**
 * Save the contents of the option struct into a (text) file.
 * This file can be read by the config file parser (if generated by gengetopt)
 * @param filename the file where to save
 * @param args_info the option struct to save
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_file_save(const char *filename,
  struct gengetopt_args_info *args_info);

/**
 * Print the help
 */
void cmdline_parser_print_help(void);
/**
 * Print the version
 */
void cmdline_parser_print_version(void);

/**
 * Initializes all the fields a cmdline_parser_params structure 
 * to their default values
 * @param params the structure to initialize
 */
void cmdline_parser_params_init(struct cmdline_parser_params *params);

/**
 * Allocates dynamically a cmdline_parser_params structure and initializes
 * all its fields to their default values
 * @return the created and initialized cmdline_parser_params structure
 */
struct cmdline_parser_params *cmdline_parser_params_create(void);

/**
 * Initializes the passed gengetopt_args_info structure's fields
 * (also set default values for options that have a default)
 * @param args_info the structure to initialize
 */
void cmdline_parser_init (struct gengetopt_args_info *args_info);
/**
 * Deallocates the string fields of the gengetopt_args_info structure
 * (but does not deallocate the structure itself)
 * @param args_info the structure to deallocate
 */
void cmdline_parser_free (struct gengetopt_args_info *args_info);

/**
 * The config file parser (deprecated version)
 * @param filename the name of the config file
 * @param args_info the structure where option information will be stored
 * @param override whether to override possibly already present options
 * @param initialize whether to initialize the option structure my_args_info
 * @param check_required whether to check that all required options were provided
 * @return 0 if everything went fine, NON 0 if an error took place
 * @deprecated use cmdline_parser_config_file() instead
 */
int cmdline_parser_configfile (const char *filename,
  struct gengetopt_args_info *args_info,
  int override, int initialize, int check_required);

/**
 * The config file parser
 * @param filename the name of the config file
 * @param args_info the structure where option information will be stored
 * @param params additional parameters for the parser
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_config_file (const char *filename,
  struct gengetopt_args_info *args_info,
  struct cmdline_parser_params *params);

/**
 * Checks that all the required options were specified
 * @param args_info the structure to check
 * @param prog_name the name of the program that will be used to print
 *   possible errors
 * @return
 */
int cmdline_parser_required (struct gengetopt_args_info *args_info,
  const char *prog_name);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CMDLINE_H */
