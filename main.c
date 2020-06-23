/*******************************************************************/ /*!
*    \file       main.c
*    \brief      See main.h for the description
*    \date       Created: 2020-06-23
*    \author     Karina Avalos <avaloskarina04@gmail.com>
***********************************************************************/

//=======================================================================
//=  MODULE INCLUDE FILE
#include "main.h"

//=======================================================================
//=  LIBRARY/OTHER INCLUDE FILES
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"
#include "gpio.h"
#include "utils.h"
#include "timer.h"
#include "systick.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1351.h"
#include "glcdfont.h"
#include "uart.h"
#include "spi.h"
#include "uart_if.h"
#include "pin_mux_config.h"
#include "gpio_if.h"
#include "timer_if.h"

//=======================================================================
//=  SYSTEM/STANDARD INCLUDE FILES
//=    E.g. #include <stdbool.h>
#include <stdio.h>
#include <string.h>

//=======================================================================
//=  PRIVATE OBJECT-LIKE/FUNCTION-LIKE MACROS
//=    E.g. #define BUFFER_SIZE 1024
//=    E.g. #define SQR(s)  ((s) * (s))
#define BUTTON_STR_LEN 31
#define SIGNAL_BUFFER_LEN 40
#define MAX_CHAR_LEN 26
//=======================================================================
//=  GLOBAL VARIABLES (extern)
//=    Declared with keyword `extern` in the .h file.
//=    E.g. int8_t g_error_variable;

//=======================================================================
//=  PRIVATE TYPE DEFINITIONS
//=    E.g. typedef private to this source file
//=    typedef struct
//=    {
//=       uint16_t count;
//=    } timer_t;

//=======================================================================
//=  PRIVATE VARIABLE DEFINITIONS (static)
//=    Definition of private constants, variables and structures at file scope
//=    E.g. static uint8_t variable_name;
//=    E.g. static const uint8_t variable_name_k;

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
extern void (*const g_pfnVectors[])(void);

static volatile unsigned long g_ulSysTickValue;
static volatile unsigned long g_ulBase;
static volatile unsigned long g_ulRefBase;
static volatile unsigned long g_ulRefTimerInts = 0;
static volatile unsigned long g_ulIntClearVector;
unsigned long                 g_ulTimerInts;
volatile unsigned uint16_t    timeCount       = 0;
volatile unsigned uint16_t    timeDiff        = 0;
volatile unsigned uint16_t    oldCount        = 0;
volatile bool                 in_transmission = 0; // to indicate if we are sending a signal
volatile uint16_t             counter         = 0;
uint16_t                      done            = 0;
volatile unsigned uint16_t    signalBuff[SIGNAL_BUFFER_LEN];                      // buffer for our signal
volatile uint16_t             index;                                              // to keep track of signal buffer
char                          alpha[MAX_CHAR_LEN] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; // string that contains all possible characters that can be displayed

