#ifndef WATCHDOG_H_
#define WATCHDOG_H_
#include "stdlib.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    WDOG_BUS_CLOCK                        = 0x00U, /*!< Bus clock */
    WDOG_LPO_CLOCK                        = 0x01U, /*!< LPO clock */
    WDOG_SOSC_CLOCK                       = 0x02U, /*!< SOSC clock */
    WDOG_SIRC_CLOCK                       = 0x03U  /*!< SIRC clock */
} wdog_clk_source_t;

/*!
 * @brief Test modes for the WDOG.
 * Implements : wdog_test_mode_t_Class
 */
typedef enum
{
    WDOG_TST_DISABLED                     = 0x00U, /*!< Test mode disabled */
    WDOG_TST_USER                         = 0x01U, /*!< User mode enabled. (Test mode disabled.) */
    WDOG_TST_LOW                          = 0x02U, /*!< Test mode enabled, only the low byte is used. */
    WDOG_TST_HIGH                         = 0x03U  /*!< Test mode enabled, only the high byte is used. */
} wdog_test_mode_t;

/*!
 * @brief set modes for the WDOG.
 * Implements : wdog_set_mode_t_Class
 */
typedef enum
{
    WDOG_DEBUG_MODE                        = 0x00U, /*!< Debug mode */
    WDOG_WAIT_MODE                         = 0x01U, /*!< Wait mode */
    WDOG_STOP_MODE                         = 0x02U  /*!< Stop mode */
} wdog_set_mode_t;

/*!
 * @brief WDOG option mode configuration structure
 * Implements : wdog_op_mode_t_Class
 */
typedef struct
{
    bool wait;  /*!< Wait mode */
    bool stop;  /*!< Stop mode */
    bool debug; /*!< Debug mode */
} wdog_op_mode_t;

/*!
 * @brief WDOG user configuration structure
 * Implements : wdog_user_config_t_Class
 */
typedef struct
{
    wdog_clk_source_t    clkSource;       /*!< The clock source of the WDOG */
    wdog_op_mode_t       opMode;          /*!< The modes in which the WDOG is functional */
    bool                 updateEnable;    /*!< If true, further updates of the WDOG are enabled */
    bool                 intEnable;       /*!< If true, an interrupt request is generated before reset */
    bool                 winEnable;       /*!< If true, window mode is enabled */
    uint16_t             windowValue;     /*!< The window value */
    uint16_t             timeoutValue;    /*!< The timeout value */
    bool                 prescalerEnable; /*!< If true, a fixed 256 prescaling of the counter reference clock is enabled */
} wdog_user_config_t;

int wdog_enable(wdog_user_config_t wdogUserConfig);
void wdog_disable(void);
void wdog_refresh(void);
void wdog_unlock(void);
void wdog_SetWindow(bool enable, uint16_t windowvalue);
int wodg_SetIntrupt(bool enable);
void wdog_SetTimeout(uint16_t timeout);
uint16_t wodg_GetCounter(void);

void EWM_init(void);
void WDOG_EWM_IRQHandler(void);
void WDOG_EWM_Enable_Interrupt(void);

#endif
