/**
 * @file startup_stm32f103.c
 * @brief Minimal bare-metal startup for STM32F103C8T6 (Blue Pill).
 *
 * Responsibilities:
 *  1. Define the Cortex-M3 vector table (16 core + 43 device exceptions).
 *  2. Implement Reset_Handler: copy .data, zero .bss, init clock, run ctors,
 *     then call main().
 *  3. Configure the system clock to 72 MHz via HSE (8 MHz) × PLL × 9.
 *  4. Provide a Default_Handler infinite-loop stub for unused IRQs.
 */

#include <stddef.h>
#include <stdint.h>
#include "stm32f1xx.h"

/* ── Symbols provided by the linker script ───────────────────────────────── */
extern uint32_t _sidata;   /* LMA of .data initialisation image (in flash)  */
extern uint32_t _sdata;    /* VMA start of .data (in RAM)                   */
extern uint32_t _edata;    /* VMA end   of .data (in RAM)                   */
extern uint32_t _sbss;     /* start of .bss                                  */
extern uint32_t _ebss;     /* end   of .bss                                  */
extern uint32_t _estack;   /* initial stack pointer = top of RAM             */

/* C++ static-constructor table (populated by the linker) */
extern void (*__init_array_start[])(void);
extern void (*__init_array_end[])(void);

/* ── Forward declarations ─────────────────────────────────────────────────── */
void Reset_Handler(void);
void Default_Handler(void);
int  main(void);

/* Interrupt handlers are defined as weak aliases so the application can
 * override any of them simply by defining a function with the same name.     */
void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__((weak, alias("Default_Handler")));
void USART1_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));

/* ── Vector table ─────────────────────────────────────────────────────────── */
/* STM32F103xB (medium density): 16 Cortex-M3 core exceptions + 43 device IRQs
 * = 59 entries.  The first entry is the initial stack pointer value.         */
