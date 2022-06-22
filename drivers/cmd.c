#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
//#include <core_cm0.h>
#include "config.h"
//#include "my_type.h"
#include "ymodem.h"
#include "comfunc.h"
#include "delay.h"
#include "cmd.h"
#include "uart.h"
#include "dbug.h"
#include "uart_fifo.h"
#include "config.h"

#if CONFIG_DEBUG 


typedef enum{ FALSE = 0, TRUE = !FALSE } bool;

static const char PROMPT[] = "BOOT> ";
static const char SYNTAX[] = "Error: Invalid syntax for: %s\r\n";
static const char CMDERR[] = "Error: Invalid cmd format: %s\r\n";
const char INVCMD[] = "Error: No such command: %s\r\n";

static char cmdline1[UIF_MAX_LINE];
static char cmdline2[UIF_MAX_LINE];

 int	uif_cmd_help(int argc, char **argv);
 int	uif_cmd_show(int argc, char **argv);
 int	uif_cmd_init(int argc, char **argv);
 int	uif_cmd_reboot(int argc, char **argv); 

#define PARAM_INIT_FLAG			0x1236
#define PARAM_ADDR		((u32)0x0800FC00)

 
 int uif_cmd_dn(int argc, char **argv)
{
	int ret;
 
	ret = Ymodem_receive(EMBED_PARA_ADDR,SIZE_1M*2);
	
	if (ret > 0)
	{
		delay_ms(1000); 
	}
	
	return ret;
}
int uif_cmd_reset(int argc, char **argv)
{
	hard_reset_now();
	return 0;
}

int  flash_dump(int argc, char *argv[]);

int  dn_image(int argc, char *argv[])
{
	int ret;

	ret = Ymodem_receive(TEST_IMG_ADDR, TEST_IMG_SIZE);
	if (ret > 0)
	{
		delay_ms(1000); 
	}
	
	return ret;

}



const uif_cmd_t uif_cmdtab[] =
{
	{"dncnn", 1, 0, uif_cmd_dn,  "<dn>","down flash 2M or small tiny flash"},
    {"dnimage",1,0,dn_image,"download image"},
	{"reset", 1, 0, uif_cmd_reset,  "<reset>","reset"},
    {"dump", 3, 0,flash_dump,"dum flash"},
	{"help",  1, 0, uif_cmd_help,"<help>","show help message"}, 
};

#define UIF_NUM_CMD ARRAY_SIZE(uif_cmdtab)

char * get_history_line(char *userline)
{
	char line[UIF_MAX_LINE];
	int pos, ch;

	pos = 0;

	ch = (int) board_getchar();
	while ((ch != 0x0D /* CR */) && (ch != 0x0A /* LF/NL */) && (pos < UIF_MAX_LINE))
	{
		switch (ch)
		{
		case 0x08:
			/* Backspace */
		case 0x7F:
			/* Delete */
			if (pos > 0)
			{
				pos -= 1;
				board_putchar(0x08);	/* backspace */
				board_putchar(' ');
				board_putchar(0x08);	/* backspace */
			}
			break;

		default:
			if ((pos + 1) < UIF_MAX_LINE)
			{
				/* only printable characters */
				if ((ch > 0x1f) && (ch < 0x80))
				{
					line[pos++] = (char) ch;
					board_putchar((char) ch);
				}
			}
			break;
		} 
		ch = (int) board_getchar();
	}
	line[pos] = '\0';
	board_putchar(0x0D);	/* CR */
	board_putchar(0x0A);	/* LF */

	strcpy(userline, line);

	return userline;
}

/********************************************************************/
int make_argv(char *cmdline, char *argv[])
{
	int argc, i, in_text;

	/* break cmdline into strings and argv */
	/* it is permissible for argv to be NULL, in which case */
	/* the purpose of this routine becomes to count args */
	argc = 0;
	i = 0;
	in_text = FALSE;
	while (cmdline[i] != '\0')  /* getline() must place 0x00 on end */
	{
		if (((cmdline[i] == ' ') || (cmdline[i] == '\t')))
		{
			if (in_text)
			{
				/* end of command line argument */
				cmdline[i] = '\0';
				in_text = FALSE;
			}
		}
		else
		{
			/* got non-whitespace character */
			if (!in_text)
			{
				/* start of an argument */
				in_text = TRUE;
				if (argc < UIF_MAX_ARGS)
				{
					if (argv != NULL)
						argv[argc] = &cmdline[i];
					argc++;
				}
				else
									/*return argc;*/
					break;
			}
		}
		i++;	/* proceed to next character */
	}
	if (argv != NULL)
	{
		argv[argc] = NULL;
	}
	return(argc);
}

/********************************************************************/
void run_cmd(void)
{
	int i;  
	/*
	 * Global array of pointers to emulate C argc,argv interface
	 */
	//extern char *get_history_line (char *userline);
	int argc, result;
	char *argv[UIF_MAX_ARGS + 1];   /* one extra for null terminator */

	printD(PROMPT);
	get_history_line(cmdline1);

	if (!(argc = make_argv(cmdline1, argv)))
	{
		/* no command entered, just a blank line */
		strcpy(cmdline1, cmdline2);
		argc = make_argv(cmdline1, argv);
	}
	cmdline2[0] = '\0';

	if (argc)
	{
		/*
		 * First try for an exact match on command name
		 */
		for (i = 0; i < UIF_NUM_CMD; i++)
		{
			result = strcasecmp(uif_cmdtab[i].cmd, argv[0]);
			if (result == 0)
			{
				if (argc != uif_cmdtab[i].max_args)
				{
					printD(SYNTAX, argv[0]);
					return;
				}
				result = uif_cmdtab[i].func(argc, argv);
				if (result <= 0)
					printD(CMDERR, uif_cmdtab[i].usage);
				return;
			}
		}

		printD(INVCMD, argv[0]);
	}
}

int uif_cmd_init(int argc, char **argv)
{
 
	return 1;
}

int uif_cmd_reboot(int argc, char **argv)
{
//	NVIC_SystemReset();
	return 1;
}

int uif_cmd_help(int argc, char **argv)
{
	int i;
	printD("-cmd-----example format----help message--------------------------------------\r\n");
	for (i = 0; i < UIF_NUM_CMD; i++)
	{
		printD(" %-6s  %-16s  %-46s\r\n", uif_cmdtab[i].cmd, uif_cmdtab[i].usage,
			   uif_cmdtab[i].help);
	}
	printD("-ms/ss/ps指令,参数从左至右依次表示串口0-7配置,0-1与DM交互串口,2-7与38交互串口-\r\n");
	printD("------------------------------------------------------------------------------\r\n");
	return 1;
}

int uif_cmd_show(int argc, char **argv)
{
	int i;
	printD("-cmd---param value---------help message--------------------------------------\r\n");
	for (i = 0; i < UIF_NUM_CMD; i++)
	{
		if (uif_cmdtab[i].max_args <= 1)
			continue;

		printD(" %-4s  ", uif_cmdtab[i].cmd);
		uif_cmdtab[i].func(0, NULL);
		printD("%-46s\r\n", uif_cmdtab[i].help);
	} 
	printD("-ms/ss/ps指令,参数从左至右依次表示串口0-7配置,0-1与DM交互串口,2-7与38交互串口-\r\n");
	printD("------------------------------------------------------------------------------\r\n");   	 
	return 1;
}


#endif 

