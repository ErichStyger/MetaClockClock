/*
 * Copyright (c) 2022-2023, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_SHIFT_REGS
#include "shiftreg.h"
#include "spireg.h"
#include "McuWait.h"
#include "McuRTOS.h"
#include "McuLog.h"
#include "McuUtility.h"
#include "McuShell.h"
#include "stepper.h"
#include "stdbool.h"

/* GPIO handles for latching shift registers */
static McuGPIO_Handle_t MotorLatch; /* to latch the motor signals, this pin does a active high pulse */
#if 0 /* not used */
static McuGPIO_Handle_t SensorLatch;  /* to latch the sensor signals, this pin does a active low pulse */
#endif
static SemaphoreHandle_t ShiftRegMutex = NULL; /* Mutex to protect SPI bus access */
static uint8_t MotorBitsByteToSend[SHIFTREG_NOF_MOTORBIT_BYTES]; /* array of bits stored to be sent */
static uint8_t MotorBitsByteToSendWithoutClk[SHIFTREG_NOF_MOTORBIT_BYTES]; /* array of bits stored to be sent */
static uint8_t prevMotorBitsByte[SHIFTREG_NOF_MOTORBIT_BYTES]; /* backup of what has been sent */
static bool dirInv[]= { /*Indicates whether the direction of rotation of the motor is inverted.*/
		false, /*0*/
		false, /*1*/
		false, /*2*/
		false, /*3*/
		false, /*4*/
		false, /*5*/
		false, /*6*/
		false, /*7*/
		false, /*8*/
		false, /*9*/
		false, /*10*/
		false, /*11*/
		false, /*12*/
		false, /*13*/
		false, /*14*/
		false, /*15*/
		true, /*16*/
		true, /*17*/
		true, /*18*/
		true, /*19*/
		true, /*20*/
		true, /*21*/
		true, /*22*/
		true, /*23*/
		true, /*24*/
		true, /*25*/
		true, /*26*/
		true, /*27*/
		true, /*28*/
		true, /*29*/
		true, /*30*/
		true, /*31*/
		true, /*32*/
		true, /*33*/
		true, /*34*/
		true, /*35*/
		true, /*36*/
		true, /*37*/
		true, /*38*/
		true, /*39*/
		true, /*40*/
		true, /*41*/
		true, /*42*/
		true, /*43*/
		true, /*44*/
		true, /*45*/
		true, /*46*/
		true, /*47*/
		true, /*48*/
		true, /*49*/
		true, /*50*/
		true, /*51*/
		true, /*52*/
		true, /*53*/
		true, /*54*/
		true, /*55*/
		true, /*56*/
		true, /*57*/
		true, /*58*/
		true, /*59*/
		true, /*60*/
		true, /*61*/
		true, /*62*/
		true, /*63*/
		false, /*64*/
		false, /*65*/
		true, /*66*/
		true, /*67*/
		true, /*68*/
		true, /*69*/
		true, /*70*/
		true, /*71*/
		true, /*72*/
		true, /*73*/
		true, /*74*/
		true, /*75*/
		true, /*76*/
		true, /*77*/
		true, /*78*/
		true, /*79*/
};

void ShiftReg_WriteMotorBits(const uint8_t *data, size_t nofBytes) {
#if STEPPER_CONFIG_USE_FREERTOS_TIMER
  if (xSemaphoreTake(ShiftRegMutex, portMAX_DELAY)==pdTRUE) {
#endif
    SpiReg_WriteData(data, nofBytes);
    /* latch: idle state is LOW, make a HIGH pulse: */
    McuGPIO_SetHigh(MotorLatch);
    McuWait_Wait10Cycles();
    McuGPIO_SetLow(MotorLatch);
#if STEPPER_CONFIG_USE_FREERTOS_TIMER
    xSemaphoreGive(ShiftRegMutex);
  }
#endif
}

#if 0 /* not used */
void ShiftReg_ReadSensorBits(uint8_t *data, size_t dataSize) {
  if (xSemaphoreTake(ShiftRegMutex, portMAX_DELAY)==pdTRUE) {
    /* latch: LOW pulse latches the data */
    McuGPIO_SetLow(SensorLatch);
    McuWait_Wait10Cycles();
    McuGPIO_SetHigh(SensorLatch); /* now read the data */

    SpiReg_ReadData(data, dataSize);
    xSemaphoreGive(ShiftRegMutex);
  }
}
#endif

