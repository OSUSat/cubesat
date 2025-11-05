/**
 ******************************************************************************
 * @file    system_stm32l4xx.c
 * @author  MCD Application Team
 * @brief   CMSIS Cortex-M4 Device Peripheral Access Layer System Source File
 *
 *   This file provides two functions and one global variable to be called from
 *   user application:
 *      - SystemInit(): This function is called at startup just after reset and
 *                      before branch to main program. This call is made inside
 *                      the "startup_stm32l4xx.s" file.
 *
 *      - SystemCoreClock variable: Contains the core clock (HCLK), it can be
 * used by the user application to setup the SysTick timer or configure other
 * parameters.
 *
 *      - SystemCoreClockUpdate(): Updates the variable SystemCoreClock and must
 *                                 be called whenever the core clock is changed
 *                                 during program execution.
 *
 ******************************************************************************
 */

#include <stdint.h>

/* Uncomment the line below according to the target STM32L4 device used in your
 * application */
#if !defined(STM32L496xx)
#define STM32L496xx
#endif

/* Configure the System clock source, PLL Multiplier and Divider factors,
   AHB/APBx prescalers and Flash settings */
#define VECT_TAB_OFFSET                                                        \
    0x00 /*!< Vector Table base offset field.                                  \
              This value must be a multiple of 0x200. */

/* Memory base addresses */
#define FLASH_BASE 0x08000000UL
#define SRAM_BASE 0x20000000UL
#define PERIPH_BASE 0x40000000UL
#define AHB1PERIPH_BASE (PERIPH_BASE + 0x00020000UL)
#define RCC_BASE (AHB1PERIPH_BASE + 0x00001000UL)
#define SCS_BASE (0xE000E000UL)
#define SCB_BASE (SCS_BASE + 0x0D00UL)

/* Register structures */
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t ICSCR;
    volatile uint32_t CFGR;
    volatile uint32_t PLLCFGR;
    volatile uint32_t PLLSAI1CFGR;
    volatile uint32_t PLLSAI2CFGR;
    volatile uint32_t CIER;
    volatile uint32_t CIFR;
    volatile uint32_t CICR;
    uint32_t RESERVED0;
    volatile uint32_t AHB1RSTR;
    volatile uint32_t AHB2RSTR;
    volatile uint32_t AHB3RSTR;
    uint32_t RESERVED1;
    volatile uint32_t APB1RSTR1;
    volatile uint32_t APB1RSTR2;
    volatile uint32_t APB2RSTR;
    uint32_t RESERVED2;
    volatile uint32_t AHB1ENR;
    volatile uint32_t AHB2ENR;
    volatile uint32_t AHB3ENR;
    uint32_t RESERVED3;
    volatile uint32_t APB1ENR1;
    volatile uint32_t APB1ENR2;
    volatile uint32_t APB2ENR;
    uint32_t RESERVED4;
    volatile uint32_t AHB1SMENR;
    volatile uint32_t AHB2SMENR;
    volatile uint32_t AHB3SMENR;
    uint32_t RESERVED5;
    volatile uint32_t APB1SMENR1;
    volatile uint32_t APB1SMENR2;
    volatile uint32_t APB2SMENR;
    uint32_t RESERVED6;
    volatile uint32_t CCIPR;
    uint32_t RESERVED7;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
    volatile uint32_t CRRCR;
    volatile uint32_t CCIPR2;
} RCC_TypeDef;

typedef struct {
    volatile uint32_t ISER[8U];
    uint32_t RESERVED0[24U];
    volatile uint32_t ICER[8U];
    uint32_t RESERVED1[24U];
    volatile uint32_t ISPR[8U];
    uint32_t RESERVED2[24U];
    volatile uint32_t ICPR[8U];
    uint32_t RESERVED3[24U];
    volatile uint32_t IABR[8U];
    uint32_t RESERVED4[56U];
    volatile uint8_t IP[240U];
    uint32_t RESERVED5[644U];
    volatile uint32_t STIR;
} NVIC_Type;

typedef struct {
    volatile uint32_t CPUID;
    volatile uint32_t ICSR;
    volatile uint32_t VTOR;
    volatile uint32_t AIRCR;
    volatile uint32_t SCR;
    volatile uint32_t CCR;
    volatile uint8_t SHP[12U];
    volatile uint32_t SHCSR;
    volatile uint32_t CFSR;
    volatile uint32_t HFSR;
    volatile uint32_t DFSR;
    volatile uint32_t MMFAR;
    volatile uint32_t BFAR;
    volatile uint32_t AFSR;
    volatile uint32_t PFR[2U];
    volatile uint32_t DFR;
    volatile uint32_t ADR;
    volatile uint32_t MMFR[4U];
    volatile uint32_t ISAR[5U];
    uint32_t RESERVED0[5U];
    volatile uint32_t CPACR;
} SCB_Type;

