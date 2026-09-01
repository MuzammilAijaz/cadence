#ifndef LED__H
#define LED__H

/* Build in LED is in pin B5 (STM8S103 board) or D3 (STM8S003F3 board) */
#ifdef STM8S103
#define LED_PORT    GPIOB
#define LED_PIN     GPIO_PIN_5
#endif

#endif
