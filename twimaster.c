
#include <inttypes.h>
#include <compat/twi.h>
#include <avr/io.h>
#include <i2cmaster.h>


/* define CPU frequency in hz here if not defined in Makefile */
#ifndef F_CPU
#define F_CPU 4000000UL
#endif

/* I2C clock in Hz */
#define SCL_CLOCK  100000L


/*************************************************************************
 Initialization of the I2C bus interface. Need to be called only once
*************************************************************************/
void i2c_init(void)
{
  TWI0.MCTRLA = TWI_ENABLE_bm;                                        // Enable as master, no interrupts
  TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;

}/* i2c_init */


/*************************************************************************	
  Issues a start condition and sends address and transfer direction.
  return 0 = device accessible, 1= failed to access device
*************************************************************************/
unsigned char i2c_start(unsigned char address)
{
	TWI0.MCTRLB |= TWI_FLUSH_bm; //flush
	TWI0.MADDR = address;                                     // Send START condition and address
	while (!(TWI0.MSTATUS & (TWI_WIF_bm | TWI_RIF_bm)));                // Wait for write or read interrupt flag
	if ((TWI0.MSTATUS & (TWI_ARBLOST_bm | TWI_BUSERR_bm)) return false;                  // Return false if arbitration lost or bus error
	return !(TWI0.MSTATUS & TWI_RXACK_bm);                              // Return true if slave gave an ACK

}/* i2c_start */


/*************************************************************************
 Issues a start condition and sends address and transfer direction.
 If device is busy, use ack polling to wait until device is ready
 
 Input:   address and transfer direction of I2C device
*************************************************************************/
void i2c_start_wait(unsigned char address)
{
    while (!i2c_start(address) )
	{
		while(!(TWI0.MSTATUS & TWI_BUSSTATE_IDLE_gc)); // wait for bus to be idle
	}
}

}/* i2c_start_wait */


/*************************************************************************
 Issues a repeated start condition and sends address and transfer direction 

 Input:   address and transfer direction of I2C device
 
 Return:  0 device accessible
          1 failed to access device
*************************************************************************/
unsigned char i2c_rep_start(unsigned char address)
{
    return i2c_start( address );

}/* i2c_rep_start */


/*************************************************************************
 Terminates the data transfer and releases the I2C bus
*************************************************************************/
void i2c_stop(void)
{
    TWI0.MCTRLB = TWI_MCMD_STOP_gc; 
	
	// wait until stop condition is executed and bus released
	while(TWCR & (1<<TWSTO));

}/* i2c_stop */


/*************************************************************************
  Send one byte to I2C device
  
  Input:    byte to be transfered
  Return:   0 write successful 
            1 write failed
*************************************************************************/
unsigned char i2c_write( unsigned char data )
{	
    uint8_t   twst;
    
	// send data to the previously addressed device
	TWI0.MDATA = data;

	while (!(TWI0.MSTATUS & TWI_WIF_bm)); //Wait for transmission to complete

	return !(TWI0.MSTATUS & TWI_RXACK_bm); 

}/* i2c_write */


/*************************************************************************
 Read one byte from the I2C device, request more data from device 
 
 Return:  byte read from I2C device
*************************************************************************/
unsigned char i2c_readAck(void)
{
	while (!(TWI0.MSTATUS & TWI_RIF_bm)); // Wait for read interrupt flag
	uint8_t data = TWI0.MDATA;
	TWI0.MCTRLB = TWI_MCMD_RECVTRANS_gc; //return ack
	return data;
}/* i2c_readAck */


/*************************************************************************
 Read one byte from the I2C device, read is followed by a stop condition 
 
 Return:  byte read from I2C device
*************************************************************************/
unsigned char i2c_readNak(void)
{
	while (!(TWI0.MSTATUS & TWI_RIF_bm));                               // Wait for read interrupt flag
	uint8_t data = TWI0.MDATA;
	TWI0.MCTRLB = TWI_ACKACT_bm | TWI_MCMD_RECVTRANS_gc;           // Send NAK
	return data;
}/* i2c_readNak */
