### PIC16F-CapSense Description:
A port of the [Arduino ADCtouch library](https://playground.arduino.cc/Code/ADCTouch/) for PIC16F µCs / MCUs.

It uses the same charge/discharge of the internal ADC capacitor idea (10pF Chold) and also only requires 1 analog input.

### Files:
"PIC16F CapSense.X\\"-- folder containing project files for MPLAB IDE V5.30 on the PIC16F1937 "CAP TOUCH EVAL BOARD"
* ..\main.h       --  XC8 declarations, includes, and 16Mhz intosc clock
* ..\main.c       --  CapSense function and  UART serial output main loop
* ..\serial.h     --  UART serial header file (function declarations)
* ..\serial.c     --  UART serial port configuration (init) and R/W functions

"picADC graph.vi" -- LabVIEW 2014 (or newer) graphical utility to plot ADC sums from UART (serial com port)


### Background And Theory:

[Arduino's Capacitive Sensor Library](https://playground.arduino.cc/Main/CapacitiveSensor/) is a very useful library for all sorts of hobbyist projects and is similiar to [Microchip's mTouch technology](https://www.microchip.com/stellent/groups/SiteComm_sg/documents/DeviceDoc/en542979.pdf) that uses counting of clock pulses to determine capacitance changes.  Similarly, mTouch requires dedicated circuitry, components and two analog inputs; [the mTouch software library API](https://microchipdeveloper.com/touch:mcc-api) also requires several complicated steps to get working properly.

Alternatively, [Microchip's AN1298 (CVD application note)](http://ww1.microchip.com/downloads/en/appnotes/01298a.pdf) provides a method whereby the onboard ADCs of PIC microcontrollers can be used for capacitive sensing using charge/discharge cycles of its internal ADC hold capacitor (typically 10pF).  Two analog channels are held in parallel and the voltage divider circuit can be sampled by the input ADC channel for external capacitive changes.

This requires two ADC channels to be used concurrently; albeit, their functionality can then be reversed subsequently.  Thus, the code can easily become confusing.

![AN1298](https://www.walduk.at/wp-content/uploads/2018/08/Screenshot_20180911_101414-1024x333.png)


Meanwhile, [Martin Pittermann's ADCTtouch library for AVR](https://github.com/martin2250/ADCTouch) only requires a single ADC channel and makes use of the AVR ADC's internal hold capacitor to charge/discharge [the ADC's sample and hold voltage](https://en.wikipedia.org/wiki/Sample_and_hold) and then take measurements of the change provided by an externally coupled capacitance.

Similarly, this CapSense example also only requires 1 ADC input channel and works in a similar fashion by discharging/charging up the ADC hold capacitor in reverse to the ADCtouch library.  The reason being that [PIC ADC circuitry is designed](https://electrosome.com/adc-pic-microcontroller-mplab-xc8/) differently than the AVR's ADC for the reference voltages.

The AVR ADCtouch library initially charges up the hold capacitor of the input pin in a digital output state, and then [discharges the capacitor by grounding the ADMUX bits to 0V](http://maxembedded.com/2011/06/the-adc-of-the-avr/).  ADC measurements are subsequently taken to see how much capacitance change has been made.

However, as shown for the example PIC ADC diagram below, the Vref pins of the PIC ADC can only be held to Vdd/Vss or external inputs.  Therefore, the PIC ADC channel should initially be made digital output 0V (GND), and then switched to analog internally referenced to Vdd/Vss and ADC measurements made to detect any capacitance changes for the Chold charge up times.

![PIC ADC MUX](https://electrosome.com/wp-content/uploads/2013/07/ADC-Module-Block-Diagram-PIC16F877A.jpg)

The advantage of this process is no baseline offset calibration value need to be taken, since the ADC values will be mostly ~0 due to fast switching of the onboard MUX doesn't allow Chold capacitance time to charge up.   But with an externally coupled capacitance, the total RC charge time will be faster and thus higher values will be detected.

The disadvantage of this process is that sensitivity is low and noise is higher if the external capacitance source is not large enough.  On tests with the LabVIEW "picADC graph.vi" utility, only a single wire input was used and only direct contact provided noticeable change to the ADC sums value output.


### Explanation Of Code:

```
unsigned int CapSense(void){
    unsigned int tempADC = 0;
    /*----- iterations should be <64 to avoid stack overflow for tempADC -----*/
    int iterations = 16;
    for (int i=0; i<iterations; i++) {
        /*----- enable GPIO output low to discharge VChold -----*/
        TRISB5  = 0;
        ANSELB  = 0;
        RB5     = 0;
        /*----- enable ADC input to charge up VChold -----*/
        TRISB5  = 1;             // RB5 to input for ADC
        ANSELB  = 0b00100000;    // select AN13
        ADCON0  = 0b00110101;    // enable AN13 and ADC
        ADCON1  = 0b01010000;    // Fosc÷16, Vref- to Vss, Vref+ to AVdd
        /*----- start ADC conversion and sum results -----*/
        ADGO    = 1;
        while(ADGO);
        tempADC += (ADRESH << 8) + ADRESL;
    }
    return tempADC;
}
```

The core of the function is within the for-loop.

* In this example, AN13 (RB5) of the PIC16F1937 is put in digital output state 0 (GND) to discharge Chold.


* Then AN13 is set to ADC input and the appropriate ANSEL and ADCON values are set for the corresponding registers.  Vref- is set to Vss (0V) and Vref+ is set to AVdd in order to measure the charging voltage.  For instoc 16MHz, the datasheet recommends ADC sampling rate of Fosc ÷ 16.

* ADC conversion is then performed to measure the VChold voltage.  The result is then summed into the return value.

A slight ```__delay_us_()``` between the steps might help to fully discharge/charge the Chold capacitor, but I've not personally noticed any significant changes and believe the built-in registers handle the digital states.

Likewise, I've also not found the sampling frequency to have much effect on results.


### Ideas For Improvements:

* Perhaps for applications where initial ADC noise can be a problem, the ADC can be put into a digital output ground 0 after the for-loop.

* The number of iterations of the for-loop can help to "increase sensitivity" but I think hardware mostly determins the capacitance change values.

* Generalization of C function for any ADC channel is possible, but dependent on PIC16F variant.   Might not be worth the time.

* I've tested on a few PIC16F variants and PIC ADC are the same so functionality should be consistent.   However, PIC18F and PIC24F probably requires some register changes to affect the same functionality.

* [ADC input protection methods](https://www.analog.com/en/technical-articles/protecting-adc-inputs.html) (clamping, op-amp buffering, etc.) are recommended against ESD and over current.
