// This file is Copyright (c) 2020 Florent Kermarrec <florent@enjoy-digital.fr>
// License: BSD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <irq.h>
#include <libbase/uart.h>
#include <libbase/console.h>
#include <generated/csr.h>

/*-----------------------------------------------------------------------*/
/* Adder Peripheral                                                      */
/*-----------------------------------------------------------------------*/


static void adder_set_operand_a(uint32_t value) {
    adder_operand_a_write(value);
}

static void adder_set_operand_b(uint32_t value) {
    adder_operand_b_write(value);
}

static uint32_t adder_get_result(void) {
    return adder_result_read();
}

static uint32_t adder_add(uint32_t a, uint32_t b) {
    adder_set_operand_a(a);
    adder_set_operand_b(b);
    return adder_get_result();
}


/*-----------------------------------------------------------------------*/
/* Uart                                                                  */
/*-----------------------------------------------------------------------*/

static char *readstr(void)
{
	char c[2];
	static char s[64];
	static int ptr = 0;

	if(readchar_nonblock()) {
		c[0] = getchar();
		c[1] = 0;
		switch(c[0]) {
			case 0x7f:
			case 0x08:
				if(ptr > 0) {
					ptr--;
					fputs("\x08 \x08", stdout);
				}
				break;
			case 0x07:
				break;
			case '\r':
			case '\n':
				s[ptr] = 0x00;
				fputs("\n", stdout);
				ptr = 0;
				return s;
			default:
				if(ptr >= (sizeof(s) - 1))
					break;
				fputs(c, stdout);
				s[ptr] = c[0];
				ptr++;
				break;
		}
	}

	return NULL;
}

static char *get_token(char **str)
{
	char *c, *d;

	c = (char *)strchr(*str, ' ');
	if(c == NULL) {
		d = *str;
		*str = *str+strlen(*str);
		return d;
	}
	*c = 0;
	d = *str;
	*str = c+1;
	return d;
}

static void prompt(void)
{
	printf("\e[92;1mBruno-luiz-app\e[0m> ");
}

/*-----------------------------------------------------------------------*/
/* Help                                                                  */
/*-----------------------------------------------------------------------*/

static void help(void)
{
	puts("\nLiteX minimal demo app built "__DATE__" "__TIME__"\n");
	puts("Available commands:");
	puts("help                              - Show this command");
	puts("reboot                            - Reboot CPU");
	puts("hello                             - Hello world");
    puts("add <numberA> <numberB>    - Add two integers number");
}


/*-----------------------------------------------------------------------*/
/* Commands                                                              */
/*-----------------------------------------------------------------------*/

static void reboot_cmd(void)
{
	ctrl_reset_write(1);
}


static void helloc_cmd(void)
{
	printf("Hello C demo...\n");
}


/*-----------------------------------------------------------------------*/
/* Console service / Main                                                */
/*-----------------------------------------------------------------------*/

static void console_service(void)
{
	char *str;
	char *token;

	str = readstr();
	if(str == NULL) return;
	token = get_token(&str);
	if(strcmp(token, "help") == 0)
    {
		help();
    } 
    else if(strcmp(token, "reboot") == 0)
    {
		reboot_cmd();
    } 
    else if(strcmp(token, "hello") == 0) 
    {
		helloc_cmd();
    } 
    else if(strcmp(token, "add") == 0)
    {
        char *arg_a = get_token(&str);
        char *arg_b = get_token(&str);

        if(arg_a == NULL || arg_b == NULL) {
            printf("Usage: add_uint32 <numberA> <numberB>\n");
            return;
        }

        uint32_t a = strtoul(arg_a, NULL, 0);
        uint32_t b = strtoul(arg_b, NULL, 0);
        uint32_t result = adder_add(a, b);

        printf("Result: 0x%08lx (%lu)\n", result, result);
    }
	prompt();
}

int main(void)
{
#ifdef CONFIG_CPU_HAS_INTERRUPT
	irq_setmask(0);
	irq_setie(1);
#endif
	uart_init();

	help();
	prompt();

	while(1) {
		console_service();
	}

	return 0;
}
