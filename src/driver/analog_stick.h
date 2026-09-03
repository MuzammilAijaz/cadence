#ifndef ANALOG_STICK__H
#define ANALOG_STICK__H

#define ANALOG_STICK_AXIS_X_PORT   GPIOD
#define ANALOG_STICK_AXIS_Y_PORT   GPIOD
#define ANALOG_STICK_AXIS_X_PIN    GPIO_PIN_6
#define ANALOG_STICK_AXIS_Y_PIN    GPIO_PIN_5

void analog_stick_init(void);
void analog_stick_start_x_conversion(void);
void analog_stick_start_y_conversion(void);

typedef enum {
    ADC_AXIS_X,
    ADC_AXIS_Y
} AdcAnalogAxis;

AdcAnalogAxis get_axis_being_converted(void);

#endif
