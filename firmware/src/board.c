#include "board.h"
#include "stm32g030_min.h"

#define SDB_PIN SC_BIT(5)
#define SCL_PIN SC_BIT(6)
#define SDA_PIN SC_BIT(7)
#define BUS_PINS (SCL_PIN | SDA_PIN)
#define BUS_MODE_MASK ((UINT32_C(3) << 12) | (UINT32_C(3) << 14))
#define BUS_AF_MODE ((UINT32_C(2) << 12) | (UINT32_C(2) << 14))
#define BUS_OUTPUT_MODE ((UINT32_C(1) << 12) | (UINT32_C(1) << 14))
#define BUS_TIMEOUT_MS 30u
#define POLL_GUARD UINT32_C(500000)
#define I2C_TIMING_16MHZ UINT32_C(0x10f11f2f)

static volatile uint32_t milliseconds;
static uint32_t reset_cause;
static bool i2c_ready;

static bool wait_register(volatile uint32_t *reg, uint32_t mask,
                          uint32_t expected)
{
    /* Before SysTick exists, a loop bound is the independent escape path. */
    for (uint32_t n = 0; n < POLL_GUARD; ++n) {
        if ((*reg & mask) == expected) {
            return true;
        }
    }
    return false;
}

static void fatal_reset(void) __attribute__((noreturn));
static void fatal_reset(void)
{
    SC_GPIO_BSRR(SC_GPIOA) = SDB_PIN << 16;
    sc_irq_disable();
    sc_dsb();
    SC_SCB_AIRCR = UINT32_C(0x05fa0004); /* VECTKEY + SYSRESETREQ */
    sc_dsb();
    for (;;) {
        sc_nop(); /* The reset request or the already running IWDG resets us. */
    }
}

static void configure_i2c(bool enable)
{
    SC_RCC_APBRSTR1 |= SC_RCC_I2C1RST;
    (void)SC_RCC_APBRSTR1;
    SC_RCC_APBRSTR1 &= ~SC_RCC_I2C1RST;
    SC_I2C_CR1 = 0u;
    SC_I2C_CR2 = 0u;
    SC_I2C_OAR1 = 0u;
    SC_I2C_OAR2 = 0u;
    SC_I2C_TIMINGR = I2C_TIMING_16MHZ;
    SC_I2C_TIMEOUTR = 0u;
    SC_I2C_ICR = SC_I2C_CLEAR;
    /* ANFOFF=0 and DNF=0: analog filtering enabled, digital filtering off. */
    SC_I2C_CR1 = enable ? SC_I2C_PE : 0u;
    i2c_ready = enable;
}