void ShiftReg_StoreMotorBits(uint32_t motorIdx, const bool w[SHIFTREG_NOF_MOTOR_BITS]) {
  /* w[0]: CLK
   * w[1]: DIR
   * w[2]: STDBY
   */
  bool b[SHIFTREG_NOF_MOTOR_BITS];
  for(int i = 0; i < SHIFTREG_NOF_MOTOR_BITS; i++){
	  b[i]=w[i];
  }

  size_t byteIdx;

  /* One driver board:
   * Each motor has 3 bits
   * MotorBitsByteToSend: [5]        [4]        [3]        [2]        [1]        [0]
   * MotorIdx             ddee'efff  abbb'cccd  8889'99aa  5566'6777  2333'4445  0001'1122  ==> shift (MSB first!)
   * clock/dir/stdby	  dscd scds  scds cdsc  cdsc dscd  dscd scds  scds cdsc  cdsc dscd
   */

  /*Invert direction bit if the motor rotation is inverted. (true=CW | false=CCW)*/
  if(dirInv[motorIdx]){
	  b[1]=!b[1];
  }

  /* transform truth table for motor wires/connections into bit sequence */
  byteIdx = 3*(motorIdx/8);
  switch(motorIdx%8) {
    case 0:
      MotorBitsByteToSend[byteIdx] &= ~0b11100000;
      MotorBitsByteToSend[byteIdx] |= (b[0]<<7) + (b[1]<<6) + (b[2]<<5);
      break;
    case 1:
      MotorBitsByteToSend[byteIdx] &= ~0b00011100;
      MotorBitsByteToSend[byteIdx] |= (b[0]<<4) + (b[1]<<3) + (b[2]<<2);
      break;
    case 2:
      MotorBitsByteToSend[byteIdx] &= ~0b00000011;
      MotorBitsByteToSend[byteIdx] |= (b[0]<<1) + (b[1]<<0);
      MotorBitsByteToSend[byteIdx+1] &= ~0b10000000;
      MotorBitsByteToSend[byteIdx+1] |= b[2]<<7;
      break;
    case 3:
      MotorBitsByteToSend[byteIdx+1] &= ~0b01110000;
      MotorBitsByteToSend[byteIdx+1] |= (b[0]<<6) + (b[1]<<5) + (b[2]<<4);
      break;
    case 4:
      MotorBitsByteToSend[byteIdx+1] &= ~0b00001110;
      MotorBitsByteToSend[byteIdx+1] |= (b[0]<<3) + (b[1]<<2) + (b[2]<<1);
      break;
    case 5:
      MotorBitsByteToSend[byteIdx+1] &= ~0b00000001;
      MotorBitsByteToSend[byteIdx+1] |= (b[0]<<0);
      MotorBitsByteToSend[byteIdx+2] &= ~0b11000000;
      MotorBitsByteToSend[byteIdx+2] |= (b[1]<<7) + (b[2]<<6);
      break;
    case 6:
      MotorBitsByteToSend[byteIdx+2] &= ~0b00111000;
      MotorBitsByteToSend[byteIdx+2] |= (b[0]<<5) + (b[1]<<4) + (b[2]<<3);
      break;
    case 7:
      MotorBitsByteToSend[byteIdx+2] &= ~0b00000111;
      MotorBitsByteToSend[byteIdx+2] |= (b[0]<<2) + (b[1]<<1) + (b[2]<<0);
      break;
    default:
      break;
  }
}

void ShiftReg_StoreMotorStbyBit(uint32_t motorIdx) { /* Clear stby- and clk-bit */
  /* w[0]: CLK
   * w[1]: DIR
   * w[2]: STDBY
   */
  size_t byteIdx;

  /* One driver board:
   * Each motor has 3 bits
   * MotorBitsByteToSend: [5]        [4]        [3]        [2]        [1]        [0]
   * MotorIdx             ddee'efff  abbb'cccd  8889'99aa  5566'6777  2333'4445  0001'1122  ==> shift (MSB first!)
   * clock/dir/stdby	  dscd scds  scds cdsc  cdsc dscd  dscd scds  scds cdsc  cdsc dscd
   */

  /* transform truth table for motor wires/connections into bit sequence */
  byteIdx = 3*(motorIdx/8);
  switch(motorIdx%8) {
    case 0:
      MotorBitsByteToSend[byteIdx] &= ~0b10100000;
      break;
    case 1:
      MotorBitsByteToSend[byteIdx] &= ~0b00010100;
      break;
    case 2:
      MotorBitsByteToSend[byteIdx] &= ~0b00000010;
      MotorBitsByteToSend[byteIdx+1] &= ~0b10000000;
      break;
    case 3:
      MotorBitsByteToSend[byteIdx+1] &= ~0b01010000;
      break;
    case 4:
      MotorBitsByteToSend[byteIdx+1] &= ~0b00001010;
      break;
    case 5:
      MotorBitsByteToSend[byteIdx+1] &= ~0b00000001;
      MotorBitsByteToSend[byteIdx+2] &= ~0b01000000;
      break;
    case 6:
      MotorBitsByteToSend[byteIdx+2] &= ~0b00101000;
      break;
    case 7:
      MotorBitsByteToSend[byteIdx+2] &= ~0b00000101;
      break;
    default:
      break;
  }
}

