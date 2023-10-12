#include <stdio.h>
#include <string.h>
#include "stm32l4xx_hal.h"		// editar para o tipo do teu stm32, como: stm32f4xx, stm32l0 e etc...

#ifndef LoRaWAN_Radioenge_H
#define LoRaWAN_Radioenge_H

#define _ATZ_               0
#define _DADDR_             1
#define _APPKEY_            2
#define _APPSKEY_           3
#define _NWKSKEY_           4
#define _APPEUI_            5
#define _ADR_               6
#define _DR_                7
#define _DCS_               8
#define _PNM_               9
#define _RX2FQ_             10
#define _RX2DR_             11
#define _RX1DL_             12
#define _RX2DL_             13
#define _JN1DL_             14
#define _JN2DL_             15
#define _NJM_               16
#define _NWKID_             17
#define _CLASS_             18
#define _JOIN_              19
#define _NJS_               20
#define _SENDB_             21
#define _SEND_              22
#define _VER_               23
#define _CFM_               24
#define _SNR_               25
#define _RSSI_              26
#define _BAT_               27
#define _BAUDRATE_          28
#define _NBTRIALS_          29
#define _KEEPALIVE_         30
#define _TXCFM_             31
#define _CHMASK_            32
#define _ADC_               33
#define _GPIOC_             34
#define _WPIN_              35
#define _RPIN_              36
#define _AJOIN_             37
#define _DEUI_              38

#define ABP                 0
#define OTAA                1

#define CS                  0
#define TTN                 1
#define EN                  2

#define OUTPUT				1
#define OUTPUT_OPENDRAIN    2
#define OUTPUT_FA_PUSHPULL  3
#define OUTPUT_FA_OPENDRAIN 4

#define INPUT				0
#define INPUT_PULLUP		5
#define INPUT_PULLDOWN		9
#define INPUT_ADC           6
#define INTERRUPT_RISING    7
#define INTERRUPT_FALLING   8
#define INTERRUPT_CHANGE    10

#define BUFFER_SIZE   256

void UART_WriteString(UART_HandleTypeDef *huart, char *val);
char* UART_ReadString(UART_HandleTypeDef *huart);

void deserializeAT(uint8_t cmd);
void ATZ(void);
void ConfigNetwork(uint8_t njm, uint8_t net, char* appkey, char* appeui, char* nwkskey, char* daddr);
void printParameters(void);
void LoRaWAN_Begin(uint8_t _feedback, UART_HandleTypeDef *_LoRa, UART_HandleTypeDef *_SerialFeedBack);

char* separator(char* val);
char* feedbackSerial(char* val, uint8_t exception);
char* commandAT(uint8_t cmd, char* val, uint8_t exception);
char* bool_to_intString(uint8_t val);
char* DADDR(char* val);
char* APPKEY(char* val);
char* APPSKEY(char* val);
char* NWKSKEY(char* val);
char* APPEUI(char* val);
char* DEUI(void);
char* CHMASK(char* val);
char* NWKID(void);
char* VER(void);
char* uint32_tTocharPointer(uint32_t val);

uint8_t ADR(uint8_t val);
uint8_t DR(uint8_t val);
uint8_t DCS(uint8_t val);
uint8_t PNM(uint8_t val);
uint8_t NJM(uint8_t val);
uint8_t CLASS(uint8_t val);
uint8_t JOIN(void);
uint8_t AJOIN(uint8_t val);
uint8_t NJS(void);
uint8_t NBTRIALS(uint8_t val);
uint8_t TXCFM(uint8_t _port, uint8_t _confirmado, uint8_t _retries, char* payload);
uint8_t KEEPALIVE(uint8_t habilitado, uint8_t _port, uint8_t _confirmado, int _periodicidade);
uint8_t pinMode(uint8_t pin, uint8_t modo);
uint8_t digitalRead(uint8_t pin);
uint8_t digitalWrite(uint8_t pin, uint8_t val);
uint8_t CFM(uint8_t val);
uint8_t SNR(void);
uint8_t JoinNetwork(uint8_t njm, uint8_t net,  uint8_t autoconfig, uint8_t automatic, char* appkey, char* appeui, char* nwkskey, char* daddr);
uint8_t SendString(char* string, uint8_t _port);
uint8_t SendRaw(char* payload, uint8_t _port);

uint16_t RX2DR(uint16_t val);
uint16_t RX1DL(uint16_t val);
uint16_t RX2DL(uint16_t val);
uint16_t JN1DL(uint16_t val);
uint16_t JN2DL(uint16_t val);
uint16_t GPIO(uint8_t cmd, uint8_t pin, uint8_t val);
uint16_t BAUDRATE(uint16_t val);
uint16_t analogRead(uint8_t pin);

uint32_t RX2FQ(uint32_t val);

int RSSI(void);
int indexOf(char* val, char search);

float BAT(void);

#endif