void board_init(void)
{
    sc_irq_disable();
    SC_RCC_IOPENR |= SC_RCC_GPIOAEN | SC_RCC_GPIOBEN;
    (void)SC_RCC_IOPENR;
    /* Preload low before the mode change; leave SWD pins PA13/PA14 intact. */
    SC_GPIO_BSRR(SC_GPIOA) = SDB_PIN << 16;
    SC_GPIO_OTYPER(SC_GPIOA) &= ~SDB_PIN;
    SC_GPIO_PUPDR(SC_GPIOA) &= ~(UINT32_C(3) << 10);
    SC_GPIO_MODER(SC_GPIOA) =
        (SC_GPIO_MODER(SC_GPIOA) & ~(UINT32_C(3) << 10)) | SC_BIT(10);

    reset_cause = SC_RCC_CSR;
    SC_RCC_APBENR1 |= SC_RCC_PWREN | SC_RCC_DBGEN;
    (void)SC_RCC_APBENR1;
    if ((reset_cause & SC_RCC_PWRRSTF) != 0u) {
        /* ES0486 2.2.10: stale LSCO state can otherwise drive button PA2. */
        SC_PWR_CR1 |= SC_PWR_DBP;
        if (!wait_register(&SC_PWR_CR1, SC_PWR_DBP, SC_PWR_DBP)) {
            fatal_reset();
        }
        SC_RCC_BDCR = SC_RCC_BDRST;
        (void)SC_RCC_BDCR;
        SC_RCC_BDCR = 0u;
        SC_PWR_CR1 &= ~SC_PWR_DBP;
    }
    SC_RCC_CSR |= SC_RCC_RMVF;

    SC_RCC_CR |= SC_RCC_HSION;
    if (!wait_register(&SC_RCC_CR, SC_RCC_HSIRDY, SC_RCC_HSIRDY)) {
        fatal_reset();
    }
    SC_RCC_CR &= ~SC_RCC_HSIDIV;
    SC_RCC_CFGR &= ~SC_RCC_CFGR_SW;
    if (!wait_register(&SC_RCC_CFGR, SC_RCC_CFGR_SWS, 0u)) {
        fatal_reset();
    }
    SC_RCC_CFGR &= ~(SC_RCC_CFGR_HPRE | SC_RCC_CFGR_PPRE);
    sc_dsb();
    sc_isb();

    /* Existing 10k external pull-ups supply PA0..PA4; inputs never source. */
    SC_GPIO_MODER(SC_GPIOA) &= ~UINT32_C(0x3ff);
    SC_GPIO_PUPDR(SC_GPIOA) &= ~UINT32_C(0x3ff);
    SC_GPIO_BSRR(SC_GPIOB) = BUS_PINS;
    SC_GPIO_OTYPER(SC_GPIOB) |= BUS_PINS;
    SC_GPIO_PUPDR(SC_GPIOB) &= ~BUS_MODE_MASK;
    SC_GPIO_OSPEEDR(SC_GPIOB) =
        (SC_GPIO_OSPEEDR(SC_GPIOB) & ~BUS_MODE_MASK) | BUS_OUTPUT_MODE;
    SC_GPIO_AFRL(SC_GPIOB) =
        (SC_GPIO_AFRL(SC_GPIOB) & ~UINT32_C(0xff000000)) |
        UINT32_C(0x66000000);
    SC_GPIO_MODER(SC_GPIOB) =
        (SC_GPIO_MODER(SC_GPIOB) & ~BUS_MODE_MASK) | BUS_AF_MODE;
    SC_RCC_CCIPR &= ~SC_RCC_I2C1SEL; /* PCLK, fixed at 16 MHz here. */
    SC_RCC_APBENR1 |= SC_RCC_I2C1EN;
    (void)SC_RCC_APBENR1;
    configure_i2c(true);

    milliseconds = 0u;
    SC_SYST_CSR = 0u;
    SC_SYST_RVR = 15999u;
    SC_SYST_CVR = 0u;
    SC_SCB_SCR &= ~SC_BIT(2); /* WFI uses Sleep, never Stop/Standby. */
    SC_SYST_CSR = 7u; /* CPU clock, interrupt, counter enable. */
    SC_DBG_APBFZ1 |= SC_DBG_IWDG_STOP;
    sc_irq_enable();

    /* /64, 1000 ticks at nominal 32 kHz: 2 seconds. No window restriction. */
    SC_IWDG_KR = UINT32_C(0xcccc);
    SC_IWDG_KR = UINT32_C(0x5555);
    SC_IWDG_PR = 4u;
    SC_IWDG_RLR = 999u;
    if (!wait_register(&SC_IWDG_SR, 7u, 0u)) {
        fatal_reset();
    }
    SC_IWDG_WINR = UINT32_C(0xfff);
    SC_IWDG_KR = UINT32_C(0xaaaa); /* Initial load, not a background feed. */
}

void SysTick_Handler(void)
{
    ++milliseconds;
}

uint32_t board_millis(void)
{
    return milliseconds;
}

uint8_t board_buttons(void)
{
    return (uint8_t)(~SC_GPIO_IDR(SC_GPIOA) & UINT32_C(0x1f));
}

uint32_t board_seed(void)
{
    const uint32_t a = SC_UID_WORD(0);
    const uint32_t b = SC_UID_WORD(1);
    const uint32_t c = SC_UID_WORD(2);
    const uint32_t seed = a ^ ((b << 11) | (b >> 21)) ^
                          ((c << 23) | (c >> 9)) ^ reset_cause ^
                          (board_millis() * UINT32_C(0x9e3779b9)) ^ SC_SYST_CVR;
    return seed != 0u ? seed : UINT32_C(0x6d2b79f5);
}

void board_sdb(bool high, void *user)
{
    (void)user;
    SC_GPIO_BSRR(SC_GPIOA) = high ? SDB_PIN : SDB_PIN << 16;
}

void board_delay_ms(uint32_t duration, void *user)
{
    (void)user;
    const uint32_t start = board_millis();
    uint32_t previous = start;
    uint32_t guard = POLL_GUARD;
    while ((uint32_t)(board_millis() - start) < duration) {
        const uint32_t now = board_millis();
        if (now != previous) {
            previous = now;
            guard = POLL_GUARD;
        } else if (--guard == 0u) {
            fatal_reset(); /* A failed tick cannot turn a delay into a hang. */
        }
    }
}

static bool bus_abort(void)
{
    board_sdb(false, NULL);
    SC_I2C_CR1 = 0u;
    SC_I2C_ICR = SC_I2C_CLEAR;
    i2c_ready = false;
    return false;
}

