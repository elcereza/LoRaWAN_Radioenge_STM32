#include "LoRaWAN_Radioenge.h"

UART_HandleTypeDef *LoRa;
UART_HandleTypeDef *SerialFeedback;

uint8_t feedback = 0, connected = 0;

char* AT_CMD[39] = {
      "ATZ",
      "DADDR",
      "APPKEY",
      "APPSKEY",
      "NWKSKEY",
      "APPEUI",
      "ADR",
      "DR",
      "DCS",
      "PNM",
      "RX2FQ",
      "RX2DR",
      "RX1DL",
      "RX2DL",
      "JN1DL",
      "JN2DL",
      "NJM",
      "NWKID",
      "CLASS",
      "JOIN",
      "NJS",
      "SENDB",
      "SEND",
      "VER",
      "CFM",
      "SNR",
      "RSSI",
      "BAT",
      "BAUDRATE",
      "NBTRIALS",
      "KEEPALIVE",
      "TXCFM",
      "CHMASK",
      "ADC",
      "GPIOC",
      "WPIN",
      "RPIN",
      "AJOIN",
      "DEUI"
  };

char g_payload[BUFFER_SIZE];
uint8_t array[BUFFER_SIZE];
char* payloads[5];
uint8_t port = 1, confirmado = 0, retries = 0;
int periodicidade = 0;
uint8_t buffer_err = 0;

void UART_WriteString(UART_HandleTypeDef *huart, char *val){
	uint32_t len_val = strlen(val);
	HAL_UART_Transmit(huart, (uint8_t*)val, len_val, HAL_MAX_DELAY);
}

char* UART_ReadString(UART_HandleTypeDef *huart){
	uint8_t received_data = 0;
	uint8_t buffer_pData[1024];
	uint32_t count = 0;

	while(1){
		int response = HAL_UART_Receive(huart, &received_data, 1, 1000);
		if(HAL_OK == response){
			if(received_data == '\n'){
				buffer_pData[count] = received_data;
				++count;
				break;
			}
			else{
				if(received_data != '\r'){
					buffer_pData[count] = received_data;
					++count;
				}
			}
		}
		else if(HAL_ERROR == response){
			buffer_err = HAL_ERROR;
			return "HAL_ERROR";
		}
		else if(HAL_BUSY == response){
			buffer_err = HAL_BUSY;
			return "HAL_BUSY";
		}
		else if(HAL_TIMEOUT == response){
			buffer_err = HAL_TIMEOUT;
			return "HAL_TIMEOUT";
		}
	}

	char* val_string = (char*)malloc(count + 1);
	if (val_string != NULL) {
		for (uint32_t i = 0; i < count; ++i)
			if(buffer_pData[i] != '\n' || buffer_pData[i] != '\r')
				val_string[i] = (char)buffer_pData[i];
	    val_string[count] = '\0';
	}

	char* buffer_val = val_string;
	free(val_string);
	buffer_err = HAL_OK;
	return buffer_val;
}

char* feedbackSerial(char* val, uint8_t exception){
  if(feedback == 1){
	  UART_WriteString(SerialFeedback, "TX: ");
	  UART_WriteString(SerialFeedback, val);
  }

  UART_WriteString(LoRa, val);
  char* buff;
  uint8_t count = 8;
  static uint8_t count_err = 0;

  while(1){
	  buff = UART_ReadString(LoRa);

	  if(count_err >= 3)
		  NVIC_SystemReset();
	  else if(buffer_err != HAL_OK)
		  ++count_err;
	  else
		  count_err = 0;

	  if(exception == 0){
		  if(indexOf(buff, 'E') == indexOf(buff, 'R') - 1)
			  return "";
	      break;
	  }
	  else{
		  if(indexOf(buff, 'E') > 0 && indexOf(buff, 'D') < 0 && count > 0)
			count -= 1;
	      else if(count <= 0)
	        break;
	      else if(indexOf(buff, 'K') > 0 || indexOf(buff, 'D') == indexOf(buff, 'N') + 2 || indexOf(buff, 'Y') > 0){
	    	  connected = 1;
	    	  break;
	      }
	  }
  }

  if(feedback == 1){
	  if(buff != ""){
		  UART_WriteString(SerialFeedback, "RX: ");
	  	  UART_WriteString(SerialFeedback, buff);
	  }

	  UART_WriteString(SerialFeedback, "\r\n");
  }
  return buff;
}

