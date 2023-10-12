<H1>LoRaWAN_Radioenge_STM32</H1> 

Essa biblioteca é destinada para microcontroladores STM32 e compatíveis, porém é estruturado encima da STM32CubeIDE. Para fazer com que rode com qualquer microcontrolador da STM32, é necessário fazer uma pequena modificação no arquivo [LoRaWAN_Radioenge.h](https://github.com/elcereza/LoRaWAN_Radioenge_STM32/blob/main/Library/LoRaWAN_Radioenge.h) onde será necessário mudar o include 'stm32l4xx_hal.h' para o mais adequado para o microcontrolador que você estiver usando, como por exemplo: stm32f4xx_hal.h, stm32l1xx_hal.h, stm32l0xx_hal.h e etc...

A biblioteca em si é uma "replica" quase identica a que foi feita para Arduino (https://github.com/elcereza/LoRaWAN), porém com suas respectivas modificações para atender a estrutura do STM32. 

É importante considerar que a estrutura do STM32CubeIDE trabalha com comentários do tipo:
```
/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

```
Essas estruturas são extremamentes importantes para o código montado dentro da IDE, pois há um autogerador de código dentro da IDE e caso você não respeite isso, poderá perder o código que foi montado. Se por exemplo você cria seus '#defines' fora desses comentários, é certo que perderá seu código a qualquer atualização do autogerador...

Para mais informações, assista este vídeo: https://youtu.be/JPEG9oSDyiA
