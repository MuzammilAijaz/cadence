#include "analog_stick.h"
#include "utils.h"
#include <stm8s_gpio.h>
#include <stm8s_clk.h>
#include <stm8s_adc1.h>

#define GPIO_PD5_ADC_CHANNEL              ADC1_CHANNEL_5
#define GPIO_PD6_ADC_CHANNEL              ADC1_CHANNEL_6
#define ANALOG_STICK_AXIS_X_ADC_CHANNEL   GPIO_PD6_ADC_CHANNEL
#define ANALOG_STICK_AXIS_Y_ADC_CHANNEL   GPIO_PD5_ADC_CHANNEL

static volatile AdcAnalogAxis adc_current_axis = ADC_AXIS_X;

/* NOTE: does not configure the buttons that are usually found on analog sticks. */
void analog_stick_init(void) {
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_ADC, ENABLE);

    //----- Setup and Start ADC ------------------------------------

    ADC1_DeInit();

    // Initialize ADC1 in single conversion mode with right alignment
    ADC1_Init(ADC1_CONVERSIONMODE_SINGLE,
        // set current channel to:
        ANALOG_STICK_AXIS_X_ADC_CHANNEL,

        ADC1_PRESSEL_FCPU_D18,
        ADC1_EXTTRIG_GPIO,
        DISABLE,
        ADC1_ALIGN_RIGHT,

        // Disable Schmitt triggers: to avoid digital input circuitry from
        // trying to interpret that same voltage as a 0 or 1 during ADC
        ADC1_SCHMITTTRIG_ALL,
        DISABLE
    );

    // enable interrupt on ADC conversion // @see ADC1_IRQHandler
    ADC1_ITConfig(ADC1_IT_EOCIE, ENABLE);

    ADC1_Cmd(ENABLE); // Power on ADC1
    delay_ms(5); // Stabilization delay for analog reference

    adc_current_axis = ADC_AXIS_X;
}

/* Non blocking asynchronous call to get ADC value from ADC channel for x axis */
void analog_stick_start_x_conversion(void) {
    adc_current_axis = ADC_AXIS_X;
    ADC1_ConversionConfig(ADC1_CONVERSIONMODE_SINGLE, ANALOG_STICK_AXIS_X_ADC_CHANNEL, ADC1_ALIGN_RIGHT);
    ADC1_StartConversion();
}

/* Non blocking asynchronous call to get ADC value from ADC channel for y axis */
void analog_stick_start_y_conversion(void) {
    adc_current_axis = ADC_AXIS_Y;
    ADC1_ConversionConfig(ADC1_CONVERSIONMODE_SINGLE, ANALOG_STICK_AXIS_Y_ADC_CHANNEL, ADC1_ALIGN_RIGHT);
    ADC1_StartConversion();
}

AdcAnalogAxis get_axis_being_converted(void) {
    return adc_current_axis;
}