char* commandAT(uint8_t cmd, char* val, uint8_t exception){
  char* AT = "AT+";
  char* buff;

  char command[BUFFER_SIZE];

  if(exception == 0 && val == "")
	  sprintf(command, "%s%s=?\r\n\0", AT, AT_CMD[cmd]);
  else if(exception == 1)
	  sprintf(command, "%s%s\r\n\0", AT, AT_CMD[cmd]);
  else
	  sprintf(command, "%s%s=%s\r\n\0", AT, AT_CMD[cmd], val);

  HAL_Delay(50);
  return feedbackSerial(command, exception);
}

char* uint32_tTocharPointer(uint32_t val){
	char buffer[1024];
	snprintf(buffer, sizeof(buffer), "%lu", (unsigned long)val);
	return buffer;
}

int indexOf(char* val, char search){
	for(uint8_t i = 0; i < strlen(val); ++i){
		if(val[i] == search)
			return i;
	}
	return -1;
}

char* bool_to_intString(uint8_t val){
	if(val == 1)
		return "1";
	return "0";
}

void deserializeAT(uint8_t cmd) {
    char* val_char = commandAT(cmd, "", 0);
    uint8_t count = 0;

    for (uint8_t i = 0; i < strlen(val_char); ++i) {
        if (val_char[i] != ':') {
            if (count < strlen(val_char) - 1) {
                payloads[count] += val_char[i];
            }
        } else {
            ++count;
        }
    }
}

char* separator(char* val) {
    size_t val_size = strlen(val);

    if (val_size % 2 == 0 && indexOf(val, ':') < 1) {
        size_t result_size = val_size * 2 + 1;
        char* result = (char*)malloc(result_size);

        if (result != NULL) {
            uint8_t count = 0;
            uint8_t pair_count = 0;

            for (uint8_t i = 0; i < val_size; ++i) {
                result[count] = val[i];
                ++count;

                if (pair_count > 0 && i + 1 < val_size) {
                    result[count] = ':';
                    ++count;
                    pair_count = 0;
                }

                if(i % 2 == 0)
                	++pair_count;

            }
            result[count] = '\0';

            val = result;
            free(result);
            return val;
        }
    }

    return val;
}

uint16_t GPIO(uint8_t cmd, uint8_t pin, uint8_t val){
  char* buff = "";

  if(val != 2){
	sprintf(g_payload, "%d:%d\0", pin, val);
	buff = commandAT(cmd, g_payload, 0);
  }
  else{
	buff = commandAT(cmd, uint32_tTocharPointer(pin), 0);
  }

  return (uint16_t)strtoul(buff, NULL, 10);
}

void printParameters(void){
  uint8_t buff = feedback;
  feedback = 0;
  char* version = VER();

  UART_WriteString(SerialFeedback, "---------------------------------------------------\r\n");
  UART_WriteString(SerialFeedback, "                  LoRaWAN Radioenge\r\n");
  UART_WriteString(SerialFeedback, " Version        = "); UART_WriteString(SerialFeedback, version);  UART_WriteString(SerialFeedback, "\r"); char* _DEUI = DEUI();
  UART_WriteString(SerialFeedback, " DevEui         = "); UART_WriteString(SerialFeedback, _DEUI); UART_WriteString(SerialFeedback, "\r"); char* _DADDR = DADDR("");
  UART_WriteString(SerialFeedback, " DevAddr        = "); UART_WriteString(SerialFeedback, _DADDR); UART_WriteString(SerialFeedback, "\r"); char* _APPKEY = APPKEY("");
  UART_WriteString(SerialFeedback, " AppKey         = "); UART_WriteString(SerialFeedback, _APPKEY); UART_WriteString(SerialFeedback, "\r"); char* _APPSKEY = APPSKEY("");
  UART_WriteString(SerialFeedback, " AppSKey        = "); UART_WriteString(SerialFeedback, _APPSKEY); UART_WriteString(SerialFeedback, "\r"); char* _NWKSKEY = NWKSKEY("");
  UART_WriteString(SerialFeedback, " NwkSKey        = "); UART_WriteString(SerialFeedback, _NWKSKEY); UART_WriteString(SerialFeedback, "\r"); char* _APPEUI = APPEUI("");
  UART_WriteString(SerialFeedback, " AppEui/JoinEui = "); UART_WriteString(SerialFeedback, _APPEUI); UART_WriteString(SerialFeedback, "\r");
  UART_WriteString(SerialFeedback, "                    elcereza.com\r\n");
  UART_WriteString(SerialFeedback, "---------------------------------------------------\r\n");

  feedback = buff;
}

