//This Arduino UNO Sketch requires the Seeed CAN-BUS Shield Libraries
//https://github.com/yexiaobo-seeedstudio/CAN_BUS_Shield
#include <SPI.h>
#include "mcp_can.h"

INT32U canId = 0x000;

typedef unsigned char uint8_t;
typedef char int8_t;

//2024   4   65   12   18   248   185   147
 

uint8_t len = 0;
uint8_t buf[8];
char str[20];

String BuildMessage="";
int MSGIdentifier=0;

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
    uint8_t rpmFrame[7] =                {0x04, 0x40 & 0x01, 0x0C, rpm1, rpm2, 0, 0};
    uint8_t speedFrame[7] =             {0x04, 0x40 & 0x01, 0x0D, speed, 0, 0, 0};
    uint8_t loadFrame[7] =             {0x04, 0x40 & 0x01, 0x04, load, 0, 0, 0};

    if(CAN_MSGAVAIL == CAN.checkReceive())  
    {
      
        CAN.readMsgBuf(&len, buf); 
        canId = CAN.getCanId();

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
            
        }
    }
}
