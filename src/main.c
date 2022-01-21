#include "stm8s.h"
#include "milis.h"
#include "stm8_hd44780.h"
#include "delay.h"

 
void setup(void)
{
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);      // taktovani MCU na 16MHz
    lcd_init();
    init_milis();
    GPIO_Init(GPIOC,GPIO_PIN_3,GPIO_MODE_OUT_PP_LOW_SLOW); // nastavíme PC5 jako výstup typu push-pull (LEDka)
    GPIO_Init(GPIOE, GPIO_PIN_4,GPIO_MODE_IN_FL_NO_IT); // nastavíme PE4 jako vstup (tlačítko)
    GPIO_Init(GPIOC,
                 ((GPIO_Pin_TypeDef)(GPIO_PIN_1 | GPIO_PIN_2)),
                 GPIO_MODE_IN_PU_NO_IT); //inicializace pinů C1 a C2 pro enkoder 
    TIM1_DeInit();
    TIM1_TimeBaseInit(8, TIM1_COUNTERMODE_UP, 60, 8); //inicializace enkoderu
    TIM1_EncoderInterfaceConfig(TIM1_ENCODERMODE_TI12,
                                 TIM1_ICPOLARITY_FALLING,
                                 TIM1_ICPOLARITY_FALLING);
   TIM1_Cmd(ENABLE);
}

void lcd_print(unsigned char x_pos, unsigned char y_pos, unsigned int value) //funkce pro vypisování čísel na LCD
{
       char tmp[4] = {0x20, 0x20, '\0'} ;
       tmp[0] = (((value / 10) % 10) + 0x30);
       tmp[1] = ((value % 10) + 0x30);
       lcd_gotoxy(x_pos, y_pos);
       lcd_puts(tmp);  
}

void signalizace(void) //funkce která bude signalizovat konec odpočtu blikáním ledkou
{
    uint8_t signalizace_hodnota = 10; // proměnné pro blikání
    uint32_t time3 = milis(); // proměnné pro blikání
    while(signalizace_hodnota){ //každý cyklus se kontroluje jestli uběhlo 300ms pokud ano tak se reverzne pin a odečte se zbývajicí počet kroků                       
                if (milis() - time3 > 300) {
                    time3 = milis();                  
                    GPIO_WriteReverse(GPIOC,GPIO_PIN_3);
                    signalizace_hodnota -= 1;
                }
    }
}


int main(void)
{
    uint32_t time = 0; //inicializace proměnných
    uint32_t time2 = 0;
    unsigned int present_value = 0x0000;
    unsigned int previous_value = 0x0001;
    uint8_t tlacitko = 0;
    setup();

    lcd_gotoxy(0,0);
    lcd_puts("nastaveny");
    lcd_gotoxy(0,1);
    lcd_puts("zbivajici"); //vypsání textu na LCD
    while (1) {
        if(GPIO_ReadInputPin(GPIOE,GPIO_PIN_4)==RESET){ //při stisknutí tlačítka se vezme hodnota co je na enkoderu a pak se spustí stopky s tím časem
            time2 = milis();
            time = present_value * 1000;
            tlacitko = 1;
            while(tlacitko){ //loop se opakuje tak dlouho dokud nedojede čas
                lcd_print(12, 1, ((time2 + time) - milis()) / 1000);                            
                if (milis() - time2 > time) {                    
                    signalizace();
                    tlacitko = 0;
                }
            }
        }

//získávání aktuální hodnoty z enkoderu
        present_value = TIM1_GetCounter();

        if(present_value != previous_value)
        {
            lcd_print(12, 0, present_value);
        }
        previous_value = present_value;

    }
}

/*-------------------------------  Assert -----------------------------------*/
#include "__assert__.h"