void LoRaWAN_Begin(uint8_t _feedback, UART_HandleTypeDef* _LoRa, UART_HandleTypeDef* _SerialFeedBack){
  LoRa = _LoRa;
  SerialFeedback = _SerialFeedBack;
  feedback = _feedback;
  printParameters();
}

char* DADDR(char* val){
  if(val != "") commandAT(_DADDR_, separator(val), 0);
  return commandAT(_DADDR_, "", 0);;
}

char* APPKEY(char* val){
  if(val != "") commandAT(_APPKEY_, separator(val), 0);
  return commandAT(_APPKEY_, "", 0);
}

char* APPSKEY(char* val){
  if(val != "") commandAT(_APPSKEY_, separator(val), 0);
  return commandAT(_APPSKEY_, "", 0);
}

char* NWKSKEY(char* val){
  if(val != "") commandAT(_NWKSKEY_, separator(val), 0);
  return commandAT(_NWKSKEY_, "", 0);
}

char* APPEUI(char* val){
  if(val != "") commandAT(_APPEUI_, separator(val), 0);
  return commandAT(_APPEUI_, "", 0);
}

char* DEUI(void){
  return commandAT(_DEUI_, "", 0);
}

char* CHMASK(char* val){
  if(val != "") commandAT(_CHMASK_, separator(val), 0);
  return commandAT(_CHMASK_, "", 0);
}

void ATZ(void){
	commandAT(_ATZ_, "", 0);
}

uint8_t ADR(uint8_t val){
  if(val != NULL) commandAT(_ADR_, bool_to_intString(val), 0);
  return (uint8_t)strtoul(commandAT(_ADR_, "", 0), NULL, 10);
}

uint8_t DR(uint8_t val){
  if(val < 14) (uint8_t)commandAT(_DR_, uint32_tTocharPointer(val), 0);
  return (uint8_t)strtoul(commandAT(_DR_, "", 0), NULL, 10);
}

uint8_t DCS(uint8_t val){
  if(val != NULL) commandAT(_DCS_, bool_to_intString(val), 0);
  return (uint8_t)strtoul(commandAT(_DCS_, "", 0), NULL, 10);
}

uint8_t PNM(uint8_t val){
  if(val != NULL) commandAT(_PNM_, bool_to_intString(val), 0);
  return (uint8_t)strtoul(commandAT(_PNM_, "", 0), NULL, 10);
}

uint32_t RX2FQ(uint32_t val){
  if(val != NULL) commandAT(_RX2FQ_, uint32_tTocharPointer(val), 0);
  return (uint32_t)commandAT(_RX2FQ_, "", 0);
}

uint16_t RX2DR(uint16_t val){
  if(val != NULL) commandAT(_RX2DR_, uint32_tTocharPointer(val), 0);
  return (uint16_t)strtoul(commandAT(_RX2DR_, "", 0), NULL, 10);
}

uint16_t RX1DL(uint16_t val){
  if(val != NULL) commandAT(_RX1DL_, uint32_tTocharPointer(val), 0);
  return (uint16_t)strtoul(commandAT(_RX1DL_, "", 0), NULL, 10);
}

uint16_t RX2DL(uint16_t val){
  if(val != NULL) commandAT(_RX2DL_, uint32_tTocharPointer(val), 0);
  return (uint16_t)strtoul(commandAT(_RX2DL_, "", 0), NULL, 10);
}

uint16_t JN1DL(uint16_t val){
  if(val != NULL) commandAT(_JN1DL_, uint32_tTocharPointer(val), 0);
  return (uint16_t)strtoul(commandAT(_JN1DL_, "", 0), NULL, 10);
}

uint16_t JN2DL(uint16_t val){
  if(val != NULL) commandAT(_JN2DL_, uint32_tTocharPointer(val), 0);
  return (uint16_t)strtoul(commandAT(_JN2DL_, "", 0), NULL, 10);
}