// The representation for the buttons
unsigned char zero[BUTTON_STR_LEN]  = {'0', '1', '0', '1', '0', '0', '0', '1', '1', '0', '1', '0', '1', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '1', '1', '1', '1', '1', '1'};
unsigned char one[BUTTON_STR_LEN]   = {'0', '1', '0', '1', '0', '0', '0', '1', '1', '0', '1', '0', '1', '1', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '1', '1', '1', '1', '1'};
unsigned char two[BUTTON_STR_LEN]   = {'0', '1', '0', '1', '0', '0', '0', '1', '1', '0', '1', '0', '1', '1', '1', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '1', '1', '1', '1', '1', '1'};
unsigned char three[BUTTON_STR_LEN] = {'0', '1', '0', '1', '0', '0', '0', '1', '1', '0', '1', '0', '1', '1', '1', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '1', '1', '1', '1'};
unsigned char four[BUTTON_STR_LEN]  = {'0', '1', '0', '1', '0', '0', '0', '1', '1', '0', '1', '0', '1', '1', '1', '0', '0', '0', '1', '0', '0', '0', '0', '1', '1', '1', '0', '1', '1', '1', '1'};
unsigned char five[BUTTON_STR_LEN]  = {'0', '1', '0', '1', '0', '0', '0', '1', '1', '0', '1', '0', '1', '1', '1', '1', '0', '0', '1', '0', '0', '0', '0', '0', '1', '1', '0', '1', '1', '1', '1'};
unsigned char six[BUTTON_STR_LEN]   = {'0', '1', '0', '1', '0', '0', '0', '1', '1', '0', '1', '0', '1', '1', '1', '0', '1', '0', '1', '0', '0', '0', '0', '1', '0', '1', '0', '1', '1', '1', '1'};
unsigned char seven[BUTTON_STR_LEN] = {'0', '1', '0', '1', '0', '0', '0', '1', '1', '0', '1', '0', '1', '1', '1', '1', '1', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '1', '1', '1', '1'};
unsigned char eight[BUTTON_STR_LEN] = {'0', '1', '0', '1', '0', '0', '0', '1', '1', '0', '1', '0', '1', '1', '1', '0', '0', '0', '0', '1', '0', '0', '0', '1', '1', '1', '1', '0', '1', '1', '1'};
unsigned char nine[BUTTON_STR_LEN]  = {'0', '1', '0', '1', '0', '0', '0', '1', '1', '0', '1', '0', '1', '1', '1', '1', '0', '0', '0', '1', '0', '0', '0', '0', '1', '1', '1', '0', '1', '1', '1'};
unsigned char enter[BUTTON_STR_LEN] = {'0', '1', '0', '1', '0', '0', '0', '1', '1', '0', '1', '0', '1', '1', '1', '0', '1', '0', '1', '1', '0', '0', '0', '1', '0', '1', '0', '0', '1', '1', '1'};
unsigned char vol[BUTTON_STR_LEN]   = {'0', '1', '0', '1', '0', '0', '0', '1', '1', '0', '1', '0', '1', '1', '1', '1', '0', '0', '1', '1', '0', '0', '0', '0', '1', '1', '0', '0', '1', '1', '1'};

unsigned long str[500];       // string to hold what we are sending
uint16_t      t          = 0; // to keep track of length of string being send
uint16_t      u          = 0; // to iterate through list of possible letters that can be sent using a single button
uint16_t      itr        = 0;
uint16_t      itr2       = 0;  // y position for recieved messaged
uint16_t      prevButton = 12; // initialize the previous number with a number that is not valid
uint16_t      flag       = 0;  // to begin sending
uint16_t      x          = 6;  //current positon
uint16_t      y          = 6;
uint16_t      prevX      = 0; // to keep from going out of bounds
uint16_t      prevY      = 0;

#define SPI_IF_BIT_RATE 100000
#define TR_BUFF_SIZE 100

static void BoardInit(void);
static void SysTickHandler(void)
{
    counter++;
    if (counter == 6)
    {
        counter = 0;
        done    = 1;
    }
}

static void GPIOA1IntHandler(void)
{ // pin connected to ir reciever handler
    unsigned long ulStatus;

    ulStatus = MAP_GPIOIntStatus(GPIOA1_BASE, true); // check uint16_terrupt status
    MAP_GPIOIntClear(GPIOA1_BASE, ulStatus);         // clear uint16_terrupts on GPIOA1
    MAP_GPIOIntDisable(GPIOA1_BASE, 0x1);

    if (!(GPIOPinRead(GPIOA1_BASE, 0x1)))
    {
        if (in_transmission)
        {                                                               // if reciving signal
            timeCount = Timer_IF_GetCount(g_ulBase, TIMER_A) % 1000000; // get count from timer after being started
            Timer_IF_Stop(g_ulBase, TIMER_A);
            signalBuff[index] = timeCount;
            index++; // increment index
            Timer_IF_Stop(g_ulBase, TIMER_A);
            Timer_IF_Start(g_ulBase, TIMER_A, 500);
        }
    }
    else
    {
        if (!in_transmission)
        {
            in_transmission = 1;
        }
        Timer_IF_Start(g_ulBase, TIMER_A, 500); // if on rising edge start timer
    }

    MAP_GPIOIntEnable(GPIOA1_BASE, 0x1);
}

