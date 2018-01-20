//This Arduino UNO Sketch requires the Seeed CAN-BUS Shield Libraries
//https://github.com/yexiaobo-seeedstudio/CAN_BUS_Shield
#include <SPI.h>
#include "mcp_can.h"

INT32U canId = 0x000;

typedef unsigned char byte_t;
typedef unsigned char uint8_t;
typedef char int8_t;

#define IS_A_FLOW_CONTROL_FRAME(X) (X[0] & 0xF0 == 3)
#define FLOW_CONTROL_FRAME_IS_CONTINUE(X) (X[0] & 0x0F == 0)
#define MODE9_PID2_VIN 0x02

#define FRAME_TYPE_REQUEST 0x00
#define FRAME_TYPE_SINGLE 0x00
#define FRAME_TYPE_FIRST 0x01
#define FRAME_TYPE_CONSECUTIVE 0x02
#define FRAME_TYPE_FLOW_CONTROL 0x03

#define ANSWER_TO_MODE9 0x40 + 0x09

typedef struct state{
    uint8_t unansweredRequestNumber = 0;
    request_t unansweredrRequests[10];
    request_t answerInProgress
}state_t;

typedef enum states{
    STATE_WAIT_FLOW_CONTROL_FRAME = 0,
    STATE_SEND_NEXT_CONSECUTIVE_FRAME,
    STATE_ANSWER_OBD_REQUEST,
    STATE_IDLE
} states_t;

char[18] VIN = "3FADP4FJ2BM11391";

uint8_t len = 0;
uint8_t buf[8];
char str[20];

states_t state = STATE_IDLE;

void setup()
{
    Serial.begin(38400);

START_INIT:

    if(CAN_OK == CAN.begin(CAN_500KBPS))
    {
        Serial.println("CAN BUS Shield init ok!");
    }
    else
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println("Init CAN BUS Shield again");
        delay(100);
        goto START_INIT;
    }
}


void loop()
{
    int8_t rpm=random(1,55);
    int8_t speed=random(0,255);
    int8_t load=random(1,55);
    
    //GENERAL ROUTINE
    uint8_t SupportedPID[1] =       {1};
    uint8_t MilCleared[7] =         {4, 65, 63, 34, 224, 185, 147}; 
    
    //SENSORS
    uint8_t rpmFrame[7] =              {0x04, 0x40 & 0x01, 0x0C, rpm1, rpm2, 0, 0};
    uint8_t speedFrame[7] =            {0x04, 0x40 & 0x01, 0x0D, speed, 0, 0, 0};
    uint8_t loadFrame[7] =             {0x04, 0x40 & 0x01, 0x04, load, 0, 0, 0};

    if(CAN_MSGAVAIL == CAN.checkReceive())  
    {

        CAN.readMsgBuf(&len, buf); 
        canId = CAN.getCanId();
        Serial.print("<");Serial.print(canId);Serial.print(",");

        if ( state == STATE_IDLE )
        {
            uint8_t len = buf[1];
            uint8_t mode = buf[1];
            uint8_t pid = buf[2];

            if ( mode == 0x02)
            {
                switch ( pid ) 
                {
                    case 0x01:
                        CAN.sendMsgBuf(0x7E8, 0, 1, SupportedPID);
                      break;
                    case 0x04:
                      CAN.sendMsgBuf(0x7E8, 0, 7, loadFrame);
                      break;
                    case 0x0C:
                        CAN.sendMsgBuf(0x7E8, 0, 7, rpmFrame);
                        break;
                    case 0x0D:
                        CAN.sendMsgBuf(0x7E8, 0, 7, speedFrame);
                        break;
                    default:
                      // do something
                }
            }
            else if ( mode == 0x09 )
            {
                if ( pid == 0x02 )
                {
                    byte_t payload = {
                        FRAME_TYPE_FIRST & 0x00,
                        0x14,
                        ANSWER_TO_MODE9,
                        MODE9_PID2_VIN,
                        0x01,
                        VIN[0],
                        VIN[1],
                        VIN[2]
                    }
                    CAN.sendMsgBuf(0x7E8, 0, 8, (byte_t){0x10 & 0x00, 0x14, 0x49, 0x02, 0x01 });
                }
            }
        }
        else if ( state == STATE_WAIT_FLOW_CONTROL_FRAME )
        {
            if ( IS_A_FLOW_CONTROL_FRAME ( buf ) )
            {
                if ( FLOW_CONTROL_FRAME_IS_CONTINUE(buf) )
                {
                    state = STATE_SEND_NEXT_CONSECUTIVE_FRAME;
                    send_next_consecutive_frame();
                }
            }
        }
    }
}
