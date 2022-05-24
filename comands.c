/*
 * comands.c
 * Terminal command functions
 *
 *  Created on: Aug 25, 2017
 *      Author: jmf6
 */

#include "terminal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "hal.h"
#include "ptt.h"

#include "USBprintf.h"


#include "USB_config/descriptors.h"
#include "USB_API/USB_CDC_API/UsbCdc.h"

int ptt_Cmd(int argc,char *argv[]){
    float delay;
    char *eptr;
    const char *state_str;
    long tmp;
    int num=PTT_PIN1;
    int option_arg=1;
    enum PTT_SIGNAL ptt_sig;
    enum PTT_STATE state;
    int i;
    if(argc==0){
        //print number of PTTs for matlab usage
        printf("%i PTT outputs:\r\n",NUM_PTT);

        for(i=0;i<NUM_PTT;i++){
            //print out status for all
            printf("PTT%i status : %s\r\n",i+1,ptt_get(i)?"on":"off");
        }
    }else{
        //attempt to parse the first argument as a number
        tmp=strtol(argv[option_arg],&eptr,10);
        //check if a number was found
        if(eptr!=argv[option_arg] && *eptr=='\0'){
            if(tmp>NUM_PTT || tmp<=PTT_PIN1){
                printf("Error : invalid PTT number\r\n");
                return 5;
            }
            //set PTT number
            num=tmp-1;
            //look at the next argument
            option_arg++;
            //check if there are any other arguments
            if(option_arg>argc){
                //print out status for PTT
                printf("PTT%i status : %s\r\n",num+1,ptt_get(num)?"on":"off");
                //done, return
                return 0;
            }
        }
        if(!strcmp("on",argv[option_arg])){
            //check number of arguments
            if((argc-option_arg)>0){
                printf("Error : too many arguments\r\n");
                return 1;
            }
            // Turn on push to talk
            ptt_set(num,PTT_ON);
        }else if(!strcmp("off",argv[option_arg])){
            //check number of arguments
            if((argc-option_arg)>0){
                printf("Error : too many arguments\r\n");
                return 1;
            }
            // Turn off push to talk
            ptt_set(num,PTT_OFF);
        }else if(!strcmp("delay",argv[option_arg]) || !strcmp("Sdelay",argv[option_arg])){
            //check if we are using PTT signal
            ptt_sig=argv[option_arg][0]=='S'?PTT_USE_SIGNAL:PTT_NO_SIGNAL;
            //check number of arguments
            if((argc-option_arg)>1){
                printf("Error : too many arguments\r\n");
                return 1;
            }
            //parse delay value
            delay=strtod(argv[option_arg+1],&eptr);
            //check that some chars were parsed
            if(argv[option_arg+1]==eptr){
                printf("Error : could not parse delay value \"%s\"\r\n",argv[option_arg+1]);
                return 3;
            }
            //check that delay is greater than or equal to zero
            if(delay<0){
                printf("Error : delay of %f is not valid. Valid delays must be greater than or equal to zero\r\n",delay);
                return 4;
            }
            //start delay count down
            delay=ptt_on_delay(num,delay,ptt_sig);
            //print out actual delay
            printf("PTT in %f sec\r\n",delay);
        }else if(!strcmp("state",argv[option_arg])){
            //check number of arguments
            if((argc-option_arg)>0){
                printf("Error : too many arguments\r\n");
                return 5;
            }
            //get ptt state
            state=ptt_get_state();
            switch(state)
            {
            case PTT_STATE_IDLE:
                state_str="Idle";
                break;
            case PTT_STATE_SIG_WAIT:
                state_str="Signal Wait";
                break;
            case PTT_STATE_DELAY:
                state_str="Delay";
                break;
            default:
                state_str="Internal Error";
                break;
            }
            printf("PTT state : \"%s\"\r\n",state_str);
        }else{
            //error
            printf("Error : Unknown state \"%s\"\r\n",argv[option_arg]);
            return 2;
        }
    }
    return 0;
}

int devtype_Cmd(int argc,char *argv[]){
    // Print device type, use a fixed version number for now
    printf("MCV radio interface : "VERSION_STR"\r\n");
    return 0;
}

