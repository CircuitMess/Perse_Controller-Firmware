#ifndef PERSE_BASICCTRL_PINS_H
#define PERSE_BASICCTRL_PINS_H

#ifdef CTRL_TYPE_MISSIONCTRL

#define PIN_BATT 9
#define PIN_BL 39
#define PIN_QUAL 8

#define JOY_H 2
#define JOY_V 1

#define BTN_PAIR 35
#define BTN_PANIC 14
#define BTN_JOY 13
#define BTN_ENC_CAM 37
#define BTN_ENC_ARM 11
#define BTN_ENC_PINCH 6

#define SW_ARM 4
#define SW_LIGHT 44

#define LED_POWER 17
#define LED_PAIR 34
#define LED_PANIC_L 33
#define LED_PANIC_R 18

#define ENC_CAM_A 38
#define ENC_CAM_B 36
#define ENC_ARM_A 10
#define ENC_ARM_B 12
#define ENC_PINCH_A 5
#define ENC_PINCH_B 7

#define TFT_SCK 40
#define TFT_MOSI 41
#define TFT_DC 42
#define TFT_RST 43

#define I2C_SDA 15
#define I2C_SCL 16

#define EXTLED_CAM_0 6
#define EXTLED_CAM_L1 5
#define EXTLED_CAM_L2 4
#define EXTLED_CAM_L3 3
#define EXTLED_CAM_L4 2
#define EXTLED_CAM_R1 14
#define EXTLED_CAM_R2 13
#define EXTLED_CAM_R3 12
#define EXTLED_CAM_R4 7
#define EXTLED_WARN 1
#define EXTLED_ARM 9
#define EXTLED_LIGHT 15
#define EXTLED_ARM_UP 11
#define EXTLED_ARM_DOWN 0
#define EXTLED_PINCH_OPEN 8
#define EXTLED_PINCH_CLOSE 10

#elifdef CTRL_TYPE_BASIC

#define LED_POWER 2
#define PIN_BATT 3
#define BTN_UP 0
#define BTN_RIGHT 1
#define BTN_LEFT 10
#define BTN_DOWN 4
#define LED_PAIR 5
#define LED_WARN 6
#define LED_SOUNDLIGHT 7
#define LED_ARMPINCH 8
#define LED_NAVIGATION 9
#define BTN_MODE 20
#define BTN_PAIR 21

#endif

#endif //PERSE_BASICCTRL_PINS_H