void ShiftReg_StoreMotorStbyBitsAll(void){
	/* Clear all stdby bits (stdby is low active) */
	int byteCnt = 0;
	int shiftCnt = 2; /* The stdby bit is the third bit */
	while(byteCnt<SHIFTREG_NOF_MOTORBIT_BYTES){
		if(shiftCnt>=8){
			shiftCnt = shiftCnt-8;
		}
		while(shiftCnt < 8){
			MotorBitsByteToSend[byteCnt] &= ~(0b10000000>>shiftCnt);
			shiftCnt += SHIFTREG_NOF_MOTOR_BITS;
		}
		byteCnt++;
	}


	/*Clear all clk-bits */
	byteCnt = 0;
	shiftCnt = 0; /* The clk bit is the first bit */
	while(byteCnt<SHIFTREG_NOF_MOTORBIT_BYTES){
		if(shiftCnt>=8){
			shiftCnt = shiftCnt-8;
		}
		while(shiftCnt < 8){
			MotorBitsByteToSend[byteCnt] &= ~(0b10000000>>shiftCnt);
			shiftCnt += SHIFTREG_NOF_MOTOR_BITS;
		}
		byteCnt++;
	}
}

void ShiftReg_SendStoredMotorBits(void) {
	ShiftReg_WriteMotorBits(MotorBitsByteToSend, sizeof(MotorBitsByteToSend));
	memcpy(prevMotorBitsByte, MotorBitsByteToSend, sizeof(MotorBitsByteToSend));
}

void ShiftReg_SendStoredMotorBitsAutoClk(void) {
	/* One driver board:
	 * Each motor has 3 bits
	 * MotorBitsByteToSend: [5]        [4]        [3]        [2]        [1]        [0]
	 * MotorIdx             ddee'efff  abbb'cccd  8889'99aa  5566'6777  2333'4445  0001'1122  ==> shift (MSB first!)
	 * clock/dir/stdby	  	dscd scds  scds cdsc  cdsc dscd  dscd scds  scds cdsc  cdsc dscd
	 */

	bool turnOn = false;
	int byteCnt = 0;
	int shiftCnt = 0; /* The stdby bit is the third bit */

	/* Check if Stepper Motors needs to be turned on first */
	shiftCnt = 2; /* The stdby bit is the third bit */
	while(byteCnt<SHIFTREG_NOF_MOTORBIT_BYTES){
		if(shiftCnt>=8){
			shiftCnt = shiftCnt-8;
		}
		while(shiftCnt < 8){
			if((MotorBitsByteToSend[byteCnt] & (0b10000000>>shiftCnt)) == (0b10000000>>shiftCnt)){ /* if stepper motor should be turned on */
				if( (prevMotorBitsByte[byteCnt] & (0b10000000>>shiftCnt)) == 0){ /* if stepper motor previosuly was turned off */
					turnOn = true; /* Stepper motor needs to be turned on first */
				}
			}
			shiftCnt += SHIFTREG_NOF_MOTOR_BITS;
		}
		byteCnt++;
	}

	/* Copy data and delet clk-bits */
	memcpy(MotorBitsByteToSendWithoutClk, MotorBitsByteToSend, sizeof(MotorBitsByteToSend));
	byteCnt = 0;
	shiftCnt = 0; /* The clk bit is the first bit */
	while(byteCnt<SHIFTREG_NOF_MOTORBIT_BYTES){
		if(shiftCnt>=8){
			shiftCnt = shiftCnt-8;
		}
		while(shiftCnt < 8){
			MotorBitsByteToSendWithoutClk[byteCnt] &= ~(0b10000000>>shiftCnt);
			shiftCnt += SHIFTREG_NOF_MOTOR_BITS;
		}
		byteCnt++;
	}

	if(turnOn){ /* Send the bytes without clk-bits to switch on the motors that were previously switched off. */
		ShiftReg_WriteMotorBits(MotorBitsByteToSendWithoutClk, sizeof(MotorBitsByteToSendWithoutClk));
		ShiftReg_WriteMotorBits(MotorBitsByteToSend, sizeof(MotorBitsByteToSend));
	}

	/* Send the bytes now with clk-bits */
	ShiftReg_WriteMotorBits(MotorBitsByteToSend, sizeof(MotorBitsByteToSend));
	memcpy(prevMotorBitsByte, MotorBitsByteToSend, sizeof(MotorBitsByteToSend));

	/* Send the same data again without clk-bits (Auto Clock) */
	ShiftReg_WriteMotorBits(MotorBitsByteToSendWithoutClk, sizeof(MotorBitsByteToSendWithoutClk));
	memcpy(prevMotorBitsByte, MotorBitsByteToSendWithoutClk, sizeof(MotorBitsByteToSend));
}

