/*
 * ptt.c
 *
 *  Created on: Sep 7, 2017
 *      Author: jmf6
 */

#include "ptt.h"
#include <msp430.h>
#include <math.h>

//base address of ports to use for PTT
volatile unsigned char *(ptt_port_base[NUM_PTT])={&P8IN,&P8IN,&P1IN}; //JODELL #2 add !PTT port def

//pin numbers used for PTT
int ptt_pin[NUM_PTT]={2,1,0};//JODELL #3 add !PTT pin def

enum PTT_STATE PTT_state=PTT_STATE_IDLE;

void PTT_init(void){
    int i;

    //set state to idle
    PTT_state=PTT_STATE_IDLE;

    //setup all PTT pins

    for(i=0;i<NUM_PTT;i++){
        //set all pins low
        ptt_port_base[i][OFFSET_OUT]&=~(1<<ptt_pin[i]);

        //set pin as output
        ptt_port_base[i][OFFSET_DIR]|= (1<<ptt_pin[i]);
    }

    ptt_set(2, PTT_OFF); //JODELL #4 command !PTT sig starting possition

    //Setup ptt tone pin

    //select timer function
    P2SEL|=BIT5;
    //set pin to output
    P2DIR|=BIT5;

    //setup clip signal pin

    //set as input
    P6DIR&=~BIT0;
    //disable digital input logic
    CBCTL3|=CBPD0;

    //select input from comparitor
    CBCTL0=CBIPEN|CBIPSEL_0;
    //turn on coparitor, high speed mode
    CBCTL1=CBON|CBPWRMD_0;
    //use 2.5V reference
    //'1' threshold is 0.4688 V
    //'0' threshold is 0.6250 V
    CBCTL2|=CBREFACC|CBREFL_3|CBREF1_6|CBRS_2|CBRSEL|CBREF0_8;

    //Setup timer for PTT signal capture and delay timing
    //set input divider expansion to /4
    TA1EX0=TAIDEX_3;
    //setup TA1 to run in continuous mode
    //set input divider to /8 for a total of /32
    TA1CTL=TASSEL_1|ID_3|MC_2|TACLR;

}

void ptt_tone_stop(void){
    //stop and timer
    TA2CTL=TASSEL_1|ID_0;
    //set output low
    TA2CCTL2=0;
}

//period of PTT tone
#define PTT_TONE_PERIOD      80

void ptt_tone_start(void){
    //stop and setup timer
    TA2CTL=TASSEL_1|ID_0;
    //set period
    TA2CCR0=PTT_TONE_PERIOD;
    //set high time
    TA2CCR2=PTT_TONE_PERIOD/2;
    //setup output mode of timer
    TA2CCTL2=OUTMOD_7;
    //start and clear timer
    TA2CTL|=MC_1|TACLR;
}


void ptt_set(int num,int action){
    //check if the PTT number is valid
    if(num>=NUM_PTT && num<PTT_PIN1){
        //invalid pin number
        return;
    }
    //disable delay timer interrupt
    TA1CCTL0=0;
    //disable signal timer interrupt
    TA1CCTL1=0;
    //set state to IDLE
    PTT_state=PTT_STATE_IDLE;
    //check action
    switch(action){
        case PTT_ON:
            //set pin high
            ptt_port_base[num][OFFSET_OUT]|= (1<<ptt_pin[num]);
            ptt_port_base[2][OFFSET_OUT]&= ~(1<<ptt_pin[2]);// JODELL #5a glue !ptt to fall low given PTT_ON state

            //start ptt tone
            ptt_tone_start();
            break;
        case PTT_OFF:
            //set ptt pin low
            ptt_port_base[num][OFFSET_OUT]&=~(1<<ptt_pin[num]);
            ptt_port_base[2][OFFSET_OUT]|=(1<<ptt_pin[2]);// JODELL #5b glue !ptt to go high given PTT_OFF state
            //stop ptt tone
            ptt_tone_stop();
            break;
        case PTT_TOGGLE:
            //toggle PTT pin
            ptt_port_base[num][OFFSET_OUT]^= (1<<ptt_pin[num]);
            //check ptt status
            if(ptt_port_base[num][OFFSET_OUT]&(1<<ptt_pin[num])){
                //start ptt tone
                ptt_tone_start();
            }else{
                //stop ptt tone
                ptt_tone_stop();
            }
            break;
    }
}

