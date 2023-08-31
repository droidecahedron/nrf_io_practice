#ifndef IO_H_
#define IO_H_

#define dk_button1_msk 1 << 11 // button1 is gpio pin 11 in the .dts
#define dk_button2_msk 1 << 12 // button2 is gpio pin 12 in the .dts
#define dk_button3_msk 1 << 24 // button3 is gpio pin 24 in the .dts
#define dk_button4_msk 1 << 25 // button4 is gpio pin 25 in the .dts

#define GPIO_SPEC_AND_COMMA(button_or_led) GPIO_DT_SPEC_GET(button_or_led, gpios),

int init_gpio(void);
void blinkythread(void);

#endif