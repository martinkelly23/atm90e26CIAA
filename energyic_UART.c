 /* ATM90E26 librería

    The MIT License (MIT)

  Copyright (c) 2017 Kelly Martin

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "energyic_UART.h"


unsigned short CommEnergyIC(unsigned char RW,unsigned char address, unsigned short val)
{
  unsigned short output;
  //Set read write flag
  address|=RW<<7;

  unsigned char host_chksum = address;
  if(!RW) //Si es una operacion de escritura en registro
  {
    unsigned short chksum_short = (val>>8) + (val&0xFF) + address;
    host_chksum = chksum_short & 0xFF;
  }

  //begin UART command
  uartWriteByte(UART_232, 0xFE); //primer byte enviado para que el ATM90E26 detecte el baud rate.
  uartWriteByte(UART_232, address); //segundo byte enviado, RW_ADDRESS, which has a R/W bit (bit7) and 7 address bits (bit6-0).


  if(!RW) //Si es una operacion de escritura en registro
  {
	  unsigned char MSBWrite = val>>8;
	  unsigned char LSBWrite = val&0xFF;

      uartWriteByte(UART_232, MSBWrite);
      uartWriteByte(UART_232, LSBWrite);
  }
  uartWriteByte(UART_232, host_chksum);
  delay(10);

  //Read register only
  if(RW) //Operacion de lectura
  {
	  unsigned char MSByte;
	  unsigned char LSByte;
	  unsigned char atm90_chksum;

    uartReadByte(UART_232, &MSByte);
    uartReadByte(UART_232, &LSByte);
    uartReadByte(UART_232, &atm90_chksum);

    if(atm90_chksum == ((LSByte + MSByte) & 0xFF))
    {
      output=(MSByte << 8) | LSByte; //join MSB and LSB;
      return output;
    }
    uartWriteString(UART_USB, "Falló la lectura");
    delay(20); // Delay from failed transaction
    return 0xFFFF;
  }

  //Write register only
  else //Operacion de escritura
  {
	  unsigned char atm90_chksum;
    uartReadByte(UART_232, &atm90_chksum);

    if(atm90_chksum != host_chksum)
    {
    	uartWriteString(UART_USB, "Falló la escritura");
    	delay(20); // Delay from failed transaction
    }
  }
  return 0xFFFF;
}


double  GetLineVoltage(){
	unsigned short voltage=CommEnergyIC(1,Urms,0xFFFF);
	return (double)voltage/100;
}

unsigned short  GetMeterStatus(){
  return CommEnergyIC(1,EnStatus,0xFFFF);
}

double GetLineCurrent(){
	unsigned short current=CommEnergyIC(1,Irms,0xFFFF);
	return (double)current/1000;
}

double GetActivePower(){
	short int apower= (short int)CommEnergyIC(1,Pmean,0xFFFF); //Complement, MSB is signed bit
	return (double)apower;
}

double GetFrequency(){
	unsigned short freq=CommEnergyIC(1,Freq,0xFFFF);
	return (double)freq/100;
}

double GetPowerFactor(){
	short int pf= (short int)CommEnergyIC(1,PowerF,0xFFFF); //MSB is signed bit
	//if negative
	if(pf&0x8000){
		pf=(pf&0x7FFF)*-1;
	}
	return (double)pf/1000;
}

double GetImportEnergy(){
	//Register is cleared after reading
	unsigned short ienergy=CommEnergyIC(1,APenergy,0xFFFF);
	return (double)ienergy/10/1000; //returns kWh if PL constant set to 1000imp/kWh
}

double GetExportEnergy(){
	//Register is cleared after reading
	unsigned short eenergy=CommEnergyIC(1,ANenergy,0xFFFF);
	return (double)eenergy/10/1000; //returns kWh if PL constant set to 1000imp/kWh
}

unsigned short GetSysStatus(){
	return CommEnergyIC(1,SysStatus,0xFFFF);
}


void InitEnergyIC(){
	unsigned short systemstatus;

	uartConfig( UART_232, 9600 ); //Configuramos la UART que vamos a utilzar con el baudRate

	CommEnergyIC(0,SoftReset,0x789A); //Perform soft reset
	CommEnergyIC(0,FuncEn,0x0030); //Voltage sag irq=1, report on warnout pin=1, energy dir change irq=0
	CommEnergyIC(0,SagTh,0x1F2F); //Voltage sag threshhold


	//Set metering calibration values
	CommEnergyIC(0,CalStart,0x5678); //Metering calibration startup command. Register 21 to 2B need to be set
	CommEnergyIC(0,PLconstH,0x00B9); //PL Constant MSB
	CommEnergyIC(0,PLconstL,0xC1F3); //PL Constant LSB
	CommEnergyIC(0,Lgain,0x1D39); 	//Line calibration gain
	CommEnergyIC(0,Lphi,0x0000); //Line calibration angle
	CommEnergyIC(0,PStartTh,0x08BD); //Active Startup Power Threshold
	CommEnergyIC(0,PNolTh,0x0000); //Active No-Load Power Threshold
	CommEnergyIC(0,QStartTh,0x0AEC); //Reactive Startup Power Threshold
	CommEnergyIC(0,QNolTh,0x0000); //Reactive No-Load Power Threshold
	CommEnergyIC(0,MMode,0x9422); //Metering Mode Configuration. All defaults. See pg 31 of datasheet.
	CommEnergyIC(0,CSOne,0x4A34); //Write CSOne, as self calculated

	uartWriteString(UART_USB, "Checksum 1:");
	uartWriteString(UART_USB, CommEnergyIC(1,CSOne,0x0000)); //HEX Checksum 1. Needs to be calculated based off the above values.

	//Set measurement calibration values
	CommEnergyIC(0,AdjStart,0x5678); //Measurement calibration startup command, registers 31-3A
	CommEnergyIC(0,Ugain,0xD464);    //Voltage rms gain
	CommEnergyIC(0,IgainL,0x6E49);   //L line current gain
	CommEnergyIC(0,Uoffset,0x0000);  //Voltage offset
	CommEnergyIC(0,IoffsetL,0x0000); //L line current offset
	CommEnergyIC(0,PoffsetL,0x0000); //L line active power offset
	CommEnergyIC(0,QoffsetL,0x0000); //L line reactive power offset
	CommEnergyIC(0,CSTwo,0xD294); //Write CSTwo, as self calculated

	uartWriteString(UART_USB, "Checksum 2:");
	uartWriteString(UART_USB, CommEnergyIC(1,CSTwo,0x0000));  //HEX Checksum 2. Needs to be calculated based off the above values.

	CommEnergyIC(0,CalStart,0x8765); //Checks correctness of 21-2B registers and starts normal metering if ok
	CommEnergyIC(0,AdjStart,0x8765); //Checks correctness of 31-3A registers and starts normal measurement  if ok

	systemstatus = GetSysStatus();

	if (systemstatus&0xC000){
		//checksum 1 error
		uartWriteString(UART_USB, "Checksum 1 Error!!");
	}
	if (systemstatus&0x3000){
		//checksum 2 error
		uartWriteString(UART_USB, "Checksum 2 Error!!");
	}
}
