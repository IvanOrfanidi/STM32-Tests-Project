

#include "includes.h"
#include "dht11.h"

void tim_init(TIM_TypeDef* p)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    p->CR1 = 0;
    p->ARR = 0xffff;
    p->PSC = 90;    // 71
    p->EGR = TIM_EGR_UG;
}

void delay_us(TIM_TypeDef* p, uint16_t us)
{
    __disable_interrupt();
    p->CNT = 0;
    while((uint16_t)(p->CNT) <= us)
        ;
    __enable_interrupt();
}

void gpio_cfg(GPIO_TypeDef* p, uint8_t pin, uint8_t mode, uint8_t cnf)
{
    uint32_t _temp = 0;

    if(pin < 8) /* CRL register */
    {
        _temp = p->CRL;
        _temp &= ~(0xf << (pin * 4));    /* clear fields */
        _temp |= mode << (pin * 4);      /* set MODE field */
        _temp |= cnf << ((pin * 4) + 2); /* set CNF field */
        p->CRL = _temp;
    }
    else /* CRH register */
    {
        pin -= 8;
        _temp = p->CRH;
        _temp &= ~(0xf << (pin * 4));    /* clear fields */
        _temp |= mode << (pin * 4);      /* set MODE field */
        _temp |= cnf << ((pin * 4) + 2); /* set CNF field */
        p->CRH = _temp;
    }
}

/* gpio set */
void gpio_set(GPIO_TypeDef* p, uint8_t pin)
{
    p->BSRR = (1 << pin);
}

/* gpio clear */
void gpio_clr(GPIO_TypeDef* p, uint8_t pin)
{
    p->BRR = (1 << pin);
}

/* gpio output data */
uint8_t gpio_get_output(GPIO_TypeDef* p, uint8_t pin)
{
    return (p->ODR >> pin) & 0x1;
}

/* gpio input data */
uint8_t gpio_get_input(GPIO_TypeDef* p, uint8_t pin)
{
    return (p->IDR >> pin) & 0x1;
}

/* dht11_read */
int dht11_read(uint8_t* p)
{
    int i, j, exitCode = DHT11_OK;

#ifdef ENABLE_TIMEOUTS
    int timeout = TIMEOUT_VALUE;
#endif

    CRITICAL_SECTION_DECL;

    ENABLE_TIMER;

    SET_GPIO_AS_OUTPUT;

    CRITICAL_SECTION_PROTECT;

    /* start sequence */
    GPIO_CLEAR;
    DELAY_US(18000);

    GPIO_SET;
    DELAY_US(40);

    SET_GPIO_AS_INPUT;

    while(GPIO_GET_INPUT == 0) /* 80us on '0' */
    {
#ifdef ENABLE_TIMEOUTS
        if(--(timeout) <= 0) {
            exitCode = DHT11_TIMEOUT;
            break;
        }
#endif
    };

#ifdef ENABLE_TIMEOUTS
    timeout = TIMEOUT_VALUE;
#endif
    if(exitCode == DHT11_OK)
        while(GPIO_GET_INPUT == 1) /* 80us on '1' */
        {
#ifdef ENABLE_TIMEOUTS
            if(--(timeout) <= 0) {
                exitCode = DHT11_TIMEOUT;
                break;
            }
#endif
        };
    /* start sequence - end */

    /* read sequence */
    if(exitCode == DHT11_OK)
        for(j = 0; j < 5; j++) {
            for(i = 0; i < 8; i++) {
#ifdef ENABLE_TIMEOUTS
                timeout = TIMEOUT_VALUE;
#endif
                while(GPIO_GET_INPUT == 0) {
#ifdef ENABLE_TIMEOUTS
                    if(--(timeout) <= 0) {
                        exitCode = DHT11_TIMEOUT;
                        break;
                    }
#endif
                }; /* 50 us on 0 */

                if(GPIO_GET_INPUT == 1)
                    DELAY_US(30);

                p[j] <<= 1;

                if(GPIO_GET_INPUT == 1) {
                    DELAY_US(40); /* wait 'till 70us */
                    p[j] |= 1;
                }
                else {
                    p[j] &= 0xfe;
                }
            }
        }
    /* read sequence - end */

    CRITICAL_SECTION_UNPROTECT;
    DISABLE_TIMER;

    /* checksum check */
    if(exitCode == DHT11_OK) {
        if((p[0] + p[2]) != p[4]) {
            exitCode = DHT11_WRONG_CHCKSUM;
        }
        else {
            exitCode = DHT11_OK;
        }
    }

    return exitCode;
}