//PTT on delay
static unsigned short ptt_delay=0;

//PTT delay pin
static int ptt_delay_pin=-1;

float ptt_on_delay(int num,float delay,enum PTT_SIGNAL signal){
    //check if the PTT number is valid
    if(num>=NUM_PTT && num<PTT_PIN1){
        //invalid pin number
        return NAN;
    }
    //set pin number
    ptt_delay_pin=num;
    //calculate delay in timer clocks
    delay=delay*(32768/32);
    //limit to timer maximum
    if(delay>65535){
        //set to maximum
        delay=65535;
    }
    //with signal we can go all the way down to zero delay
    if(signal==PTT_NO_SIGNAL)
    {
        //limit minimum to 1
        if(delay<1){
            //set to one
            delay=1;
        }
    }
    else
    {
        //limit minimum to 0
        if(delay<0){
            //set to zero
            delay=0;
        }
    }
    //set ptt delay
    ptt_delay=delay;

    //check if we are expecting a PTT signal
    if(signal==PTT_NO_SIGNAL)
    {
        //not expecting a signal, start timer now

        //set state to waiting for timeout
        PTT_state=PTT_STATE_DELAY;

        //setup TA1CCR0 to capture timer value
        TA1CCTL0=CM_3|CCIS_2|SCS|CAP|CCIE;
        //capture current timer value
        TA1CCTL0^=CCIS0;
    }
    else
    {
        //Wait for signal before starting PTT delay timer

        //set state to waiting for signal
        PTT_state=PTT_STATE_SIG_WAIT;

        //setup TA1CCR1 to capture PTT signal start
        //select input signal CCI1B (CBOUT)
        TA1CCTL1=CM_1|CCIS_1|SCS|CAP|CCIE;
    }
    //return actual delay
    return ptt_delay/(float)(32768/32);
}

//get PTT state
enum PTT_STATE ptt_get_state(void){
    //return state
    return PTT_state;
}

//get ptt status
enum PTT_STATUS ptt_get(int num){
    //check if the PTT number is valid
    if(num>=NUM_PTT && num<PTT_PIN1){
        //invalid pin number
        return PTT_INVALID;
    }
    //get status from ptt pin
    return (ptt_port_base[num][OFFSET_OUT]&(1<<ptt_pin[num]))?PTT_HIGH:PTT_LOW;
}


// ============ TA1 CCR0 ISR ============
// used for PTT delay
#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A0_VECTOR
__interrupt void PTT_timer_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(TIMER1_A0_VECTOR))) PTT_timer_ISR (void)
#else
#error Compiler not found!
#endif
{
    //check if timer is in capture mode
    if(TA1CCTL0&=CAP){
        //add delay to capture time
        TA1CCR0+=ptt_delay;
        //set to compare mode and enable interrupts
        TA1CCTL0=CCIE;
    }else{
        //disable timer interrupt
        TA1CCTL0=0;
        //turn on ptt, will set PTT state
        ptt_set(ptt_delay_pin,PTT_ON);
    }
}

// ============== TA1 ISR ==============
#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A1_VECTOR
__interrupt void PTT_sig_timer_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(TIMER1_A1_VECTOR))) PTT_timer_ISR (void)
#else
#error Compiler not found!
#endif
{
    switch(TA1IV)
    {
    case TA1IV_TACCR1:
        //check for zero delay
        if(ptt_delay==0)
        {
            //turn on PTT pin now
            ptt_set(ptt_delay_pin,PTT_ON);
        }
        else
        {
            //set state to waiting for timeout
            PTT_state=PTT_STATE_DELAY;
            //add delay to captured time and set CCR0
            TA1CCR0=TA1CCR1+ptt_delay;
            //set to compare mode and enable interrupts
            TA1CCTL0=CCIE;
        }
        //disable further capture interrupts
        TA1CCTL1&=~CCIE;
    break;
    }
}