void ShiftReg_SendStoredMotorBitsIfChanged(void) {
	if (memcmp(MotorBitsByteToSend, prevMotorBitsByte, sizeof(MotorBitsByteToSend))!=0) { /* has it changed? */
		ShiftReg_WriteMotorBits(MotorBitsByteToSend, sizeof(MotorBitsByteToSend));
		memcpy(prevMotorBitsByte, MotorBitsByteToSend, sizeof(MotorBitsByteToSend));
	}
}

void ShiftReg_SendStoredMotorBitsIfChangedAutoClk(void) {
	/* One driver board:
	 * Each motor has 3 bits
	 * MotorBitsByteToSend: [5]        [4]        [3]        [2]        [1]        [0]
	 * MotorIdx             ddee'efff  abbb'cccd  8889'99aa  5566'6777  2333'4445  0001'1122  ==> shift (MSB first!)
	 * clock/dir/stdby	  	dscd scds  scds cdsc  cdsc dscd  dscd scds  scds cdsc  cdsc dscd
	 */

	bool turnOn = false;
	int byteCnt = 0;
	int shiftCnt = 0;
	if (memcmp(MotorBitsByteToSend, prevMotorBitsByte, sizeof(MotorBitsByteToSend))!=0) { /* has it changed? */
		/* Check if Stepper Motors needs to be turned on first */
		shiftCnt = 2; /* The stdby bit is the third bit */
		while(byteCnt<SHIFTREG_NOF_MOTORBIT_BYTES){
			if(shiftCnt>=8){
				shiftCnt = shiftCnt-8;
			}
			while(shiftCnt < 8){
				if((MotorBitsByteToSend[byteCnt] & (0b10000000>>shiftCnt)) == (0b10000000>>shiftCnt)){ /* if stepper motor should be turned on */
					if( (prevMotorBitsByte[byteCnt] & (0b10000000>>shiftCnt)) == 0){ /* if stepper motor previosuly was turned off */
						turnOn = true; /* Stepper motor needs to be turned on first */
					}
				}
				shiftCnt += SHIFTREG_NOF_MOTOR_BITS;
			}
			byteCnt++;
		}

		/* Copy data and delet clk-bits */
			memcpy(MotorBitsByteToSendWithoutClk, MotorBitsByteToSend, sizeof(MotorBitsByteToSend));
			byteCnt = 0;
			shiftCnt = 0; /* The clk bit is the first bit */
			while(byteCnt<SHIFTREG_NOF_MOTORBIT_BYTES){
				if(shiftCnt>=8){
					shiftCnt = shiftCnt-8;
				}
				while(shiftCnt < 8){
					MotorBitsByteToSendWithoutClk[byteCnt] &= ~(0b10000000>>shiftCnt);
					shiftCnt += SHIFTREG_NOF_MOTOR_BITS;
				}
				byteCnt++;
			}

		if(turnOn){ /* Send the bytes without clk-bits to switch on the motors that were previously switched off. */
			ShiftReg_WriteMotorBits(MotorBitsByteToSendWithoutClk, sizeof(MotorBitsByteToSendWithoutClk));
			//vTaskDelay(pdMS_TO_TICKS(2));
		}

		/* Send the bytes now with clk-bits */
		ShiftReg_WriteMotorBits(MotorBitsByteToSend, sizeof(MotorBitsByteToSend));
		memcpy(prevMotorBitsByte, MotorBitsByteToSend, sizeof(MotorBitsByteToSend));

		/* Send the same data again without clk-bits (Auto Clock) */
		ShiftReg_WriteMotorBits(MotorBitsByteToSendWithoutClk, sizeof(MotorBitsByteToSendWithoutClk));
		memcpy(prevMotorBitsByte, MotorBitsByteToSendWithoutClk, sizeof(MotorBitsByteToSend));
	}
}