const struct{
    int port,pin;
}leds[2]        ={{GPIO_PORT_P1,GPIO_PIN0},{GPIO_PORT_P4,GPIO_PIN7}},
shadow_leds[2]  ={{GPIO_PORT_P3,GPIO_PIN5},{GPIO_PORT_P3,GPIO_PIN6}};

int LED_Cmd(int argc,char *argv[]){
    int LED_num,LED_idx;
    if(argc<2){
        //too few arguments
        printf("Too few arguments\r\n");
        return 1;
    }
    //get LED number
    LED_num=atoi(argv[1]);
    if(LED_num<=0 || LED_num>2){
        //invalid LED number
        printf("Invalid LED number \"%s\"\r\n",argv[1]);
        return 1;
    }
    //subtract 1 to get index
    LED_idx=LED_num-1;
    //parse state
    if(!strcmp("on",argv[2])){
        GPIO_setOutputHighOnPin(       leds[LED_idx].port,       leds[LED_idx].pin);
        GPIO_setOutputHighOnPin(shadow_leds[LED_idx].port,shadow_leds[LED_idx].pin);
    }else if(!strcmp("off",argv[2])){
        GPIO_setOutputLowOnPin(       leds[LED_idx].port,       leds[LED_idx].pin);
        GPIO_setOutputLowOnPin(shadow_leds[LED_idx].port,shadow_leds[LED_idx].pin);
    }else{
        //invalid state
        printf("Error : invalid state \"%s\"\r\n",argv[2]);
        return 2;
    }
    return 0;
}

int closeout_Cmd(int argc,char *argv[]){
    int i;
    //loop through all PTT's
    for(i=0;i<NUM_PTT;i++){
        //turn off ptt
        ptt_set(i,PTT_OFF);
    }
    //loop through all LED's
    for(i=0;i<sizeof(leds)/sizeof(leds[0]);i++){
        //turn off LED
        GPIO_setOutputLowOnPin(       leds[i].port,       leds[i].pin);
        GPIO_setOutputLowOnPin(shadow_leds[i].port,shadow_leds[i].pin);
    }
    return 0;
}

float ADC_temp_scl,ADC_temp_offset;

