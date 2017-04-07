/*
 * modified version of I2C master library
 * added a timeout variable for non blocking i2c
 *
 * 06.04.2016:
 * 	- Fixed timeout handling!
 * 	- Routines return int16_t; in case of failure -1
 */

/*************************************************************************
* Title:    I2C master library using hardware TWI interface
* Author:   Peter Fleury <pfleury@gmx.ch>  http://jump.to/fleury
* File:     $Id: twimaster.c,v 1.3 2005/07/02 11:14:21 Peter Exp $
* Software: AVR-GCC 3.4.3 / avr-libc 1.2.3
* Target:   any AVR device with hardware TWI 
* Usage:    API compatible with I2C Software Library i2cmaster.h
**************************************************************************/
#include "i2cmaster_V2.h"

#include <inttypes.h>
#include <compat/twi.h>
#include <stdio.h>



/* Compiler-Vorgang abbrechen, wenn Prozessortakt nicht definiert F_CPU */
#ifndef F_CPU
#error "F_CPU not defined!"
#endif

/* I2C clock in Hz */
#define SCL_CLOCK  400000L
/* Baudrate-Register berechnen */
#define TWBR_VAL ((F_CPU/SCL_CLOCK)-16)/2
#if (TWBR_VAL <= 10)
	#error "TWBR must be >10 for stable operation"
#endif

/* I2C timeout Wert. uint32_t -> Gr��ere Werte = l�ngeres Warten*/
#define I2C_TIMER_DELAY 0xFFFF

/*************************************************************************
 Initialization of the I2C bus interface. Need to be called only once
*************************************************************************/
void i2c_init(void)
{
  TWSR = 0;                         /* no prescaler */
  TWBR = TWBR_VAL;  /* must be > 10 for stable operation */
}/* i2c_init */


/*************************************************************************	
  Issues a start condition and sends address and transfer direction.
  return 0 = device accessible, -1= failed to access device
*************************************************************************/
int16_t i2c_start(unsigned char address)
{
	uint32_t  i2c_timer = 0;
    uint8_t   twst;

	// send START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

	// wait until transmission completed
	i2c_timer = I2C_TIMER_DELAY;
	while(!(TWCR & (1<<TWINT)) && (--i2c_timer) );
	if(i2c_timer == 0)
	{
		//printf("ERROR: I2C Timeout (i2c_start:1)\n\n");
		return -1;
	}

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;

	// send device address
	TWDR = address;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// wail until transmission completed and ACK/NACK has been received
	i2c_timer = I2C_TIMER_DELAY;
	while(!(TWCR & (1<<TWINT)) && (--i2c_timer) );
	if(i2c_timer == 0)
	{
		//printf("ERROR: I2C Timeout (i2c_start:2)\n\n");
		return -1;
	}

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) )
	{
		//printf("ERROR: I2C Error (i2c_start:3)\n\n");
		return -1;
	}

	return 0;

}/* i2c_start */


/*************************************************************************
 Issues a start condition and sends address and transfer direction.
 If device is busy, use ack polling to wait until device is ready
 
 Input:   address and transfer direction of I2C device
*************************************************************************/
void i2c_start_wait(unsigned char address)
{
	uint32_t  i2c_timer = 0;
	uint8_t   twst;

    while ( 1 )
    {
	    // send START condition
	    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
    
    	// wait until transmission completed
	    i2c_timer = I2C_TIMER_DELAY;
    	while(!(TWCR & (1<<TWINT)) && (--i2c_timer));

    	// check value of TWI Status Register. Mask prescaler bits.
    	twst = TW_STATUS & 0xF8;
    	if ( (twst != TW_START) && (twst != TW_REP_START)) continue;
    
    	// send device address
    	TWDR = address;
    	TWCR = (1<<TWINT) | (1<<TWEN);
    
    	// wail until transmission completed
    	i2c_timer = I2C_TIMER_DELAY;
    	while(!(TWCR & (1<<TWINT)) && (--i2c_timer) );
    
    	// check value of TWI Status Register. Mask prescaler bits.
    	twst = TW_STATUS & 0xF8;
    	if ( (twst == TW_MT_SLA_NACK )||(twst ==TW_MR_DATA_NACK) ) 
    	{    	    
    	    /* device busy, send stop condition to terminate write operation */
	        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	        
	        // wait until stop condition is executed and bus released
	        i2c_timer = I2C_TIMER_DELAY;
	        while((TWCR & (1<<TWSTO)) && i2c_timer--);
	        
    	    continue;
    	}
    	//if( twst != TW_MT_SLA_ACK) return 1;
    	break;
     }

}/* i2c_start_wait */


/*************************************************************************
 Issues a repeated start condition and sends address and transfer direction 

 Input:   address and transfer direction of I2C device
 
 Return:  0 device accessible
          -1 failed to access device
*************************************************************************/
int16_t i2c_rep_start(unsigned char address)
{
    return i2c_start( address );
}/* i2c_rep_start */


/*************************************************************************
 Terminates the data transfer and releases the I2C bus
*************************************************************************/
int16_t i2c_stop(void)
{
	uint32_t  i2c_timer = 0;

    /* send stop condition */
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	
	// wait until stop condition is executed and bus released
	i2c_timer = I2C_TIMER_DELAY;
	while((TWCR & (1<<TWSTO)) && (--i2c_timer));
	if (i2c_timer==0)
	{
		//printf("ERROR: I2C Timeout (i2c_stop:1)\n\n");
		return -1;
	}
	return 0;
}/* i2c_stop */


/*************************************************************************
  Send one byte to I2C device
  
  Input:    byte to be transfered
  Return:   0 write successful 
            -1 write failed
*************************************************************************/
int16_t i2c_write( unsigned char data )
{	
	uint32_t  i2c_timer = 0;
    uint8_t   twst;
    
	// send data to the previously addressed device
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// wait until transmission completed
	i2c_timer = I2C_TIMER_DELAY;
	while(!(TWCR & (1<<TWINT)) && (--i2c_timer));
	if (i2c_timer==0)
	{
		printf("ERROR: I2C Timeout (i2c_write:1)\n\n");
		return -1;
	}

	// check value of TWI Status Register. Mask prescaler bits
	twst = TW_STATUS & 0xF8;
	if( twst != TW_MT_DATA_ACK)
	{
		printf("ERROR: I2C Timeout (i2c_write:2)\n\n");
		return -1;
	}

	return 0;

}/* i2c_write */


/*************************************************************************
 Read one byte from the I2C device, request more data from device 
 
 Return:  byte read from I2C device
*************************************************************************/
int16_t i2c_readAck(void)
{
	uint32_t  i2c_timer = 0;

	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	i2c_timer = I2C_TIMER_DELAY;
	while(!(TWCR & (1<<TWINT)) && (--i2c_timer));
	if (i2c_timer==0)
	{
		printf("ERROR: I2C Timeout (i2c_readAck:1)\n\n");
		return -1;
	}

    return (int16_t)TWDR;

}/* i2c_readAck */


/*************************************************************************
 Read one byte from the I2C device, read is followed by a stop condition 
 
 Return:  byte read from I2C device
*************************************************************************/
int16_t i2c_readNak(void)
{
	uint32_t  i2c_timer = 0;

	TWCR = (1<<TWINT) | (1<<TWEN);
	i2c_timer = I2C_TIMER_DELAY;
	while(!(TWCR & (1<<TWINT)) && (--i2c_timer));
	if (i2c_timer==0)
	{
		printf("ERROR: I2C Timeout (i2c_readNak:1)\n\n");
		return -1;
	}
	
    return (int16_t)TWDR;

}/* i2c_readNak */