__attribute__((section(".isr_vector"), used))
void (* const g_pfnVectors[])(void) =
{
    /* ── ARM Cortex-M3 core exceptions (indices 0–15) ─────────────────── */
    (void (*)(void))&_estack,  /*  0  Initial stack pointer  */
    Reset_Handler,             /*  1  Reset                  */
    NMI_Handler,               /*  2  NMI                    */
    HardFault_Handler,         /*  3  Hard Fault             */
    MemManage_Handler,         /*  4  Memory Management      */
    BusFault_Handler,          /*  5  Bus Fault              */
    UsageFault_Handler,        /*  6  Usage Fault            */
    0,                         /*  7  Reserved               */
    0,                         /*  8  Reserved               */
    0,                         /*  9  Reserved               */
    0,                         /* 10  Reserved               */
    SVC_Handler,               /* 11  SVCall                 */
    DebugMon_Handler,          /* 12  Debug Monitor          */
    0,                         /* 13  Reserved               */
    PendSV_Handler,            /* 14  PendSV                 */
    SysTick_Handler,           /* 15  SysTick                */

    /* ── STM32F103xB device IRQs (indices 16–58, IRQ 0–42) ────────────── */
    Default_Handler,           /* 16  WWDG            IRQ 0  */
    Default_Handler,           /* 17  PVD             IRQ 1  */
    Default_Handler,           /* 18  TAMPER          IRQ 2  */
    Default_Handler,           /* 19  RTC             IRQ 3  */
    Default_Handler,           /* 20  FLASH           IRQ 4  */
    Default_Handler,           /* 21  RCC             IRQ 5  */
    Default_Handler,           /* 22  EXTI0           IRQ 6  */
    Default_Handler,           /* 23  EXTI1           IRQ 7  */
    Default_Handler,           /* 24  EXTI2           IRQ 8  */
    Default_Handler,           /* 25  EXTI3           IRQ 9  */
    Default_Handler,           /* 26  EXTI4           IRQ 10 */
    Default_Handler,           /* 27  DMA1_Channel1   IRQ 11 */
    Default_Handler,           /* 28  DMA1_Channel2   IRQ 12 */
    Default_Handler,           /* 29  DMA1_Channel3   IRQ 13 */
    Default_Handler,           /* 30  DMA1_Channel4   IRQ 14 */
    Default_Handler,           /* 31  DMA1_Channel5   IRQ 15 */
    Default_Handler,           /* 32  DMA1_Channel6   IRQ 16 */
    Default_Handler,           /* 33  DMA1_Channel7   IRQ 17 */
    Default_Handler,           /* 34  ADC1_2          IRQ 18 */
    Default_Handler,           /* 35  USB_HP_CAN_TX   IRQ 19 */
    Default_Handler,           /* 36  USB_LP_CAN_RX0  IRQ 20 */
    Default_Handler,           /* 37  CAN_RX1         IRQ 21 */
    Default_Handler,           /* 38  CAN_SCE         IRQ 22 */
    Default_Handler,           /* 39  EXTI9_5         IRQ 23 */
    Default_Handler,           /* 40  TIM1_BRK        IRQ 24 */
    Default_Handler,           /* 41  TIM1_UP         IRQ 25 */
    Default_Handler,           /* 42  TIM1_TRG_COM    IRQ 26 */
    Default_Handler,           /* 43  TIM1_CC         IRQ 27 */
    Default_Handler,           /* 44  TIM2            IRQ 28 */
    Default_Handler,           /* 45  TIM3            IRQ 29 */
    Default_Handler,           /* 46  TIM4            IRQ 30 */
    Default_Handler,           /* 47  I2C1_EV         IRQ 31 */
    Default_Handler,           /* 48  I2C1_ER         IRQ 32 */
    Default_Handler,           /* 49  I2C2_EV         IRQ 33 */
    Default_Handler,           /* 50  I2C2_ER         IRQ 34 */
    Default_Handler,           /* 51  SPI1            IRQ 35 */
    Default_Handler,           /* 52  SPI2            IRQ 36 */
    USART1_IRQHandler,         /* 53  USART1          IRQ 37 */
    Default_Handler,           /* 54  USART2          IRQ 38 */
    Default_Handler,           /* 55  USART3          IRQ 39 */
    Default_Handler,           /* 56  EXTI15_10       IRQ 40 */
    Default_Handler,           /* 57  RTCAlarm        IRQ 41 */
    Default_Handler,           /* 58  USBWakeUp       IRQ 42 */
};

/* ── Default_Handler ─────────────────────────────────────────────────────── */
void Default_Handler(void)
{
    while (1) {}
}

/* ── SystemInit: configure PLL for 72 MHz from HSE 8 MHz ────────────────── */
void SystemInit(void)
{
    /* Enable HSE and wait until it is stable */
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY)) {}

    /* Flash: 2 wait states required for SYSCLK 48–72 MHz; enable prefetch */
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | 2U | FLASH_ACR_PRFTBE;

    /* Bus prescalers:
     *   AHB  = SYSCLK / 1  → 72 MHz
     *   APB1 = HCLK  / 2  → 36 MHz  (≤ 36 MHz limit)
     *   APB2 = HCLK  / 1  → 72 MHz  (USART1 is on APB2)
     * PLL source = HSE, multiplier = ×9  → 8 × 9 = 72 MHz                 */
    RCC->CFGR = RCC_CFGR_HPRE_DIV1   |
                RCC_CFGR_PPRE1_DIV2  |
                RCC_CFGR_PPRE2_DIV1  |
                RCC_CFGR_PLLSRC      |
                RCC_CFGR_PLLMULL9;

    /* Enable PLL and wait until it locks */
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY)) {}

    /* Switch system clock to PLL and wait for the switch to complete */
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {}
}

/* ── Reset_Handler ───────────────────────────────────────────────────────── */
void Reset_Handler(void)
{
    /* Copy initialised data from its flash image to RAM */
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* Zero-initialise BSS */
    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0U;
    }

    /* Bring the PLL up to 72 MHz */
    SystemInit();

    /* Run C++ static constructors */
    size_t n = (size_t)(__init_array_end - __init_array_start);
    for (size_t i = 0; i < n; ++i) {
        __init_array_start[i]();
    }

    main();

    /* main() should never return on bare metal */
    while (1) {}
}
