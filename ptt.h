/*
 * ptt.h
 *
 *  Created on: Sep 7, 2017
 *      Author: jmf6
 */

#ifndef PTT_H_
#define PTT_H_

enum PTT_ACTION {PTT_ON,PTT_OFF,PTT_TOGGLE};

enum PTT_OUTPUTS {PTT_PIN1,PTT_PIN2,PTT_PIN3,NUM_PTT}; // JODELL #1 add PTT_PIN3 so init. entries are not ignored

enum PTT_STATUS {PTT_INVALID=-1,PTT_HIGH=1,PTT_LOW=0};

enum PTT_SIGNAL {PTT_NO_SIGNAL=0,PTT_USE_SIGNAL=1};

enum PTT_STATE {PTT_STATE_IDLE=0,PTT_STATE_SIG_WAIT,PTT_STATE_DELAY};

void PTT_init(void);

void ptt_set(int num,int action);

float ptt_on_delay(int num,float delay,enum PTT_SIGNAL signal);

enum PTT_STATE ptt_get_state(void);

enum PTT_STATUS ptt_get(int num);

#define OFFSET_IN   0x00
#define OFFSET_OUT  0x02
#define OFFSET_DIR  0x04
#define OFFSET_REN  0x06
#define OFFSET_DS   0x08
#define OFFSET_SEL  0x0A



#endif /* PTT_H_ */