uint8_t NJM(uint8_t val){
  if(val != NULL) commandAT(_NJM_, bool_to_intString(val), 0);
  return (uint8_t)strtoul(commandAT(_NJM_, "", 0), NULL, 10);
}

char* NWKID(void){
  return commandAT(_NWKID_, "", 0);
}

uint8_t CLASS(uint8_t val){
  if(val == 0) commandAT(_CLASS_, "A", 0);
  else if(val == 1) commandAT(_CLASS_, "C", 0);
  else if(commandAT(_CLASS_, "", 0) == "C") return 1;
  return 0;
}

uint8_t JOIN(void){
  char* buff = commandAT(_JOIN_, "", 1);
  return connected;
}

uint8_t AJOIN(uint8_t val){
  if(val != "") commandAT(_AJOIN_, bool_to_intString(val), 0);
  return (uint8_t)strtoul(commandAT(_AJOIN_, "", 0), NULL, 10);
}

uint8_t NJS(){
  return commandAT(_NJS_, "", 0);
}

char* VER(void){
  return commandAT(_VER_, "", 0);
}

uint8_t CFM(uint8_t val){
  if(val != NULL) commandAT(_CFM_, bool_to_intString(val), 0);
  return (uint8_t)strtoul(commandAT(_CFM_, "", 0), NULL, 10);
}

uint8_t SNR(void){
  return (uint8_t)strtoul(commandAT(_SNR_, "", 0), NULL, 10);
}

int RSSI(void){
  return (int)strtoul(commandAT(_RSSI_, "", 0), NULL, 10);
}

float BAT(void){
  return (uint16_t)strtoul(commandAT(_BAT_, "", 0), NULL, 10) * 100 / 253;
}

uint16_t BAUDRATE(uint16_t val){
  if(val != NULL) commandAT(_BAUDRATE_, uint32_tTocharPointer(val), 0);
  return (uint16_t)strtoul(commandAT(_BAUDRATE_, "", 0), NULL, 10);
}

uint8_t NBTRIALS(uint8_t val){
  if(val != NULL) commandAT(_NBTRIALS_, uint32_tTocharPointer(val), 0);
  return (uint8_t)strtoul(commandAT(_NBTRIALS_, "", 0), NULL, 10);
}

uint8_t TXCFM(uint8_t _port, uint8_t _confirmado, uint8_t _retries, char* payload){
  uint8_t index = 0;

  memset(array, 0, BUFFER_SIZE);

  strcpy((char*)&array[index], payload);
  index += strlen(payload);

  if(index > BUFFER_SIZE)
    return 0;

  char* _payload = "";
  for(int i = 0; i < index; ++i)
    _payload += array[index];

  sprintf(g_payload, "%d:%d:%d:%s\0", _port, _retries, _confirmado, _payload);
  commandAT(_SENDB_, g_payload, 0);
  return 1;
}

uint8_t KEEPALIVE(uint8_t habilitado, uint8_t _port, uint8_t _confirmado, int _periodicidade){
  if(habilitado != NULL && _port != NULL && _confirmado != NULL, _periodicidade != NULL){
    sprintf(g_payload, "%d:%d:%d:%d\0", habilitado, _port, _confirmado, _periodicidade);
	commandAT(_KEEPALIVE_, g_payload, 0);
  }

  deserializeAT(_KEEPALIVE_);
  port          = uint32_tTocharPointer(payloads[1]);
  confirmado    = uint32_tTocharPointer(payloads[2]);
  periodicidade = uint32_tTocharPointer(payloads[3]);

  return uint32_tTocharPointer(payloads[0]);
}

uint8_t pinMode(uint8_t pin, uint8_t modo){
  uint8_t pull = 0;

  if(pin > 9 || modo > 10) return 0;
  else if((modo == OUTPUT_FA_PUSHPULL || modo == OUTPUT_FA_OPENDRAIN) && pin != 0 && pin != 1) return 0;
  else if(modo == INPUT_ADC && pin != 0 && pin != 1 && pin != 7 && pin != 8) return 0;
  else if((modo == INTERRUPT_RISING || modo == INTERRUPT_FALLING || modo == INTERRUPT_CHANGE) && pin == 0 && pin == 3 && pin == 7 && pin == 8) return 0;

  if(modo == INPUT)
    modo = 0;
  else if(modo == OUTPUT)
    modo = 1;
  if(modo == INPUT_PULLUP){
    modo = 0;
    pull = 1;
  }
  else if(pull == INPUT_PULLDOWN){
    modo = 0;
    pull = 2;
  }

  deserializeAT(_GPIOC_);
  uint8_t _modo = uint32_tTocharPointer(payloads[1]);
  uint8_t _pull = uint32_tTocharPointer(payloads[2]);

  char command[BUFFER_SIZE];
  sprintf(command, "%d:%d:%d\0", pin, modo, pull);

  if(_modo != modo || _pull != pull)
	  commandAT(_GPIOC_, command, 0);
  return 1;
}