static bool wait_bus_flag(uint32_t flag, uint32_t start, uint32_t *guard)
{
    while (*guard != 0u) {
        --*guard;
        const uint32_t status = SC_I2C_ISR;
        if ((status & SC_I2C_BERR) != 0u) {
            /* ES0486 2.8.2: a lone BERR must not abort a healthy transfer. */
            SC_I2C_ICR = SC_I2C_BERR;
        }
        if ((status & SC_I2C_ERRORS) != 0u) {
            return false;
        }
        if ((status & flag) != 0u) {
            return true;
        }
        if ((status & SC_I2C_STOPF) != 0u ||
            (uint32_t)(board_millis() - start) >= BUS_TIMEOUT_MS) {
            return false;
        }
    }
    return false;
}

bool board_i2c_write(uint8_t address, const uint8_t *data, size_t length,
                     void *user)
{
    (void)user;
    if (!i2c_ready || address > 0x7fu || data == NULL || length == 0u ||
        length > 255u) {
        return bus_abort();
    }
    const uint32_t start = board_millis();
    uint32_t guard = POLL_GUARD;
    while ((SC_I2C_ISR & SC_I2C_BUSY) != 0u ||
           (SC_GPIO_IDR(SC_GPIOB) & BUS_PINS) != BUS_PINS) {
        if (--guard == 0u ||
            (uint32_t)(board_millis() - start) >= BUS_TIMEOUT_MS) {
            return bus_abort();
        }
    }
    SC_I2C_ICR = SC_I2C_CLEAR;
    SC_I2C_CR2 = ((uint32_t)address << 1) | ((uint32_t)length << 16) |
                 SC_I2C_AUTOEND | SC_I2C_START;
    for (size_t i = 0; i < length; ++i) {
        if (!wait_bus_flag(SC_I2C_TXIS, start, &guard)) {
            return bus_abort();
        }
        SC_I2C_TXDR = data[i];
    }
    if (!wait_bus_flag(SC_I2C_STOPF, start, &guard)) {
        return bus_abort();
    }
    SC_I2C_ICR = SC_I2C_STOPF;
    return true;
}

static void bus_half_period(void)
{
    /* At 16 MHz, 160 NOPs alone take >=10 us. Loop overhead adds margin. */
    for (uint32_t n = 0; n < 160u; ++n) {
        sc_nop();
    }
}

static bool wait_scl_released(void)
{
    const uint32_t start = board_millis();
    for (uint32_t n = 0; n < POLL_GUARD; ++n) {
        if ((SC_GPIO_IDR(SC_GPIOB) & SCL_PIN) != 0u) {
            return true;
        }
        if ((uint32_t)(board_millis() - start) >= 2u) {
            return false;
        }
    }
    return false;
}

bool board_i2c_recover(void)
{
    bool released = false;
    board_sdb(false, NULL);
    SC_I2C_CR1 = 0u;
    i2c_ready = false;
    SC_GPIO_BSRR(SC_GPIOB) = BUS_PINS;
    SC_GPIO_OTYPER(SC_GPIOB) |= BUS_PINS;
    SC_GPIO_MODER(SC_GPIOB) =
        (SC_GPIO_MODER(SC_GPIOB) & ~BUS_MODE_MASK) | BUS_OUTPUT_MODE;
    if (!wait_scl_released()) {
        goto restore;
    }
    for (uint32_t pulse = 0; pulse < 9u &&
         (SC_GPIO_IDR(SC_GPIOB) & SDA_PIN) == 0u; ++pulse) {
        SC_GPIO_BSRR(SC_GPIOB) = SCL_PIN << 16;
        bus_half_period();
        SC_GPIO_BSRR(SC_GPIOB) = SCL_PIN;
        if (!wait_scl_released()) {
            goto restore;
        }
        bus_half_period();
    }
    /* STOP: SDA low while SCL low, release SCL, then release SDA. */
    SC_GPIO_BSRR(SC_GPIOB) = BUS_PINS << 16;
    bus_half_period();
    SC_GPIO_BSRR(SC_GPIOB) = SCL_PIN;
    if (!wait_scl_released()) {
        goto restore;
    }
    bus_half_period();
    SC_GPIO_BSRR(SC_GPIOB) = SDA_PIN;
    bus_half_period();
    released = (SC_GPIO_IDR(SC_GPIOB) & BUS_PINS) == BUS_PINS;

restore:
    SC_GPIO_BSRR(SC_GPIOB) = BUS_PINS;
    SC_GPIO_MODER(SC_GPIOB) =
        (SC_GPIO_MODER(SC_GPIOB) & ~BUS_MODE_MASK) | BUS_AF_MODE;
    configure_i2c(released);
    return released;
}

void board_watchdog_feed(void)
{
    SC_IWDG_KR = UINT32_C(0xaaaa);
}

void board_idle(void)
{
    sc_dsb();
    sc_wfi();
}
