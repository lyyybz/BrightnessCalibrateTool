 
#ifndef __I2C_H__
#define __I2C_H__ 
#include "sys.h"


#define SCL_PIN     GPIO_Pin_10
#define SDA_PIN     GPIO_Pin_11

void start_i2c(void);   
void stop_i2c(void);   
void i2c_no_ack(void);    
void i2c_ack(void); 
int i2c_write(unsigned char DData);   
int i2c_read(void);  

#endif 
