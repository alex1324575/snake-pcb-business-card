#ifndef SNAKE_STM32G030_MIN_H
#define SNAKE_STM32G030_MIN_H

#include <stdint.h>

/* Original minimal register map for this application, not a vendor library.
 * Address/bit facts checked 2026-09-17 against ST RM0454 Rev 5 and:
 * https://github.com/STMicroelectronics/cmsis-device-g0/blob/master/Include/stm32g030xx.h
 * Core registers: ST PM0223 Cortex-M0+ programming manual.
 * Each register below shows its documented peripheral offset. */
#define SC_REG32(address) (*(volatile uint32_t *)(uintptr_t)(address))
#define SC_BIT(n) (UINT32_C(1) << (n))

#define SC_RCC_BASE UINT32_C(0x40021000)
#define SC_RCC_CR SC_REG32(SC_RCC_BASE + 0x00u)
#define SC_RCC_CFGR SC_REG32(SC_RCC_BASE + 0x08u)
#define SC_RCC_APBRSTR1 SC_REG32(SC_RCC_BASE + 0x2cu)
#define SC_RCC_IOPENR SC_REG32(SC_RCC_BASE + 0x34u)
#define SC_RCC_APBENR1 SC_REG32(SC_RCC_BASE + 0x3cu)
#define SC_RCC_CCIPR SC_REG32(SC_RCC_BASE + 0x54u)
#define SC_RCC_BDCR SC_REG32(SC_RCC_BASE + 0x5cu)
#define SC_RCC_CSR SC_REG32(SC_RCC_BASE + 0x60u)
#define SC_RCC_HSION SC_BIT(8)
#define SC_RCC_HSIRDY SC_BIT(10)
#define SC_RCC_HSIDIV (UINT32_C(7) << 11)
#define SC_RCC_CFGR_SW UINT32_C(0x7)
#define SC_RCC_CFGR_SWS (UINT32_C(7) << 3)
#define SC_RCC_CFGR_HPRE (UINT32_C(15) << 8)
#define SC_RCC_CFGR_PPRE (UINT32_C(7) << 12)
#define SC_RCC_GPIOAEN SC_BIT(0)
#define SC_RCC_GPIOBEN SC_BIT(1)
#define SC_RCC_I2C1EN SC_BIT(21)
#define SC_RCC_I2C1RST SC_BIT(21)
#define SC_RCC_DBGEN SC_BIT(27)
#define SC_RCC_PWREN SC_BIT(28)
#define SC_RCC_I2C1SEL (UINT32_C(3) << 12)
#define SC_RCC_BDRST SC_BIT(16)
#define SC_RCC_PWRRSTF SC_BIT(27)
#define SC_RCC_RMVF SC_BIT(23)

#define SC_GPIOA UINT32_C(0x50000000)
#define SC_GPIOB UINT32_C(0x50000400)
#define SC_GPIO_MODER(base) SC_REG32((base) + 0x00u)
#define SC_GPIO_OTYPER(base) SC_REG32((base) + 0x04u)
#define SC_GPIO_OSPEEDR(base) SC_REG32((base) + 0x08u)
#define SC_GPIO_PUPDR(base) SC_REG32((base) + 0x0cu)
#define SC_GPIO_IDR(base) SC_REG32((base) + 0x10u)
#define SC_GPIO_BSRR(base) SC_REG32((base) + 0x18u)
#define SC_GPIO_AFRL(base) SC_REG32((base) + 0x20u)

#define SC_PWR_CR1 SC_REG32(UINT32_C(0x40007000))
#define SC_PWR_DBP SC_BIT(8)
#define SC_DBG_APBFZ1 SC_REG32(UINT32_C(0x40015808))
#define SC_DBG_IWDG_STOP SC_BIT(12)

#define SC_I2C_BASE UINT32_C(0x40005400)
#define SC_I2C_CR1 SC_REG32(SC_I2C_BASE + 0x00u)
#define SC_I2C_CR2 SC_REG32(SC_I2C_BASE + 0x04u)
#define SC_I2C_OAR1 SC_REG32(SC_I2C_BASE + 0x08u)
#define SC_I2C_OAR2 SC_REG32(SC_I2C_BASE + 0x0cu)
#define SC_I2C_TIMINGR SC_REG32(SC_I2C_BASE + 0x10u)
#define SC_I2C_TIMEOUTR SC_REG32(SC_I2C_BASE + 0x14u)
#define SC_I2C_ISR SC_REG32(SC_I2C_BASE + 0x18u)
#define SC_I2C_ICR SC_REG32(SC_I2C_BASE + 0x1cu)
#define SC_I2C_TXDR SC_REG32(SC_I2C_BASE + 0x28u)
#define SC_I2C_PE SC_BIT(0)
#define SC_I2C_START SC_BIT(13)
#define SC_I2C_AUTOEND SC_BIT(25)
#define SC_I2C_TXIS SC_BIT(1)
#define SC_I2C_NACKF SC_BIT(4)
#define SC_I2C_STOPF SC_BIT(5)
#define SC_I2C_BERR SC_BIT(8)
#define SC_I2C_ARLO SC_BIT(9)
#define SC_I2C_OVR SC_BIT(10)
#define SC_I2C_TIMEOUT SC_BIT(12)
#define SC_I2C_BUSY SC_BIT(15)
/* These ISR flags have corresponding ICR clear bits at the same positions. */
#define SC_I2C_ERRORS (SC_I2C_NACKF | SC_I2C_ARLO | SC_I2C_OVR | SC_I2C_TIMEOUT)
#define SC_I2C_CLEAR (SC_I2C_ERRORS | SC_I2C_STOPF | SC_I2C_BERR)

#define SC_IWDG_BASE UINT32_C(0x40003000)
#define SC_IWDG_KR SC_REG32(SC_IWDG_BASE + 0x00u)
#define SC_IWDG_PR SC_REG32(SC_IWDG_BASE + 0x04u)
#define SC_IWDG_RLR SC_REG32(SC_IWDG_BASE + 0x08u)
#define SC_IWDG_SR SC_REG32(SC_IWDG_BASE + 0x0cu)
#define SC_IWDG_WINR SC_REG32(SC_IWDG_BASE + 0x10u)

#define SC_SYST_CSR SC_REG32(UINT32_C(0xe000e010))
#define SC_SYST_RVR SC_REG32(UINT32_C(0xe000e014))
#define SC_SYST_CVR SC_REG32(UINT32_C(0xe000e018))
#define SC_SCB_AIRCR SC_REG32(UINT32_C(0xe000ed0c))
#define SC_SCB_SCR SC_REG32(UINT32_C(0xe000ed10))
#define SC_UID_WORD(n) SC_REG32(UINT32_C(0x1fff7590) + 4u * (n))

static inline void sc_irq_disable(void) { __asm volatile ("cpsid i" ::: "memory"); }
static inline void sc_irq_enable(void) { __asm volatile ("cpsie i" ::: "memory"); }
static inline void sc_dsb(void) { __asm volatile ("dsb 0xf" ::: "memory"); }
static inline void sc_isb(void) { __asm volatile ("isb 0xf" ::: "memory"); }
static inline void sc_wfi(void) { __asm volatile ("wfi" ::: "memory"); }
static inline void sc_nop(void) { __asm volatile ("nop"); }

#endif