uint8_t digitalRead(uint8_t pin){
  return (uint8_t)GPIO(_RPIN_, pin, "");
}

uint8_t digitalWrite(uint8_t pin, uint8_t val){
  return (uint8_t)GPIO(_WPIN_, pin, val);
}

uint16_t analogRead(uint8_t pin){
  return GPIO(_ADC_, pin, "");
}

void ConfigNetwork(uint8_t njm, uint8_t net, char* appkey, char* appeui, char* nwkskey, char* daddr){
  if(NJM(NULL) != njm) NJM(njm);
  if(njm == OTAA && CLASS(2) == 1) CLASS(0);

  if(njm == ABP) if(appkey != "" && indexOf(appkey, ':') > 0) APPSKEY(appkey);
  else if(appkey != "" && indexOf(appkey, ':') > 0) APPKEY(appkey);
  if(appeui != "" && indexOf(appeui, ':') > 0) APPEUI(appeui);
  if(nwkskey != "" && indexOf(nwkskey, ':') > 0) NWKSKEY(nwkskey);
  if(daddr != "" && indexOf(daddr, ':') > 0) DADDR(daddr);

  uint16_t buff_uint16;
  buff_uint16 = RX1DL(""); if((CS == net || TTN == net) && buff_uint16 != 1000) RX1DL(1000); else if(EN == net && buff_uint16 != 5000) RX1DL(5000);
  buff_uint16 = RX2DL(""); if((CS == net || TTN == net) && buff_uint16 != 2000) RX2DL(2000); else if(EN == net && buff_uint16 != 6000) RX2DL(6000);
  buff_uint16 = JN1DL(""); if((CS == net || TTN == net) && buff_uint16 != 5000) JN1DL(5000); else if(EN == net && buff_uint16 != 5000) JN1DL(5000);
  buff_uint16 = JN2DL(""); if((CS == net || TTN == net) && buff_uint16 != 6000) JN2DL(6000); else if(EN == net && buff_uint16 != 6000) JN2DL(5000);

  uint8_t buff_chmask = indexOf(CHMASK(""), '1');
  if(net == EN && buff_chmask < 1) CHMASK("00ff:0000:0000:0000:0001:0000");
  else if((CS == net || TTN == net) && buff_chmask < 1) CHMASK("ff00:0000:0000:0000:0002:0000");
}

uint8_t JoinNetwork(uint8_t njm, uint8_t net,  uint8_t autoconfig, uint8_t automatic, char* appkey, char* appeui, char* nwkskey, char* daddr){
  if(autoconfig > 0)
    ConfigNetwork(njm, net, appkey, appeui, nwkskey, daddr);
  if(automatic != AJOIN("")) AJOIN(automatic);
  if(!NJS() < 1)
    return JOIN();
  else
    return 1;
  return 0;
}

uint8_t SendString(char* string, uint8_t _port)
{
  if(string == NULL || strnlen(string, BUFFER_SIZE) >= BUFFER_SIZE)
    return 0;
  if(connected > 0){
	sprintf(g_payload, "%d:%s\0", _port, string);
	commandAT(_SEND_, g_payload, 0);
	return 1;
  }

  return 0;
}


uint8_t SendRaw(char* payload, uint8_t _port)
{
  uint8_t index = 0;

  memset(array, 0, BUFFER_SIZE);

  strcpy((char*)&array[index], payload);
  index += strlen(payload);

  if(index > BUFFER_SIZE)
    return 0;

  char* _payload = "";
  for(int i = 0; i < index; ++i)
    _payload += array[index];

  sprintf(g_payload, "%d:%s\0", _port, _payload);
  commandAT(_SENDB_, _payload, 0);
  return 1;
}