static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  unsigned char buf[64];

  McuShell_SendStatusStr((unsigned char*)"shiftreg", (unsigned char*)"ShiftReg status\r\n", io->stdOut);
  McuGPIO_GetPinStatusString(MotorLatch, buf, sizeof(buf));
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  latch", buf, io->stdOut);

  McuUtility_Num32sToStr(buf, sizeof(buf), SHIFTREG_CONFIG_NOF_MOTORS);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  motors", buf, io->stdOut);

#if 0 /* not used */
  McuGPIO_GetPinStatusString(SensorLatch, buf, sizeof(buf));
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  sensor", buf, io->stdOut);
#endif
  return ERR_OK;
}

static uint8_t PrintInvStatus(const McuShell_StdIOType *io) {
  uint8_t buf[128];
  buf[0] = '\0';
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStr(buf, io->stdOut);
  for(int i=0; i<SHIFTREG_NOF_MOTORS; i++){
	  buf[0] = '\0';
	  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"M ");
	  McuUtility_strcatNum32s(buf, sizeof(buf), i);
	  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\t");
	  if(dirInv[i]){
		  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"true");
	  }
	  else{
		  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"false");
	  }
      McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
      McuShell_SendStr(buf, io->stdOut);
  }
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"shiftreg", (unsigned char*)"Group of ShiftReg commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  inv status", (unsigned char*)"Print inversion information of each motor.\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  inv <motor> <inv?>", (unsigned char*)"Set direction inversion of a motor. (true = inverted / false = not inverted)\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  set <motor> <val>", (unsigned char*)"Set motor value (3bits: clk, dir, stdby)\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  move <motor> <steps>", (unsigned char*)"Move motor with steps (forward/backward)\r\n", io->stdOut);
#if 0 /* not used */
  McuShell_SendHelpStr((unsigned char*)"  readsensor", (unsigned char*)"Read sensor bits\r\n", io->stdOut);
#endif
  return ERR_OK;
}

