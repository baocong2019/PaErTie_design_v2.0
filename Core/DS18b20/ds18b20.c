/*
 * DS18B20 OneWire temperature sensor driver — STM32F103C8T6 @ 72 MHz
 * Pin: PB10 (DQ), requires external 4.7 kΩ pull-up to VDD.
 *
 * All GPIO operations use direct register access to avoid HAL overhead.
 * Microsecond delays use the DWT cycle counter.
 */
#include "ds18b20.h"
#include "main.h"

/* ── PB10 register-level macros ─────────────────────────── */
#define DQ_PIN_NUM    10u                /* PB10 is on CRH */
#define DQ_MASK        GPIO_PIN_10       /* 0x0400 */

#define DQ_H()         (GPIOB->BSRR = DQ_MASK)
#define DQ_L()         (GPIOB->BRR  = DQ_MASK)
#define DQ_RD()        ((GPIOB->IDR & DQ_MASK) != 0u)

/* ── Internal state ──────────────────────────────────────── */
static int16_t  g_temp_tenths = DS18B20_ERROR_VAL;  /* last valid reading */
static uint8_t  g_state      = 0;  /* 0=idle, 1=converting */
static uint8_t  g_dwt_ready  = 0;

/* ── Enable DWT cycle counter for µs delays ──────────────── */
static void dwt_init(void)
{
    if (!g_dwt_ready) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL   |= DWT_CTRL_CYCCNTENA_Msk;
        g_dwt_ready = 1;
    }
}

/* µs delay — 72 MHz → 72 cycles/µs */
static void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000u);
    while ((DWT->CYCCNT - start) < ticks) { /* spin */ }
}

/* ── PB10 direction control (CRH register, no HAL) ───────── */
static void dq_output(void)
{
    uint32_t crh = GPIOB->CRH;
    crh &= ~(0x0FuL << ((DQ_PIN_NUM - 8u) * 4u));  /* clear PB10 */
    crh |=  (0x06uL << ((DQ_PIN_NUM - 8u) * 4u));   /* output OD, 50 MHz */
    GPIOB->CRH = crh;
}

static void dq_input(void)
{
    uint32_t crh = GPIOB->CRH;
    crh &= ~(0x0FuL << ((DQ_PIN_NUM - 8u) * 4u));
    crh |=  (0x08uL << ((DQ_PIN_NUM - 8u) * 4u));   /* input */
    GPIOB->CRH = crh;
    GPIOB->ODR |= DQ_MASK;  /* enable internal pull-up as fallback */
}

/* ── OneWire reset — returns 0=device present ────────────── */
static uint8_t ow_reset(void)
{
    uint8_t pres;

    dq_output();
    DQ_L();
    delay_us(480);              /* master reset pulse ≥ 480 µs */
    DQ_H();                     /* release */
    dq_input();
    delay_us(60);               /* wait 15–60 µs before sampling presence */

    pres = DQ_RD();             /* 0 = device pulled line low → present */

    delay_us(420);              /* wait out the presence pulse */
    dq_output();
    DQ_H();
    return pres;
}

/* ── OneWire bit write ───────────────────────────────────── */
static void ow_write_bit(uint8_t bit)
{
    dq_output();
    DQ_L();
    if (bit) {
        delay_us(1);            /* 1–15 µs low */
        DQ_H();
        delay_us(59);           /* remain high to end of slot */
    } else {
        delay_us(60);           /* 60–120 µs low */
        DQ_H();
        delay_us(1);            /* recovery ≥ 1 µs */
    }
}

/* ── OneWire bit read ────────────────────────────────────── */
static uint8_t ow_read_bit(void)
{
    uint8_t bit;

    dq_output();
    DQ_L();
    delay_us(1);                /* >1 µs low */
    DQ_H();
    dq_input();                 /* float to read */
    delay_us(12);               /* sample within 15 µs of falling edge */

    bit = DQ_RD();

    delay_us(47);               /* remain high to end of 60 µs slot */
    dq_output();
    DQ_H();
    return bit;
}

/* ── OneWire byte write / read ───────────────────────────── */
static void ow_write_byte(uint8_t data)
{
    for (int i = 0; i < 8; i++) {
        ow_write_bit(data & 0x01);
        data >>= 1;
    }
}

static uint8_t ow_read_byte(void)
{
    uint8_t data = 0;
    for (int i = 0; i < 8; i++) {
        data >>= 1;
        if (ow_read_bit()) {
            data |= 0x80;
        }
    }
    return data;
}

/* ══════════════════════════════════════════════════════════
   Public API
   ══════════════════════════════════════════════════════════ */

/**
 * @brief  Initialise DS18B20 driver.
 * @retval 0 = device present, 1 = no device / error
 */
uint8_t DS18B20_Init(void)
{
    dwt_init();
    dq_output();
    DQ_H();
    delay_us(100);
    return ow_reset();          /* check presence */
}

/**
 * @brief  Non-blocking state machine — call every ~100 ms.
 *
 * State 0 — start a new conversion (Skip ROM + Convert T).
 * State 1 — poll DQ; when high, conversion is done → read scratchpad
 *           and store result. Go back to State 0.
 *
 * Worst-case 12‑bit conversion time is 750 ms, so the first
 * reading appears after 1–8 calls.
 */
void DS18B20_Process(void)
{
    if (g_state == 0) {
        /* ── start conversion ── */
        if (ow_reset() != 0) return;        /* no device, skip */
        ow_write_byte(0xCC);                /* Skip ROM */
        ow_write_byte(0x44);                /* Convert T */
        g_state = 1;
    } else {
        /* ── wait for conversion to finish ── */
        dq_input();
        if (!DQ_RD()) return;               /* still converting → try next call */

        /* conversion done — read scratchpad */
        if (ow_reset() != 0) { g_state = 0; return; }
        ow_write_byte(0xCC);                /* Skip ROM */
        ow_write_byte(0xBE);                /* Read Scratchpad */

        uint8_t lsb = ow_read_byte();
        uint8_t msb = ow_read_byte();

        ow_reset();                         /* terminate read */

        /* Convert raw to tenths of °C */
        int16_t raw = (int16_t)((uint16_t)msb << 8) | lsb;
        g_temp_tenths = (int16_t)((raw * 10) / 16);   /* raw / 16.0 × 10 */
        g_state = 0;                        /* ready for next cycle */
    }
}

/**
 * @brief  Return last valid temperature in 0.1 °C.
 */
int16_t DS18B20_GetTempTenths(void)
{
    return g_temp_tenths;
}
