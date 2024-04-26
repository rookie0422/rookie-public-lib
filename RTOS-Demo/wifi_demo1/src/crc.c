/*
 * @Author: your name
 * @Date: 2023-02-18 15:59:46
 * @LastEditTime: 2023-02-19 10:20:04
 * @LastEditors: your name
 * @Description: 
 * @FilePath: \MDK-ARMc:\Users\86183\Desktop\powerliku\src\sensor\crc.c
 * 可以输入预定的版权声明、个性签名、空行等
 */
#include <stdint.h>
//////////////*高位表*///////////////////

extern const uint8_t auchCRCHi[];

//////////////*低位表*///////////////////
extern const uint8_t auchCRCLo[];

uint16_t CalcCRC16(uint8_t *puchMsg, uint8_t usDataLen) 	
{ 											
	uint8_t uchCRCHi = 0xFF ; 				
	uint8_t uchCRCLo = 0xFF ; 				
	uint16_t uIndex ; 							
	while (usDataLen--) 					
	{
		uIndex = uchCRCHi ^ *puchMsg++ ; 	
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
		uchCRCLo = auchCRCLo[uIndex] ;
	}
	return (((uint16_t)(uchCRCHi) << 8) | uchCRCLo) ;
}