/*
void ADCinit(void){
    unsigned int cal_30c,cal_85c;
    //disable ADC
    ADC12CTL0&=~ADC12ENC;
    //setup REF for always on 1.5V
    REFCTL0=REFMSTR|REFOUT|REFON;
    //setup ADC
    //NOTE : 8 clocks can be used for sample and hold time if only thermistor is used (5k ohm eq series resistance)
    ADC12CTL0=ADC12SHT1_6|ADC12SHT0_6|ADC12MSC|ADC12ON;
    ADC12CTL1=ADC12CSTARTADD_0|ADC12SHS_0|ADC12SHP|ADC12DIV_1|ADC12SSEL_0|ADC12CONSEQ_1;
    //disable output buffers on analog ports
    P6SEL|=BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6;
    P7SEL|=BIT0;

    //read temperature reference points
    cal_30c=*((unsigned int *)0x1A1A);
    cal_85c=*((unsigned int *)0x1A1C);
    //generate calibration factors
    ADC_temp_scl=(85 - 30)/(float)(cal_85c - cal_30c);
    ADC_temp_offset=30-((float)cal_30c)*ADC_temp_scl;
}

int analog_Cmd(int argc,char *argv[]){
    unsigned short raw[16];
    float scaled[16];
    int ch[16];
    int i;
    volatile unsigned char *memctl=&ADC12MCTL0;
    volatile unsigned int *res=&ADC12MEM0;

    //check the number of arguments
    if(argc<1){
        printf("Error : too few arguments\r\n");
        return 1;
    }
    if(argc>=16){
        printf("Error : too many arguments\r\n");
        return 2;
    }
    //disable ADC
    ADC12CTL0&=~ADC12ENC;
    //parse arguments
    for(i=0;i<argc;i++){
        if(argv[i+1][0]=='A'){
            //get channel number
            ch[i]=atoi(argv[i+1]+1);
            //check for valid chanel numbers
            if(ch[i]<0 || ch[i]>15){
                printf("Error : invalid channel number \"%s\" while parseing channel \"%s\"\r\n",argv[i+1]+1,argv[i+1]);
                return 3;
            }
            //set up memctl
            memctl[i]=ADC12SREF_0|ch[i];
        }else if(!strcmp(argv[i+1],"Tint")){
            ch[i]=-1;
            //set up memctl for temp diode using internal ref
            memctl[i]=ADC12SREF_1|ADC12INCH_10;
        }
    }
    //set end of sequence
    memctl[i]|=ADC12EOS;
    //enable ADC and start conversion
    ADC12CTL0|=ADC12ENC|ADC12SC;
    //wait while conversion is in progress
    while(ADC12CTL1&ADC12BUSY);
    //disable ADC
    ADC12CTL0&=~ADC12ENC;
    //get results
    for(i=0;i<argc;i++){
        raw[i]=res[i];
        if(ch[i]==-1){
            scaled[i]=ADC_temp_scl*raw[i]+ADC_temp_offset;
            printf("Tint = %.2f C\r\n",scaled[i]);
        }else{
            scaled[i]=3.3/((float)0xFFF)*raw[i];
            printf("A%i = %.4f V\r\n",ch[i],scaled[i]);
        }
    }
    return 0;
}


int temp_Cmd(int argc,char *argv[]){
    unsigned short raw[2];
    float Tint;
    //disable ADC
    ADC12CTL0&=~ADC12ENC;
    //set up memctl for temp diode using internal ref
    ADC12MCTL0=ADC12SREF_1|ADC12INCH_10;
    //read external thermistor voltage
    ADC12MCTL1=ADC12SREF_0|ADC12INCH_5|ADC12EOS;
    //enable ADC and start conversion
    ADC12CTL0|=ADC12ENC|ADC12SC;
    //wait while conversion is in progress
    while(ADC12CTL1&ADC12BUSY);
    //disable ADC
    ADC12CTL0&=~ADC12ENC;
    //get results
    raw[0]=ADC12MEM0;
    raw[1]=ADC12MEM1;
    //calculate internal temperature
    Tint=ADC_temp_scl*raw[0]+ADC_temp_offset;
    //print results
    printf("int = %f C\r\next = %u\r\n",Tint,raw[1]);
    return 0;
}
*/

int id_Cmd(int argc,char *argv[])
{
    uint8_t len;
    struct s_TLV_Die_Record *record;
    //get pointer to die record
    TLV_getInfo(TLV_TAG_DIERECORD,0,&len,(uint16_t**)&record);
    //print out die record
    printf("%08lX%04X%04X\r\n",record->wafer_id,record->die_x_position,record->die_y_position);
    return 0;
}

int bsl_Cmd(int argc,char *argv[])
{
    const char *str="The BSL command is used for firmware upgrade, commandline will be unavailable. Good Bye!\r\n";

    USBCDC_sendData((uint8_t*)str,strlen(str),CDC0_INTFNUM);
    //Disable interrups for BSL entry
    __disable_interrupt();
    //call BSL code
    ((void (*)())0x1000)();
    //This statement shouldn't be reached
    return 0;
}

//table of commands with help
const CMD_SPEC cmd_tbl[]={{"help"," [command]\r\n\t""get a list of commands or help on a spesific command.",helpCmd},
                          {"ptt"," state\r\n\t""change the push to talk state of the radio",ptt_Cmd},
                          {"devtype","\r\n\t""get the device type string",devtype_Cmd},
                          {"LED","number state\r\n\t""set the LED status",LED_Cmd},
                          {"closeout","\r\n\tTurn off ptt and all LED's",closeout_Cmd},
                          //{"analog","ch1 [ch2] ... [chn]\r\n\tRead analog values",analog_Cmd},
                          //{"temp","\r\n\tRead temperature sensors",temp_Cmd},
                          {"id","\r\n\tPrint a unique ID (in hex) for this processor",id_Cmd},
                          {"bsl","\r\n\tenter boot strap loader code (for loading firmware)",bsl_Cmd},
                          //end of list
                          {NULL,NULL,NULL}};