void TimerBaseIntHandler(void)
{
    // Clear the timer uint16_terrupt.
    Timer_IF_InterruptClear(g_ulBase);
    index           = 0; // reset index
    in_transmission = 0; // no longer in transmission
    Timer_IF_Stop(g_ulBase, TIMER_A);
}
void TimerRefIntHandler(void)
{
    //
    // Clear the timer uint16_terrupt.
    //
    Timer_IF_InterruptClear(g_ulRefBase);
    //reset prev button so can do double tap
    prevButton = 12;
    Timer_IF_Stop(g_ulRefBase, TIMER_A);
}

uint16_t returnNumber(unsigned char signal[])
{
    /*
     This function takes in the signal buffer and compares it to our representation of different buttons
     returns the number button that was pressed and 10 for enter and 11 for delete
     */

    if (memcmp(zero, signal, BUTTON_STR_LEN) == 0)
    {
        return 0;
    }
    else if (memcmp(one, signal, BUTTON_STR_LEN) == 0)
    {
        return 1;
    }
    else if (memcmp(two, signal, BUTTON_STR_LEN) == 0)
    {
        return 2;
    }
    else if (memcmp(three, signal, BUTTON_STR_LEN) == 0)
    {
        return 3;
    }
    else if (memcmp(four, signal, BUTTON_STR_LEN) == 0)
    {
        return 4;
    }
    else if (memcmp(five, signal, BUTTON_STR_LEN) == 0)
    {
        return 5;
    }
    else if (memcmp(six, signal, BUTTON_STR_LEN) == 0)
    {
        return 6;
    }
    else if (memcmp(seven, signal, BUTTON_STR_LEN) == 0)
    {
        return 7;
    }
    else if (memcmp(eight, signal, BUTTON_STR_LEN) == 0)
    {
        return 8;
    }
    else if (memcmp(nine, signal, BUTTON_STR_LEN) == 0)
    {
        return 9;
    }
    else if (memcmp(enter, signal, BUTTON_STR_LEN) == 0)
    {
        return 10;
    }
    else if (memcmp(vol, signal, BUTTON_STR_LEN) == 0)
    {
        return 11;
    }
    return 99;
}

void display_char(uint16_t lower, uint16_t upper, uint16_t button)
{
    /*
     This function takes in a lower bound and upper bound for the button pressed for example the button 2 can either be an A, B, or C so lower bound is 0 and upper bound is 2
     
     */
    if (prevButton == button)
    {                             // if button is same as previous button
        u++;                      // increment through the list of possible buttons
        if (u > upper) u = lower; // if at max bound then return to first possible choice
        fillRect(prevX, y, x, y + 8, 0x0000);
        drawChar(prevX, y, alpha[u], 0xFFFF, 0xFFFF, 1);
    }

    else
    {                  // if not same button
        u     = lower; // begin at first possible choice
        prevX = x;
        drawChar(x, y, alpha[u], 0xFFFF, 0xFFFF, 1);
        x += 6;
    }
}

void UARTIntHandler()
{

    char          in;
    unsigned long status;
    // for recieving text

    status = UARTIntStatus(UARTA1_BASE, true);

    // Clear the asserted interrupts.
    UARTIntClear(UARTA1_BASE, status);

    while (UARTCharsAvail(UARTA1_BASE))
    {
        in = UARTCharGet(UARTA1_BASE);
        // draw the letter that is being recivied
        drawChar(6 + itr, 64 + itr2, in, 0xFFFF, 0xFFFF, 1);
        itr = itr + 6;
        // increment position
    }
    itr = 0;
    // once out reset x position but place y on new line
    itr2 = itr + 8;
}