uint8_t ShiftReg_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  const unsigned char *p;
  int32_t val, motorIdx;

  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "shiftreg help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "shiftreg status")==0)) {
    *handled = TRUE;
    return PrintStatus(io);
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "shiftreg inv status")==0)) {
	    *handled = true;
	    PrintInvStatus(io);
	    return ERR_OK;
  } else if (McuUtility_strncmp((char*)cmd, "shiftreg inv ", sizeof("shiftreg inv ")-1)==0) {
	    *handled = TRUE;
	    p = cmd + sizeof("shiftreg inv ")-1;
	    if (McuUtility_xatoi(&p, &motorIdx)==ERR_OK && motorIdx<SHIFTREG_CONFIG_NOF_MOTORS) {
	    	if (*p==' ') {
	    	      p++;
	    	    }
	    	if (McuUtility_strncmp((char*)p, (char*)"true", sizeof("true")-1)==0) {
	    		dirInv[motorIdx]=true;
	    		return ERR_OK;
	    	} else if (McuUtility_strncmp((char*)p, (char*)"false", sizeof("false")-1)==0) {
	    		dirInv[motorIdx]=false;
	    		return ERR_OK;
	    	}
	    }
  } else if (McuUtility_strncmp((char*)cmd, "shiftreg set ", sizeof("shiftreg set ")-1)==0) {
    *handled = TRUE;
    p = cmd + sizeof("shiftreg set ")-1;
    if (McuUtility_xatoi(&p, &motorIdx)==ERR_OK && motorIdx<SHIFTREG_CONFIG_NOF_MOTORS) {
      if (McuUtility_xatoi(&p, &val)==ERR_OK && val<=0x7) {
        bool w[SHIFTREG_NOF_MOTOR_BITS];

        w[0] = (val&(1<<0))!=0; /* clk */
        w[1] = (val&(1<<1))!=0; /* dir */
        w[2] = (val&(1<<2))!=0; /* stby */
        ShiftReg_StoreMotorBits(motorIdx, w);
        ShiftReg_SendStoredMotorBits();

        /* for safety: set back to idle */
        w[0] = false; /* clk */
        w[1] = false; /* dir */
        w[2] = false; /* stby (low active) */
        ShiftReg_StoreMotorBits(motorIdx, w);
        ShiftReg_SendStoredMotorBits();

        return ERR_OK;
      }
    }
    return ERR_FAILED;
  } else if (McuUtility_strncmp((char*)cmd, "shiftreg move ", sizeof("shiftreg move ")-1)==0) {
    *handled = TRUE;
    p = cmd + sizeof("shiftreg move ")-1;
    if (McuUtility_xatoi(&p, &motorIdx)==ERR_OK && motorIdx<SHIFTREG_CONFIG_NOF_MOTORS) {
      if (McuUtility_xatoi(&p, &val)==ERR_OK) {
        bool w[SHIFTREG_NOF_MOTOR_BITS];

        w[0] = true; /* clk */
        w[1] = val<0; /* dir */
        w[2] = true; /* stby (low active) */
        if (val<0) {
          val = -val;
        }
        for(int i=0; i<val; i++) {
          ShiftReg_StoreMotorBits(motorIdx, w);
          ShiftReg_SendStoredMotorBitsAutoClk();
          vTaskDelay(pdMS_TO_TICKS(2));
        }
        /* set back to idle */
        w[0] = false; /* clk */
        w[1] = false; /* dir */
        w[2] = false; /* stby (low active) */
        ShiftReg_StoreMotorBits(motorIdx, w);
        ShiftReg_SendStoredMotorBits();

        return ERR_OK;
      }
    }
    return ERR_FAILED;
#if 0 /* not used */
  } else if (McuUtility_strcmp((char*)cmd, "shiftreg readsensor")==0) {
    uint8_t sensorBytes[SHIFTREG_NOF_SENSORBIT_BYTES];
    uint8_t buf[16];

    *handled = TRUE;
    ShiftReg_ReadSensorBits(sensorBytes, sizeof(sensorBytes));
    for(int i=0;i<sizeof(sensorBytes); i++) {
      McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"0x");
      McuUtility_strcatNum8Hex(buf, sizeof(buf), sensorBytes[i]);
      McuUtility_chcat(buf, sizeof(buf), ' ');
      McuShell_SendStr(buf, io->stdOut);
    }
    McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
    return ERR_OK;
#endif
  }
  return ERR_OK;
}

void ShiftReg_Deinit(void) {
  MotorLatch = McuGPIO_DeinitGPIO(MotorLatch);
  vQueueDelete(ShiftRegMutex);
  ShiftRegMutex = NULL;
}

void ShiftReg_Init(void) {
  McuGPIO_Config_t config;

  SHIFTREG_CONFIG_LATCH_ENABLE_CLOCKS();
  McuGPIO_GetDefaultConfig(&config);
  config.isHighOnInit = false; /* high pulse latches the data */
  config.isInput = false;
  config.hw.gpio = SHIFTREG_CONFIG_MOTORLATCH_GPIO;
  config.hw.port = SHIFTREG_CONFIG_MOTORLATCH_PORT;
  config.hw.pin = SHIFTREG_CONFIG_MOTORLATCH_PIN;
  MotorLatch = McuGPIO_InitGPIO(&config);

#if 0 /* not used */
  McuGPIO_GetDefaultConfig(&config);
  config.isHighOnInit = true; /* low pulse latches the data */
  config.isInput = false;
  config.hw.gpio = SHIFTREG_CONFIG_SENSORLATCH_GPIO;
  config.hw.port = SHIFTREG_CONFIG_SENSORLATCH_PORT;
  config.hw.pin = SHIFTREG_CONFIG_SENSORLATCH_PIN;
  SensorLatch = McuGPIO_InitGPIO(&config);
#endif

#if STEPPER_CONFIG_USE_FREERTOS_TIMER
  ShiftRegMutex = xSemaphoreCreateMutex();
  if (ShiftRegMutex==NULL) { /* semaphore creation failed */
    McuLog_fatal("failed creating ShiftReg mutex");
    for(;;) {} /* error, not enough memory? */
  }
  vQueueAddToRegistry(ShiftRegMutex, "ShiftRegMutex");
#endif
}

#endif /* PL_CONFIG_USE_SHIFT_REGS */
