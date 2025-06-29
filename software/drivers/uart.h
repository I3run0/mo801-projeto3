#ifndef __UART_DRIVER_H
#define __UART_DRIVERL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libbase/console.h>

char *readstr(void);
char *get_token(char **str);

#endif /* __UART_DRIVER_H */