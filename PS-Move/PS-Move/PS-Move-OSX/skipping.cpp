//
//  skipping.cpp
//  PSMoveOSX
//
//  Created by localadmin on 07/06/2018.
//

#include "BtBase.h"
#include "skipping.hpp"
#include "BaArchive.h"
#include "BtTime.h"
#include "SdSound.h"
#include "ShIMU.h"
#include "BtQueue.h"

#define MADGWICK
#include "MadgwickAHRS.h"
Madgwick madgwick[3];

BtBool jump = BtFalse;
BtBool battery[3];

void Skipping::setup()
{
    numControllers = psmove_count_connected();
    
    ShIMU::SetNumSensors( numControllers );
    
    printf("Connected PS Move controllers: %d\n", numControllers );
    
    // Connect all the controllers
    for( int i=0; i<numControllers; i++)
    {
        moveArr[i] = psmove_connect_by_id(i);
        
        // Set the rumble to 0
        psmove_set_rumble( moveArr[i], 0 );
        
        // Set the lights to 0
        psmove_set_leds( moveArr[i], 0, 0, 0 );
        
        battery[i] = BtFalse;
    }
    reset();
}

struct SkippingRope {
    BtQueue<BtFloat, 100> height;
};
SkippingRope rope[3];
MtVector3 v3RelativePosition[3];

void Skipping::reset()
{
#ifdef MADGWICK
    for( BtU32 i=0; i<numControllers; i++ )
    {
        Madgwick &madge = madgwick[i];
        madge.q0 = 1.0f; madge.q1 = madge.q2 = madge.q3 = 0;
        
        MtQuaternion quaternion = MtQuaternion( -madge.q1, -madge.q3, -madge.q2, madge.q0 );
        ShIMU::SetQuaternion( i, quaternion );
        
        rope[i].height.Empty();
    }
#endif
}

void Skipping::update()
{
    jump = BtFalse;
    
    for( BtU32 i=0; i<numControllers; i++ )
    {
         PSMove *move = moveArr[i];
         
         int res = psmove_poll( move );
         if (res)
         {
            MtQuaternion quaternion;
            BtFloat fax, fay, faz;
            BtFloat fgx, fgy, fgz;
         
            BtFloat tick = 1.0f / BtTime::GetTick();
            (void)tick;
            for( BtU32 j=0; j<2; j++ )
            {
                PSMove_Frame frame = (PSMove_Frame)j;
                psmove_get_accelerometer_frame( move, frame, &fax, &fay, &faz );
                psmove_get_gyroscope_frame( move, frame, &fgx, &fgy, &fgz );
                
                Madgwick &madge = madgwick[i];
                madge.MadgwickAHRSupdateIMU( fgx, fgy, fgz, fax, fay, faz );
            }
         
            // Works vertically with x, z, y
            Madgwick &madge = madgwick[i];
            quaternion = MtQuaternion( -madge.q1, -madge.q3, -madge.q2, madge.q0 );
            ShIMU::SetQuaternion( i, quaternion );
            ShIMU::SetAccelerometer( i, MtVector3( fax, fay, faz ) );
         
            // Set their positions
            v3RelativePosition[i].x += fax * BtTime::GetTick();
            v3RelativePosition[i].y += fay * BtTime::GetTick();
            v3RelativePosition[i].z += faz * BtTime::GetTick();
            ShIMU::SetPosition(i, v3RelativePosition[i] );
            v3RelativePosition[i] *= 0.99f;
             
            MtMatrix4 m4Transform;
            m4Transform.SetQuaternion(quaternion);
            MtVector3 v3Grav = MtVector3( fax, fay, faz ) * m4Transform;
             
            if( i == 1 )
            {
                v3Grav.z -= 1.0f;
                if( MtSqrt( (v3Grav.x * v3Grav.x) + (v3Grav.y * v3Grav.y) + (v3Grav.z * v3Grav.z) ) > 2.0f )
                {
                    jump = BtTrue;
                    int a=0;
                    a++;
                }
            }
        }
    }
    
    // Is tipping down
    BtBool down[3];
    
    for( BtU32 i=0; i<3; i++ )
    {
        down[i] = BtFalse;
        
        MtVector3 v3Acc = ShIMU::GetAccelerometer(i);
        
        if( v3Acc.y < 0 )
        {
            down[i] = BtTrue;
        }
    }

    if( jump )
    {
        int a=0;
        a++;
    }
    
    static BtU32 count = 0;
    BtBool bad = BtFalse;
    
    if( down[0] && !jump )
    {
        count++;
    }
    else{
        count = 0;
    }
    
    if( numControllers == 3 )
    {
        if( down[2] && !jump )
        {
            count++;
        }
        else{
            count = 0;
        }
    }
    
    if( count > 20 )
    {
        bad = BtTrue;
    }
    
    // Set the lights
    for( BtU32 i=0; i<numControllers; i++ )
    {
        PSMove *move = moveArr[i];
        
        // Respond to the buttons
        unsigned int pressed, released;
        psmove_get_button_events( move, &pressed, &released);
        if( pressed == Btn_MOVE )
        {
            // Integrate the battery level and set the colour from RED to WHITE
            PSMove_Battery_Level batt = psmove_get_battery( move );
            
            switch( batt )
            {
                case Batt_MIN :
                    psmove_set_leds( move, 255, 0, 0 );
                break;
                case Batt_20Percent:
                    psmove_set_leds( move, 128, 128, 0 );
                break;
                case Batt_40Percent:
                    psmove_set_leds( move, 128, 200, 0 );
                break;
                case Batt_60Percent:
                    psmove_set_leds( move, 0, 200, 0 );
                break;
                case Batt_80Percent :
                    psmove_set_leds( move, 0, 220, 0 );
                break;
                case Batt_MAX:
                    psmove_set_leds( move, 0, 255, 0 );
                break;
                case Batt_CHARGING:
                    psmove_set_leds( move, 0, 0, 255 );
                break;
                case Batt_CHARGING_DONE:
                    psmove_set_leds( move, 255, 255, 255 );
                break;
            }
            battery[i] = BtTrue;
        }
        if( released )
        {
            psmove_set_leds( move, 0, 0, 0 );
            battery[i] = BtFalse;
        }
        
        if( battery[i] == BtFalse )
        {
            // Setup the skipping rope
            if( i == 0 )
            {
                psmove_set_leds( move, 0, 255, 255 );
                if( down[i] )
                {
                    psmove_set_leds( move, 0, 255, 0 );
                }
            }
            if( i == 1 )
            {
                psmove_set_leds( move, 0, 0, 255 );
                
                if( bad )
                {
                    psmove_set_leds( move, 255, 0, 0 );
                }
            }
            if( i == 2 )
            {
                psmove_set_leds( move, 0, 255, 255 );
                if( down[i] )
                {
                    psmove_set_leds( move, 0, 255, 0 );
                }
            }
        }
        
        // Update any changes to the lights
        psmove_update_leds( move );
    }
}

