
#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/can.h"
#include "drivers/buttons.h"

#define LED_RED GPIO_PIN_1
#define LED_BLUE GPIO_PIN_2
#define LED_GREEN GPIO_PIN_3

static uint8_t g_ui8ButtonStates = ALL_BUTTONS;
uint8_t ButtonsPoll(uint8_t *, uint8_t *);
uint8_t led_switch;

/*
  Name of file: main_button_while.c
  Author: Helmuth Brunner
  Version: 1.0
  Describtion: A program for the tiva-launchpad. If you press one of the buttons the rgb-LED will change the color.
*/


int main() {

	uint8_t ui8Buttons;
	uint8_t ui8ButtonsChanged;

	ROM_SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,LED_RED | LED_BLUE | LED_GREEN);
	
	ROM_SysCtlPeripheralEnable(BUTTONS_GPIO_PERIPH);


	HWREG(BUTTONS_GPIO_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(BUTTONS_GPIO_BASE + GPIO_O_CR) |= 0x01;
	HWREG(BUTTONS_GPIO_BASE + GPIO_O_LOCK) = 0;

	ROM_GPIODirModeSet(BUTTONS_GPIO_BASE, ALL_BUTTONS, GPIO_DIR_MODE_IN);
	ROM_GPIOPadConfigSet(BUTTONS_GPIO_BASE,ALL_BUTTONS, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	g_ui8ButtonStates = ROM_GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS);

	for(;;) {
		/*
		ui8Buttons = ButtonsPoll(&ui8ButtonsChanged, 0);

		if(BUTTON_PRESSED(LEFT_BUTTON, ui8Buttons, ui8ButtonsChanged))
		{
			ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, LED_RED);
		}//Button leftside, if button leftside is pressed the LED will be flash red

		else if(BUTTON_PRESSED(RIGHT_BUTTON, ui8Buttons, ui8ButtonsChanged)) 
		{
			ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, LED_GREEN);
		}//Button rightside, if button leftside is pressed the LED will be flash green

		else if(BUTTON_RELEASED(LEFT_BUTTON|RIGHT_BUTTON, ui8Buttons, ui8ButtonsChanged))
		{
			ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, 0);
		}//If the buttons will be release the LED turn off.
		
		*/
		
		//----Wenn beide Buttons gedrueckt werden leuchten auch ---
		ui8Buttons = (uint8_t)~ROM_GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS);
		led_switch = 0;
		if(LEFT_BUTTON & ui8Buttons) led_switch |= LED_RED;
		if(RIGHT_BUTTON & ui8Buttons) led_switch |= LED_GREEN;
		ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, led_switch);

	}

	return 0;
}

//Method ButtonsPoll() from buttons.h

uint8_t
ButtonsPoll(uint8_t *pui8Delta, uint8_t *pui8RawState) {
	uint32_t ui32Delta;
	uint32_t ui32Data;
  	static uint8_t ui8SwitchClockA = 0;
    	static uint8_t ui8SwitchClockB = 0;

    //
    // Read the raw state of the push buttons.  Save the raw state
    // (inverting the bit sense) if the caller supplied storage for the
    // raw value.
    //
    	ui32Data = (ROM_GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS));
    	if(pui8RawState)
    	{
        	*pui8RawState = (uint8_t)~ui32Data;
    	}

    //
    // Determine the switches that are at a different state than the debounced
    // state.
    //
    	ui32Delta = ui32Data ^ g_ui8ButtonStates;

    //
    // Increment the clocks by one.
    //
    	ui8SwitchClockA ^= ui8SwitchClockB;
    	ui8SwitchClockB = ~ui8SwitchClockB; 

    //
    // Reset the clocks corresponding to switches that have not changed state.
    //
    	ui8SwitchClockA &= ui32Delta;
    	ui8SwitchClockB &= ui32Delta;

    //
    // Get the new debounced switch state.
    //
    	g_ui8ButtonStates &= ui8SwitchClockA | ui8SwitchClockB;
    	g_ui8ButtonStates |= (~(ui8SwitchClockA | ui8SwitchClockB)) & ui32Data;

    //
    // Determine the switches that just changed debounced state.
    //
    	ui32Delta ^= (ui8SwitchClockA | ui8SwitchClockB);

    //
    // Store the bit mask for the buttons that have changed for return to
    // caller.
    //
   	if(pui8Delta)
    	{
        	*pui8Delta = (uint8_t)ui32Delta;
    	}

    //
    // Return the debounced buttons states to the caller.  Invert the bit
    // sense so that a '1' indicates the button is pressed, which is a
    // sensible way to interpret the return value.
    //
    	return(~g_ui8ButtonStates);

}
