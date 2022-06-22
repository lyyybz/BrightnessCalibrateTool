#ifndef _CMD_H_
#define _CMD_H_

#include <stdint.h>
 

/*
 * Maximum command line arguments
 */
#define UIF_MAX_ARGS    (10)

/*
 * Maximum length of the command line
 */
#define UIF_MAX_LINE    (0x40)

 

/*
 * The command table entry data structure, and the prototype for the
 * command table.
 */
typedef const struct
{
    char   	*cmd;
    int     	max_args;
    uint32_t    flags;
    int			(*func)(int, char **);
    char 		*usage;    
    char 		*help;
} uif_cmd_t;


void cmd_rx_hook(char ch);
void run_cmd(void);
int get_console_status(void);
void set_console_status(int newstate);
void app_param_init(void);
#endif
