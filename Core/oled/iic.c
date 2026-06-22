/*
 * Simple software I2C (bit-bang) driver for STM32 HAL
 * Pins used: OLED_SCL = PA7, OLED_SDA = PA6
 * Optimized with direct register access instead of HAL_GPIO_Init
 */
#include "iic.h"
#include "main.h"

/* Pin numbers (0-15) for CRL/CRH register shifting */
#define IIC_SDA_PIN_NUM   6   /* PA6 */
#define IIC_SCL_PIN_NUM   7   /* PA7 */

/* ── direct register macros ────────────────────────────────── */
#define SCL_H()   (GPIOA->BSRR = OLED_SCL_Pin)
#define SCL_L()   (GPIOA->BRR  = OLED_SCL_Pin)
#define SDA_H()   (GPIOA->BSRR = OLED_SDA_Pin)
#define SDA_L()   (GPIOA->BRR  = OLED_SDA_Pin)
#define SDA_RD()  ((GPIOA->IDR & OLED_SDA_Pin) != 0)

/* ── I2C half-bit delay (~800ns @72MHz, suitable for 400kHz I2C) ── */
static void iic_delay(void)
{
    for (volatile int i = 0; i < 15; i++) {
        __NOP();
    }
}

/* ── SDA direction control (direct CRL write, no HAL) ── */
static void sda_output(void)
{
    uint32_t crl = GPIOA->CRL;
    crl &= ~(0x0FUL << (IIC_SDA_PIN_NUM * 4));
    crl |=  (0x06UL << (IIC_SDA_PIN_NUM * 4));   /* Output open-drain, 50 MHz */
    GPIOA->CRL = crl;
}

static void sda_input(void)
{
    uint32_t crl = GPIOA->CRL;
    crl &= ~(0x0FUL << (IIC_SDA_PIN_NUM * 4));
    crl |=  (0x08UL << (IIC_SDA_PIN_NUM * 4));   /* Input */
    GPIOA->CRL = crl;
    GPIOA->ODR |= OLED_SDA_Pin;                   /* internal pull-up */
}

/* ── public API ────────────────────────────────────────────── */
uint8_t iic_init(void)
{
    SCL_H();
    SDA_H();
    sda_output();
    return 0;
}

uint8_t iic_deinit(void)
{
    uint32_t crl = GPIOA->CRL;
    crl &= ~((0x0FUL << (IIC_SCL_PIN_NUM * 4)) | (0x0FUL << (IIC_SDA_PIN_NUM * 4)));
    crl |=  (0x08UL << (IIC_SCL_PIN_NUM * 4)) | (0x08UL << (IIC_SDA_PIN_NUM * 4));
    GPIOA->CRL = crl;
    return 0;
}

/* ── I2C bus operations ────────────────────────────────────── */
static void iic_start(void)
{
    sda_output();
    SDA_H();
    SCL_H();
    iic_delay();
    SDA_L();
    iic_delay();
    SCL_L();
}

static void iic_stop(void)
{
    sda_output();
    SCL_L();
    SDA_L();
    iic_delay();
    SCL_H();
    iic_delay();
    SDA_H();
    iic_delay();
}

/* Write one byte, return 0 = ACK, 1 = NACK */
static uint8_t iic_write_byte(uint8_t data)
{
    sda_output();
    for (int i = 0; i < 8; i++) {
        if (data & 0x80) SDA_H();
        else             SDA_L();
        data <<= 1;
        iic_delay();
        SCL_H();
        iic_delay();
        SCL_L();
    }
    /* read ACK */
    sda_input();
    iic_delay();
    SCL_H();
    iic_delay();
    uint8_t ack = SDA_RD();
    SCL_L();
    sda_output();
    return ack;
}

/* Write buffer: addr = 7-bit I2C address already in 8-bit format,
   reg = first control byte, buf = data, len = data length */
uint8_t iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    iic_start();
    if (iic_write_byte(addr)) { iic_stop(); return 1; }
    if (iic_write_byte(reg))  { iic_stop(); return 1; }
    for (uint16_t i = 0; i < len; i++) {
        if (iic_write_byte(buf[i])) { iic_stop(); return 1; }
    }
    iic_stop();
    return 0;
}