void UARTSend(uint16_t size)
{
    //
    // Loop while there are more characters to send.
    //
    uint16_t k;
    // unsigned char one;
    for (k = 0; k < (size + 1); k++)
    {
        UARTCharPut(UARTA1_BASE, str[k]);
    }
    t    = -1; // reset size of string being sent
    flag = 0;  // reset flag as no longer sending
}
void append(uint16_t button)
{
    // if the flag to append string is true and previous button not same as current then its time to add the char to the string at position of last pressed button
    if (flag == 1)
    {
        if (prevButton != button)
        {
            str[t] = alpha[u];
            t++;
        }
    }
}
void checkBounds()
{
    // check if x is out of bounds if it is change y value and reset the x position
    if (x >= 126)
    {

        prevX = 6;
        prevY = y;
        x     = 6;
        y += 8;
    }
    // if y is out of bounds clear screen and begin at beginning
    if (y >= 126)
    {
        fillScreen(0x0000);
        x = 6;
        y = 6;
    }
}

//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void)
{
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);

    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}
//****************************************************************************
//
//! Main function
//!
//! \param none
//!
//!
//! \return None.
//
//****************************************************************************
int main()
{
    unsigned long UARTstatus;
    unsigned long ulStatus;
    BoardInit();
    PinMuxConfig();
    //InitTerm();
    //ClearTerm();

    SysTickIntRegister(SysTickHandler);
    SysTickPeriodSet(80000); /// 1kHz so 1ms
    SysTickIntEnable();

    // Register the interrupt handlers
    MAP_GPIOIntRegister(GPIOA1_BASE, GPIOA1IntHandler);
    // configure both edges on the pin connecte to ir reciever
    MAP_GPIOIntTypeSet(GPIOA1_BASE, 0x1, GPIO_BOTH_EDGES);

    ulStatus = MAP_GPIOIntStatus(GPIOA1_BASE, false);
    MAP_GPIOIntClear(GPIOA1_BASE, ulStatus);

    // Enable SW2 and SW3 interrupts
    MAP_GPIOIntEnable(GPIOA1_BASE, 0x1);

    // Base address for first timer
    g_ulBase    = TIMERA0_BASE;
    g_ulRefBase = TIMERA1_BASE;

    // Configuring the timers
    Timer_IF_Init(PRCM_TIMERA0, g_ulBase, TIMER_CFG_PERIODIC, TIMER_A, 0);
    Timer_IF_Init(PRCM_TIMERA1, g_ulRefBase, TIMER_CFG_PERIODIC, TIMER_A, 0);
    Timer_IF_IntSetup(g_ulBase, TIMER_A, TimerBaseIntHandler);
    Timer_IF_IntSetup(g_ulRefBase, TIMER_A, TimerRefIntHandler);
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(PRCM_GSPI);

    // Reset SPI
    //
    MAP_SPIReset(GSPI_BASE);

    //
    // Configure SPI interface
    MAP_SPIConfigSetExpClk(GSPI_BASE, MAP_PRCMPeripheralClockGet(PRCM_GSPI), SPI_IF_BIT_RATE, SPI_MODE_MASTER, SPI_SUB_MODE_0, (SPI_SW_CTRL_CS | SPI_4PIN_MODE | SPI_TURBO_OFF | SPI_CS_ACTIVEHIGH | SPI_WL_8));

    //
    // Enable SPI for communication
    MAP_SPIEnable(GSPI_BASE);
    //IntEnable(INT_UARTA1);
    MAP_UARTConfigSetExpClk(UARTA1_BASE, MAP_PRCMPeripheralClockGet(PRCM_UARTA1), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    UARTEnable(UARTA1_BASE);
    UARTIntRegister(UARTA1_BASE, UARTIntHandler);
    UARTstatus = UARTIntStatus(UARTA1_BASE, false);
    UARTIntClear(UARTA1_BASE, UARTstatus);
    UARTIntEnable(UARTA1_BASE, UART_INT_RX | UART_INT_RT);

    //GPIOPinWrite(GPIOA1_BASE, 0x8, 0); //CS LOW

    index = 0;

    unsigned char decodeSignal[SIGNAL_BUFFER_LEN];
    uint16_t      i;
    uint16_t      j;
    uint16_t      numPressed = 99;
    Adafruit_Init();

    fillScreen(0x0000);
    while (1)
    {
        if (in_transmission && index >= 34)
        {
            //           MAP_GPIOIntDisable(GPIOA1_BASE, 0x1);
            j = 0;
            for (i = 0; i < 34; i++)
            {
                if (i > 1)
                {
                    //                    if(signalBuff[i] > 4255090000) {
                    //process the signal
                    if (signalBuff[i] > 90000)
                    {

                        decodeSignal[j] = '1';
                    }
                    else
                    {
                        decodeSignal[j] = '0';
                    }
                    j++;
                }
            }

            numPressed = returnNumber(decodeSignal);

            checkBounds(); // checking if out of bounds
            if (numPressed != 99)
            {
                // while number is valid, we are appending string if appropriate and displaying the characters
                if (numPressed == 0)
                {
                    // flag = 1;
                    x += 6;
                    str[t] = ' ';
                    t++;
                    prevButton = 0;
                }
                else if (numPressed == 1)
                {
                    prevButton = 1;
                }
                else if (numPressed == 2)
                {
                    flag = 1;
                    append(2);
                    display_char(0, 2, 2);
                    prevButton = 2;
                }
                else if (numPressed == 3)
                {
                    flag = 1;
                    append(3);
                    display_char(3, 5, 3);
                    prevButton = 3;
                }
                else if (numPressed == 4)
                {
                    flag = 1;
                    append(4);
                    display_char(6, 8, 4);
                    prevButton = 4;
                }
                else if (numPressed == 5)
                {
                    flag = 1;
                    append(5);
                    display_char(9, 11, 5);
                    prevButton = 5;
                }
                else if (numPressed == 6)
                {
                    flag = 1;
                    append(6);
                    display_char(12, 14, 6);
                    prevButton = 6;
                }
                else if (numPressed == 7)
                {
                    flag = 1;

                    append(7);
                    display_char(15, 18, 7);
                    prevButton = 7;
                }
                else if (numPressed == 8)
                {
                    flag = 1;

                    append(8);
                    display_char(19, 21, 8);
                    prevButton = 8;
                }
                else if (numPressed == 9)
                {
                    flag = 1;

                    append(9);
                    display_char(22, 25, 9);
                    prevButton = 9;
                }
                else if (numPressed == 10)
                {
                    if (prevButton != 12)
                    {

                        UARTSend(t);
                        x = 6;
                        y = 6;
                        fillRect(0, 64, 0, 64, 0x0000);
                    }
                    //fillScreen(0x0000);
                }
                if (numPressed == 11)
                {
                    t--;
                    // if time to delete we paint over the char and decrement the string count in addition to readjusting the x and y coordinates
                    if (x == 6)
                    {

                        if (y != 6)
                        {
                            y     = y - 8;
                            prevY = prevY - 8;
                            x     = 120;
                            prevX = 114;

                            fillRect(120, y, 126, y + 8, 0x0000);
                        }
                    }

                    else
                    {
                        fillRect(prevX, y, x, y + 8, 0x0000);
                        x     = x - 6;
                        prevX = prevX - 6;
                    }

                    prevButton = 11;
                }
            }

            //flag = 1;

            /*
            int m = 0;
            Report("%d\n\r", numPressed);
            for(m = 0; m < 32; m++){
                Report("%c", decodeSignal[m]);
            }
            Report("\n\r");
             */
            index           = 0;
            in_transmission = 0;
            MAP_GPIOIntEnable(GPIOA1_BASE, 0x1);
        }
        if (done)
        {
            SystickDisable();
            prevButton = 12;
        }
        if (!in_transmission)
        {
            SysTickEnable();
            index = 0;
        }
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
