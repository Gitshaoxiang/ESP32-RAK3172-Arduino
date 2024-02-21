#include "Arduino.h"

#define FREQ_868M 1
#define FREQ_915M 0
#define FREQ_923M 0

#if FREQ_868M
#define PRODUCT_NAME "Stamp LoRaWAN 868"

#define LORA_FREQ     868E6
#define LORA_FREQ_STR "868M"

#elif FREQ_915M
#define PRODUCT_NAME  "Stamp LoRaWAN 915"
#define LORA_FREQ     915E6
#define LORA_FREQ_STR "915M"

#elif FREQ_923M
#define PRODUCT_NAME  "Stamp LoRaWAN 923"
#define LORA_FREQ     923E6
#define LORA_FREQ_STR "923M"

#endif