#define RCC ((RCC_TypeDef *)RCC_BASE)
#define SCB ((SCB_Type *)SCB_BASE)

/* Clock variables */
uint32_t SystemCoreClock = 4000000; /* Default to 4 MHz MSI clock */

const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0,
                                   1, 2, 3, 4, 6, 7, 8, 9};
const uint8_t APBPrescTable[8] = {0, 0, 0, 0, 1, 2, 3, 4};
const uint32_t MSIRangeTable[12] = {100000,   200000,   400000,   800000,
                                    1000000,  2000000,  4000000,  8000000,
                                    16000000, 24000000, 32000000, 48000000};

/**
 * @brief  Setup the microcontroller system.
 * @param  None
 * @retval None
 */
void SystemInit(void) {
/* FPU settings ------------------------------------------------------------*/
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    /* Set CP10 and CP11 Full Access */
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2));
#endif

    /* Reset the RCC clock configuration to the default reset state
     * ------------*/
    /* Set MSION bit */
    RCC->CR |= 0x00000001;

    /* Reset CFGR register */
    RCC->CFGR = 0x00000000;

    /* Reset HSEON, CSSON , HSION, and PLLON bits */
    RCC->CR &= 0xEAF6FFFF;

    /* Reset PLLCFGR register */
    RCC->PLLCFGR = 0x00001000;

    /* Reset HSEBYP bit */
    RCC->CR &= 0xFFFBFFFF;

    /* Disable all interrupts */
    RCC->CIER = 0x00000000;

    /* Configure the Vector Table location add offset address
     * ------------------*/
#ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM_BASE |
                VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM */
#else
    SCB->VTOR = FLASH_BASE |
                VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
#endif
}

/**
 * @brief  Update SystemCoreClock variable according to Clock Register Values.
 *         The SystemCoreClock variable contains the core clock (HCLK), it can
 *         be used by the user application to setup the SysTick timer or
 * configure other parameters.
 *
 * @note   Each time the core clock (HCLK) changes, this function must be called
 *         to update SystemCoreClock variable value. Otherwise, any
 * configuration based on this variable will be incorrect.
 *
 * @param  None
 * @retval None
 */
void SystemCoreClockUpdate(void) {
    uint32_t tmp = 0, msirange = 0, pllvco = 0, pllr = 2, pllsource = 0,
             pllm = 2;

    /* Get MSI Range
     * frequency--------------------------------------------------*/
    if ((RCC->CR & 0x00000008) == 0x00000008) { /* MSIRGSEL is set */
        msirange = (RCC->CR & 0x00000F00) >> 8;
    } else { /* MSIRGSEL is not set, use MSISRANGE from RCC_CSR */
        msirange = (RCC->CSR & 0x00000F00) >> 8;
    }

    /* MSI frequency range in HZ*/
    msirange = MSIRangeTable[msirange];

    /* Get SYSCLK source
     * -------------------------------------------------------*/
    switch (RCC->CFGR & 0x0000000C) {
    case 0x00: /* MSI used as system clock source */
        SystemCoreClock = msirange;
        break;
    case 0x04: /* HSI used as system clock source */
        SystemCoreClock = 16000000;
        break;
    case 0x08:                     /* HSE used as system clock source */
        SystemCoreClock = 8000000; /* Default HSE value, modify if different */
        break;
    case 0x0C: /* PLL used as system clock source */
        /* PLL_VCO = (HSE_VALUE or HSI_VALUE or MSI_VALUE/ PLLM) * PLLN
           SYSCLK = PLL_VCO / PLLR
           */
        pllsource = (RCC->PLLCFGR & 0x00000003);
        pllm = ((RCC->PLLCFGR & 0x00000070) >> 4) + 1;

        switch (pllsource) {
        case 0x00: /* No clock selected as PLL entry clock source  */
            pllvco = 0;
            break;
        case 0x01: /* MSI used as PLL entry clock source  */
            pllvco = (msirange / pllm);
            break;
        case 0x02: /* HSI used as PLL entry clock source  */
            pllvco = (16000000 / pllm);
            break;
        case 0x03: /* HSE used as PLL entry clock source  */
            pllvco = (8000000 / pllm);
            break;
        default:
            pllvco = 0;
            break;
        }

        pllvco = pllvco * ((RCC->PLLCFGR & 0x00007F00) >> 8);
        pllr = (((RCC->PLLCFGR & 0x01800000) >> 25) + 1) * 2;
        SystemCoreClock = pllvco / pllr;
        break;
    default:
        SystemCoreClock = msirange;
        break;
    }

    /* Compute HCLK clock frequency
     * --------------------------------------------*/
    /* Get HCLK prescaler */
    tmp = AHBPrescTable[((RCC->CFGR & 0x000000F0) >> 4)];
    /* HCLK clock frequency */
    SystemCoreClock >>= tmp;
}
