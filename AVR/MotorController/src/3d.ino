/*
 * 3d:
 */

//#define DISABLE_MOTORS 1 // comment to restore permanently enabled operation
//#define DEBUG 1 //uncomment for serial debug prints

#include "Wire.h" //I2C library
#include "hardware.h" // Pin mapping and other hardware defines
#include "variables.h" // Customisable variables
#include <avr/io.h>  // for DDRB, PORTB, etc. */
#include <digitalWriteFast.h> //digitalWriteFast library


// Globals
static uint32_t microstepsPerStep                   = 1 ;
static uint32_t layerThickness                      = 25;       // microns

//Jog Axes Variables
static uint32_t rotationSpeed                       = 15;       //RPM
static uint32_t zSpeed                              = 4500;     //microns/s
static uint32_t maxZTravel                          = 168000;   //microns
static uint32_t zTravel                             = 0;        //0 is at the top of the Z axis and is the "HOME" position

//Calibration and Home Variables
static uint32_t calibrationFastZTravel              = 163000;   //microns
static uint32_t calibrationSlowZTravel              = 5000;     //microns
static uint32_t calibrationFastZSpeed               = 4500;     //microns/s
static uint32_t calibrationSlowZSpeed               = 2500;     //microns/s
static uint32_t calibrationSlideSpeed               = 6;        //RPM
static uint32_t calibrationSlideAngle               = 60000;    //millidegrees
static uint32_t rotationAxisOffset                  = 0;     //millidegrees

// First Layer Variables
static uint32_t firstLayerZaxisSep                  = 2000;     //microns
static uint32_t firstLayerZaxisApp                  = 1975;     //microns
static uint32_t firstLayerSepZSpeed                 = 5000;     //microns/s
static uint32_t firstLayerAppZSpeed                 = 5000;     //microns/s
static uint32_t firstLayerAngleOfRotation           = 60000;    //millidegrees
static uint32_t firstLayerSeparationSlideVelocity   = 6;        //RPM
static uint32_t firstLayerApproachSlideVelocity     = 6;        //RPM
static uint32_t firstLayerWaitExp                   = 0;        //ms
static uint32_t firstLayerWaitSep                   = 0;        //ms
static uint32_t firstLayerWaitApp                   = 0;        //ms

// Burn-in Layer Variables
static uint32_t burnLayerZaxisSep                   = 2000;     //microns
static uint32_t burnLayerZaxisApp                   = 1975;     //microns
static uint32_t burnLayerSepZSpeed                  = 4500;     //microns/s
static uint32_t burnLayerAppZSpeed                  = 4500;     //microns/s
static uint32_t burnLayerAngleOfRotation            = 60000;    //millidegrees
static uint32_t burnLayerSeparationSlideVelocity    = 11;       //RPM
static uint32_t burnLayerApproachSlideVelocity      = 11;       //RPM
static uint32_t burnLayerWaitExp                    = 0;        //ms
static uint32_t burnLayerWaitSep                    = 0;        //ms
static uint32_t burnLayerWaitApp                    = 0;        //ms

// Model Layer Variables
static uint32_t modelLayerZaxisSep                  = 2000;     //microns
static uint32_t modelLayerZaxisApp                  = 1975;     //microns
static uint32_t modelLayerSepZSpeed                 = 4500;     //microns/s
static uint32_t modelLayerAppZSpeed                 = 4500;     //microns/s
static uint32_t modelLayerAngleOfRotation           = 60000;    //millidegrees
static uint32_t modelLayerSeparationSlideVelocity   = 12;       //RPM
static uint32_t modelLayerApproachSlideVelocity     = 12;       //RPM
static uint32_t modelLayerWaitExp                   = 0;        //ms
static uint32_t modelLayerWaitSep                   = 0;        //ms
static uint32_t modelLayerWaitApp                   = 0;        //ms

volatile bool i2c_data = false; // flag to indicate if I2C data received

/*
 * setMicrostep:
 *	Set the microstep bits on the setpper controllers
 *********************************************************************************
 */

static void setMicrostep (uint8_t uStep)
{
    uint8_t ms0 = 0, ms1 = 0, ms2 = 0 ;
    
    switch (uStep)
    {
        case 1:	microstepsPerStep =  1 ; 			break ;
        case 2:	microstepsPerStep =  2 ; ms0 = 1 ;		break ;
        case 3:	microstepsPerStep =  4 ; ms1 = 1 ;		break ;
        case 4:	microstepsPerStep =  8 ; ms0 = ms1 = 1 ;	break ;
        case 5:	microstepsPerStep = 16 ; ms2 = 1 ;		break ;
        case 6:	microstepsPerStep = 32 ; ms0 = ms2 = 1 ;	break ;
    }
    
    digitalWriteFast (MOTOR_MS0, ms0) ;
    digitalWriteFast (MOTOR_MS1, ms1) ;
    digitalWriteFast (MOTOR_MS2, ms2) ;
}


/*
 * jogZ:
 *	Move the Z stepper motor up or down at a steady pace - say 0.1mm/sec
 *********************************************************************************
 */

void jogZ (uint32_t jogZDistance, uint32_t jogZSpeed, int jogZDirection)
{
    /*This method has been altered to so that the Z axis will not move in the up direction
     * if the Z axis microswitch is triggered (the pin is reading "LOW").
     * A counter has also been added so that the current position of the build platform
     * realtive to the microswicth position can be seen in minicom
     */
    
    uint32_t jogZPulses, jogZMovementTime, jogZStepPeriod ;
    
    jogZPulses = (jogZDistance * Z_STEPS_PER_REV * microstepsPerStep) / Z_PITCH ;
    jogZMovementTime = (jogZDistance * 1000UL) / jogZSpeed;
    jogZStepPeriod = (jogZMovementTime * 1000UL) /jogZPulses ;
    
    digitalWriteFast (Z_MOTOR_DIR, jogZDirection) ;
    delayMicroseconds (5) ;
    
    //IF the jogZ is in the up direction check whether the microswitch is triggered
    if (jogZDirection == UP_DIRECTION)
    {
        //If the microswitch is not triggered (pin reads "HIGH") then complete the jog
        if (digitalReadFast (Z_ENDSTOP) == HIGH)
        {
            //Counter to count the position the  build platfrom will travel to
            zTravel = zTravel - jogZDistance;
            
            if (zTravel < 0)
            {
                zTravel = 0UL;
            }
#ifdef DEBUG
            Serial.print("\n");
            Serial.print ("jogZPulses: "); Serial.println(jogZPulses);
            Serial.print ("Z Position: "); Serial.print(zTravel); Serial.println(" microns");
#endif
            
            // Z aixs moves up for distance defined by "jogZPulses" at speed defined by jogZSpeed
            
            while (jogZPulses-- !=0)
            {
                digitalWriteFast (Z_MOTOR_STEP, 1) ;
                delayMicroseconds (jogZStepPeriod/2) ;
                digitalWriteFast (Z_MOTOR_STEP, 0) ;
                delayMicroseconds (jogZStepPeriod/2) ;
                
                //If the Z axis microswitch is triggered (pin reads "LOW") during the jog up then set jogZPulses=0 and stop the movement
                if (digitalReadFast (Z_ENDSTOP) == LOW)
                {
                    jogZPulses = 0UL;
                    zTravel = 0UL;
                }
            }
        }
        
        //If the microswitch is triggered (pin reads "LOW") do not perform the jog
        else
        {
            //zTravel =0 as the microswitch is triggered and thus the build platfrom is at the "HOME" position
            zTravel = 0UL;
#ifdef DEBUG
            Serial.print("\n");
            Serial.print ("Z Position: "); Serial.print(zTravel); Serial.println(" microns");
#endif
        }
        
    }
    
    //If the jogZ is in the down direction check that it is not greater than the max. permissible travel
    else
    {
        
        //Counter to count the position the  build platfrom will travel to
        zTravel = zTravel + jogZDistance;
        if(zTravel <= maxZTravel)
        {
            //Print out the position the build platfrom will travel to
#ifdef DEBUG
            Serial.print("\n");
            Serial.print ("Z Position: "); Serial.print(zTravel); Serial.println(" microns");
#endif
            //Z axis moves down for the distance defined by "jogZPulses" at speed defined by jogZSpeed
            while (jogZPulses-- != 0)
            {
                digitalWriteFast (Z_MOTOR_STEP, 1) ;
                delayMicroseconds (jogZStepPeriod/2) ;
                digitalWriteFast (Z_MOTOR_STEP, 0) ;
                delayMicroseconds (jogZStepPeriod/2) ;
            }
        }
        
        //if the zTravel is greater than the max permissible travel then do not perfrom the jog
        else
        {
            //set zTravel back to value before the jog down
            zTravel = zTravel - jogZDistance;
#ifdef DEBUG
            Serial.print("\n");
            Serial.print("Jog is greater than maximum permissible z travel");
#endif
        }
    }
}

/*
 * jogTank:
 *      Move the tank stepper motor at a steady pace
 *********************************************************************************
 */

void jogTank (uint32_t JogTankAngle, uint32_t jogTankSpeed, int jogTankDirection)
{
    uint32_t jogTanksSteps, jogTanksPulses, targetStepFreq, startStepFreq, accelerationSteps, deltaStepFreq, i, j, microdegreesPerStep, microRotationAngle ;
    i = 0;
    j = 0;
    deltaStepFreq = 2;
    accelerationSteps = 50;
    
    //Calculate the micro-degrees per step
    microdegreesPerStep = 360000000 / (TANK_STEPS_PER_REV * microstepsPerStep);
    
    //Convert the rotation angle input from millidegrees to microdegrees
    microRotationAngle = JogTankAngle * 1000;
    
    // Calculate steps reqruied to move stepper motor through the angle microRotationAngle
    jogTanksSteps = microRotationAngle / microdegreesPerStep;
    
    //Calcaulate acceleration profile parameters
    jogTanksPulses = jogTanksSteps;
    targetStepFreq  = (60000000UL / jogTankSpeed) / (TANK_STEPS_PER_REV * microstepsPerStep);
    startStepFreq = targetStepFreq + ( accelerationSteps * deltaStepFreq) ;
    
    digitalWriteFast (TANK_MOTOR_DIR, jogTankDirection) ;
    delayMicroseconds (1) ;
    
    while (jogTanksPulses-- != 0)
    {
        if(jogTanksPulses >= jogTanksSteps - accelerationSteps)
        {
            i = i+1;
            
            digitalWriteFast (TANK_MOTOR_STEP, 1) ;
            delayMicroseconds ((startStepFreq - i * deltaStepFreq)/2) ;
            digitalWriteFast (TANK_MOTOR_STEP, 0) ;
            delayMicroseconds ((startStepFreq - i * deltaStepFreq)/2) ;
        }
        
        else if(jogTanksPulses > accelerationSteps)
        {
            digitalWriteFast (TANK_MOTOR_STEP, 1) ;
            delayMicroseconds (targetStepFreq/2) ;
            digitalWriteFast (TANK_MOTOR_STEP, 0) ;
            delayMicroseconds (targetStepFreq/2) ;
        }
        
        else
        {
            j = j + 1;
            
            digitalWriteFast (TANK_MOTOR_STEP, 1) ;
            delayMicroseconds ((targetStepFreq + j * deltaStepFreq)/2) ;
            digitalWriteFast (TANK_MOTOR_STEP, 0) ;
            delayMicroseconds ((targetStepFreq + j * deltaStepFreq)/2) ;
        }
    }
}

/*
 * homeAxes:
 *      Home the  rotation axis and Z axis
 *********************************************************************************
 */

void homeAxes (void)
{
    
    uint32_t homePulses, homeStepPeriod ;
    
    homePulses =  (Z_STEPS_PER_REV * microstepsPerStep) / Z_PITCH ;
    homeStepPeriod = (1000000UL / calibrationFastZSpeed) /homePulses ;
    
    //Home Rotation Axis
    if (digitalReadFast (TANK_ENDSTOP) == HIGH)
    {
        while (digitalReadFast (TANK_ENDSTOP) == HIGH)
        {
            jogTank (1000, calibrationSlideSpeed, CLOCKWISE);
        }
        
        jogTank (rotationAxisOffset, calibrationSlideSpeed, CLOCKWISE);
        delayMicroseconds (5);
        
        jogTank (calibrationSlideAngle, calibrationSlideSpeed, ANTI_CLOCKWISE);
        delayMicroseconds (5);
        
    }
    
    else
    {
        jogTank (calibrationSlideAngle, calibrationSlideSpeed, ANTI_CLOCKWISE);
        delayMicroseconds (5);
        
    }
    
    
    //Home Z-axis
    digitalWriteFast (Z_MOTOR_DIR, UP_DIRECTION) ;
    delayMicroseconds (1) ;
    
    while (digitalReadFast (Z_ENDSTOP) == HIGH)
    {
        digitalWriteFast (Z_MOTOR_STEP, 1) ;
        delayMicroseconds (homeStepPeriod/2) ;
        digitalWriteFast (Z_MOTOR_STEP, 0) ;
        delayMicroseconds (homeStepPeriod/2) ;
        
    }
    
    //Set the zTravel counter to zero to indicate "HOME" position
    zTravel = 0;
#ifdef DEBUG
    Serial.print("\n");
    Serial.print("Z Position: "); Serial.print(zTravel); Serial.println("microns");
#endif
    
}


/*
 * calibrateAxes:
 *      Move the  rotation axis and the Z axis so that the build head is at the start position
 *********************************************************************************
 */
void calibrateAxes (void)
{
    //home the rotation and z axes
    homeAxes();
    delayMicroseconds (5);
    //move the z axis downwards for distance calibrationFastZTravel
    jogZ (calibrationFastZTravel, calibrationFastZSpeed, DOWN_DIRECTION);
    delayMicroseconds (5);
    //rotate the rotation axis by angle calibrationSlideAngle
    jogTank (calibrationSlideAngle, calibrationSlideSpeed, CLOCKWISE);
    delayMicroseconds (5);
    //move the z axis downards for distance calibrationSlowZTravel
    jogZ (calibrationSlowZTravel, calibrationSlowZSpeed, DOWN_DIRECTION);
    delayMicroseconds (5);
}


/*
 * firstLayerPrintCycle:
 *********************************************************************************
 */

void firstLayerPrintCycle (void)
{
    
    delay (firstLayerWaitExp);
    //Rotation Separation
    jogTank (firstLayerAngleOfRotation, firstLayerSeparationSlideVelocity, ANTI_CLOCKWISE);
    delayMicroseconds (5);
    //Z axis Separation
    jogZ (firstLayerZaxisSep, firstLayerSepZSpeed, UP_DIRECTION);
    delay (firstLayerWaitSep);
    //Rotation Approach
    jogTank (firstLayerAngleOfRotation, firstLayerApproachSlideVelocity, CLOCKWISE);
    delayMicroseconds (5);
    //Z axis Approach
    jogZ (firstLayerZaxisApp, firstLayerAppZSpeed, DOWN_DIRECTION);
    delay (firstLayerWaitApp);
    
}

/*
 * burnLayerPrintCycle:
 *********************************************************************************
 */

void burnLayerPrintCycle (void)
{
    
    delay (burnLayerWaitExp);
    //Rotation Separation
    jogTank (burnLayerAngleOfRotation, burnLayerSeparationSlideVelocity, ANTI_CLOCKWISE);
    delayMicroseconds (5);
    //Z axis Separation
    jogZ (burnLayerZaxisSep, burnLayerSepZSpeed, UP_DIRECTION);
    delay (burnLayerWaitSep);
    //Rotation Approach
    jogTank (burnLayerAngleOfRotation, burnLayerApproachSlideVelocity, CLOCKWISE);
    delayMicroseconds (5);
    //Z axis Approach
    jogZ (burnLayerZaxisApp, burnLayerAppZSpeed, DOWN_DIRECTION);
    delay (burnLayerWaitApp);
    
}

/*
 * ModelLayerPrintCycle:
 *********************************************************************************
 */

void modelLayerPrintCycle (void)
{
    delay (modelLayerWaitExp);
    //Rotation Separation
    jogTank (modelLayerAngleOfRotation, modelLayerSeparationSlideVelocity, ANTI_CLOCKWISE);
    delayMicroseconds (5);
    //Z axis Separation
    jogZ (modelLayerZaxisSep, modelLayerSepZSpeed, UP_DIRECTION);
    delay (modelLayerWaitSep);
    //Rotation Approach
    jogTank (modelLayerAngleOfRotation, modelLayerApproachSlideVelocity, CLOCKWISE);
    delayMicroseconds (5);
    //Z axis Approach
    jogZ (modelLayerZaxisApp, modelLayerAppZSpeed, DOWN_DIRECTION);
    delay (modelLayerWaitApp);
    
}




void do_interrupt() {
#ifdef DEBUG
    Serial.println("Sending INT");
#endif
    digitalWriteFast(MB_INT,HIGH);
    delay(INT_TIME);
    digitalWriteFast(MB_INT,LOW);
}

void ignore_i2c_data(int numBytes) {
    
}

void receive_i2c_data(int numBytes) {
#ifdef DEBUG
    Serial.print("Got data:");
    Serial.print(numBytes);
    Serial.println(" bytes");
#endif
    i2c_data=true;
}

void on_i2c_request() {
    Wire.write(0x00);
    //do nothing
}


void flush_i2c() {
    //flush i2c
    while (Wire.available()) {
        Wire.read();
    }
}

#ifdef DEBUG
void print_i2c() {
    char c;
    while (Wire.available()) {
        c = Wire.read();
        Serial.println(c);
    }
    
}
#endif

/*
 * main:
 *	Setup hardware and listen for commands
 *********************************************************************************
 */

void setup(void) {
    
#ifdef DEBUG
    Serial.begin(9600) ;
    Serial.println("3D Stepper Firmware") ;
#endif
    
    Wire.begin(I2C_ADDRESS);
    Wire.onRequest(on_i2c_request);
    
    for (int i = 2 ; i < 14 ; ++i)
    {
        digitalWriteFast (i, LOW) ;
        pinMode      (i, OUTPUT) ;
    }
    pinMode(M_SLEEPn,OUTPUT);
    pinMode(M_ENABLEn,OUTPUT);
    pinMode(MB_INT,OUTPUT);
    digitalWriteFast(MB_INT,HIGH);
    pinMode(Z_ENDSTOP,INPUT);
    pinMode(TANK_ENDSTOP,INPUT);
    
    
    delay (1) ;
    digitalWriteFast (MOTOR_RESET, HIGH) ;	// Enables the steppers
    digitalWriteFast (M_SLEEPn, HIGH) ;
    digitalWriteFast (M_ENABLEn, HIGH) ;
    
    pinMode(Z_MOTOR_DIR,OUTPUT);
    pinMode(TANK_MOTOR_DIR,OUTPUT);
    digitalWriteFast (Z_MOTOR_DIR,    UP_DIRECTION) ;
    digitalWriteFast (TANK_MOTOR_DIR, CLOCKWISE) ;
    setMicrostep (6) ;
    
#ifdef USE_MCU_CURRENT_LIMIT
    int limit = CURRENT_LIMIT;
    limit = min(limit,2.5);
    limit = max(limit,0);
    
    int vref = ((limit/2)/LOGIC_LEVEL)/255;
    
    analogWrite(M1_VREF,vref);
    analogWrite(M2_VREF,vref);
    
#endif
    
}

boolean validCommand = false;

void loop(void) {
    int i  ;
    uint8_t cmd, c2, cmd2 ;
    uint32_t newLayerThickness, valid, newRotationSpeed, newJogZDistanceUp, newJogZDistanceDown, newZSpeed, newClockwiseRotationAngle, newAntiClockwiseRotationAngle, newMaxZTravel;
    
    
    //Calibration and home variables
    uint32_t newCalibrationFastZTravel;                         //microns
    uint32_t newCalibrationSlowZTravel;                         //microns
    uint32_t newCalibrationFastZSpeed;                          //microns/s
    uint32_t newCalibrationSlowZSpeed;                          //microns/s
    uint32_t newCalibrationSlideSpeed;                          //RPM
    uint32_t newCalibrationSlideAngle;                          //millidegrees
    uint32_t newrotationAxisOffset;                             //millidegrees
    
    //First layer variables
    uint32_t newFirstLayerSeparationSlideVelocity;              //RPM
    uint32_t newFirstLayerApproachSlideVelocity;                //RPM
    uint32_t newFirstLayerZaxisSep;                             //microns
    firstLayerZaxisApp = firstLayerZaxisSep - layerThickness;   //microns
    uint32_t newFirstLayerSepZSpeed;                            //microns/s
    uint32_t newFirstLayerAppZSpeed;                            //microns/s
    uint32_t newFirstLayerAngleOfRotation;                      //millidegrees
    uint32_t newFirstLayerWaitExp;                               //ms
    uint32_t newFirstLayerWaitSep;                               //ms
    uint32_t newFirstLayerWaitApp;                               //ms
    
    //Burn in layer variables
    uint32_t newBurnLayerSeparationSlideVelocity;               //RPM
    uint32_t newBurnLayerApproachSlideVelocity;                 //RPM
    uint32_t newBurnLayerZaxisSep;                              //microns
    burnLayerZaxisApp = burnLayerZaxisSep - layerThickness;     //microns
    uint32_t newBurnLayerSepZSpeed;                             //microns/s
    uint32_t newBurnLayerAppZSpeed;                             //microns/s
    uint32_t newBurnLayerAngleOfRotation;                       //millidegrees
    uint32_t newBurnLayerWaitExp;                               //ms
    uint32_t newBurnLayerWaitSep;                               //ms
    uint32_t newBurnLayerWaitApp;                               //ms
    
    //Model layer variables
    uint32_t newModelLayerSeparationSlideVelocity;
    uint32_t newModelLayerApproachSlideVelocity;                 //RPM
    uint32_t newModelLayerZaxisSep;                              //microns
    modelLayerZaxisApp = modelLayerZaxisSep - layerThickness;    //microns
    uint32_t newModelLayerSepZSpeed;                             //microns/s
    uint32_t newModelLayerAppZSpeed;                             //microns/s
    uint32_t newModelLayerAngleOfRotation;                       //millidegrees
    uint32_t newModelLayerWaitExp;                               //ms
    uint32_t newModelLayerWaitSep;                               //ms
    uint32_t newModelLayerWaitApp;                               //ms
    
    if (validCommand) {
        do_interrupt();
    }
    validCommand = true;
#ifdef DEBUG
    Serial.println("Waiting for data");
#endif
    Wire.onReceive(receive_i2c_data);
    while (!i2c_data) ; //loop until received I2C data
    i2c_data=false;
    int i2c_register = Wire.read () ; //ignore address byte
    cmd = Wire.read () ; // 2nd byte is command
    
    digitalWriteFast (M_ENABLEn, LOW) ; // ensure motors enabled
    
    switch (cmd)
    {
        case '@':			// Sync
#ifdef DEBUG
            Serial.println ("Got @ command");
#endif
            break ;
            
        case 's':			// Sync
#ifdef DEBUG
            Serial.println ("Got s command");
#endif
            break ;
            
        case 'c':        		// Move z axis and rotation axis to print ready position
#ifdef DEBUG
            Serial.println ("Got C command");
#endif
            calibrateAxes ();
#ifdef DEBUG
            Serial.print ("\n") ;
            Serial.print ("Z Position: "); Serial.print(zTravel); Serial.println(" microns");
#endif
            break;
            
        case 'C':
            cmd2 = Wire.read ();
            switch (cmd2)
        {
            case 'a':
                newCalibrationFastZTravel = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newCalibrationFastZTravel = newCalibrationFastZTravel * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    calibrationFastZTravel = newCalibrationFastZTravel;
#ifdef DEBUG
                    Serial.print ("calibrationFastZTravel set to: ");
                    Serial.println ( newCalibrationFastZTravel );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid ca data");
#endif
                }
                break ;
                
            case 'b':
                newCalibrationSlowZTravel = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newCalibrationSlowZTravel = newCalibrationSlowZTravel * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    calibrationSlowZTravel = newCalibrationSlowZTravel;
#ifdef DEBUG
                    Serial.print ("calibrationSlowZTravel set to: ");
                    Serial.println ( newCalibrationSlowZTravel );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid cb data");
#endif
                }
                break ;
                
            case 'c':
                newCalibrationFastZSpeed = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newCalibrationFastZSpeed = newCalibrationFastZSpeed * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    calibrationFastZSpeed = newCalibrationFastZSpeed;
#ifdef DEBUG
                    Serial.print ("calibrationFastZSpeed set to: ");
                    Serial.println ( newCalibrationFastZSpeed );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid cc data");
#endif
                }
                break ;
                
            case 'd':
                newCalibrationSlowZSpeed = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newCalibrationSlowZSpeed = newCalibrationSlowZSpeed * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    calibrationSlowZSpeed = newCalibrationSlowZSpeed;
#ifdef DEBUG
                    Serial.print ("calibrationSlowZSpeed set to: ");
                    Serial.println ( newCalibrationSlowZSpeed );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid cd data");
#endif
                }
                break ;
                
            case 'e':
                newCalibrationSlideSpeed = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newCalibrationSlideSpeed = newCalibrationSlideSpeed * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    calibrationSlideSpeed = newCalibrationSlideSpeed;
#ifdef DEBUG
                    Serial.print ("calibrationSlideSpeed set to: ");
                    Serial.println ( newCalibrationSlideSpeed );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid ce data");
#endif
                }
                break ;
                
            case 'f':
                newCalibrationSlideAngle = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newCalibrationSlideAngle = newCalibrationSlideAngle * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    calibrationSlideAngle = newCalibrationSlideAngle;
#ifdef DEBUG
                    Serial.print ("calibrationSlideAngle set to: ");
                    Serial.println ( newCalibrationSlideAngle );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid cf data");
#endif
                }
                break ;
                
            case 'g':
                newrotationAxisOffset = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newrotationAxisOffset = newrotationAxisOffset * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    rotationAxisOffset = newrotationAxisOffset;
#ifdef DEBUG
                    Serial.print ("rotationAxisOffset set to: ");
                    Serial.println ( newrotationAxisOffset );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid cg data");
#endif
                }
                break ;
                
                
                
        }
            break ;
            
        case 'h':			// Home z axis up
#ifdef DEBUG
            Serial.println ("Got h command");
#endif
            homeAxes() ;
#ifdef DEBUG
            Serial.print ("\n") ;
            Serial.print ("Z Position: "); Serial.print(zTravel); Serial.println(" microns");
            Serial.println("Print complete - disabling motors");
#endif
            //print over disable
            //digitalWriteFast(M_ENABLEn,HIGH);
            break ;
            
        case 'm':			// Set microstep
#ifdef DEBUG
            Serial.println ("Got m command");
            i = (int) Wire.read () - '0' ;
#endif
#ifdef DEBUG
            Serial.print ("Setting microsteps mode: ");
            Serial.println(i);
#endif
            setMicrostep (i) ;
            break ;
            
        case 'W':			// Set rotation speed
            newRotationSpeed = 0 ;
            valid   = TRUE ;
            for (i = 0 ; i < 6 ; ++i)
            {
                c2 = Wire.read () ;
                if ((c2 < '0') || (c2 > '9'))
                    valid = FALSE ;
                else
                    newRotationSpeed = newRotationSpeed * 10 + (c2 - '0') ;
            }
            if (valid) {
                rotationSpeed = newRotationSpeed ;
#ifdef DEBUG
                Serial.print ("Rotation Speed set to: ");
                Serial.println ( rotationSpeed );
#endif
            }
            else {
#ifdef DEBUG
                Serial.println ("Invalid s data");
#endif
            }
            
            break ;
            
        case 'S':			// Set Z speed
            newZSpeed = 0 ;
            valid   = TRUE ;
            for (i = 0 ; i < 6 ; ++i)
            {
                c2 = Wire.read () ;
                if ((c2 < '0') || (c2 > '9'))
                    valid = FALSE ;
                else
                    newZSpeed = newZSpeed * 10 + (c2 - '0') ;
            }
            if (valid) {
                zSpeed = newZSpeed ;
#ifdef DEBUG
                Serial.print ("Z Speed set to: ");
                Serial.println ( newZSpeed );
#endif
            }
            else {
#ifdef DEBUG
                Serial.println ("Invalid S data");
#endif
            }
            
            break ;
            
        case 'Z':			// Set Z tarvel
            newMaxZTravel = 0 ;
            valid   = TRUE ;
            for (i = 0 ; i < 6 ; ++i)
            {
                c2 = Wire.read () ;
                if ((c2 < '0') || (c2 > '9'))
                    valid = FALSE ;
                else
                    newMaxZTravel = newMaxZTravel * 10 + (c2 - '0') ;
            }
            if (valid) {
                maxZTravel = newMaxZTravel ;
#ifdef DEBUG
                Serial.print ("maxZTravel set to: ");
                Serial.println ( newMaxZTravel );
#endif
            }
            else {
#ifdef DEBUG
                Serial.println ("Invalid S data");
#endif
            }
            
            break ;
            
        case 'U':			// jog Z axis up by inputted distance
            //jogZ (100UL, UP_DIRECTION) ;
            
            newJogZDistanceUp = 0 ;
            valid   = TRUE ;
            for (i = 0 ; i < 6 ; ++i)
            {
                c2 = Wire.read () ;
                if ((c2 < '0') || (c2 > '9'))
                    valid = FALSE ;
                else
                    newJogZDistanceUp = newJogZDistanceUp * 10 + (c2 - '0') ;
            }
            if (valid) {
                jogZ (newJogZDistanceUp, zSpeed, UP_DIRECTION) ;
#ifdef DEBUG
                Serial.print ("Jog Z distance set to: ");
                Serial.println ( newJogZDistanceUp );
#endif
            }
            else {
#ifdef DEBUG
                Serial.println ("Invalid u data");
#endif
            }
            
            break ;
            
        case 'D':			// jog Z axis down by inputted distance
            newJogZDistanceDown = 0 ;
            valid   = TRUE ;
            for (i = 0 ; i < 6 ; ++i)
            {
                c2 = Wire.read () ;
                if ((c2 < '0') || (c2 > '9'))
                    valid = FALSE ;
                else
                    newJogZDistanceDown = newJogZDistanceDown * 10 + (c2 - '0') ;
            }
            if (valid) {
                jogZ (newJogZDistanceDown, zSpeed, DOWN_DIRECTION) ;
#ifdef DEBUG
                Serial.print ("Jog Z distance set to: ");
                Serial.println ( newJogZDistanceDown );
#endif
            }
            else {
#ifdef DEBUG
                Serial.println ("Invalid d data");
#endif
            }
            
            
            break ;
            
        case 'r':			// Rotate the tank clockwise with input in millidegrees
            
#ifdef DEBUG
            Serial.println ("Got r command");
#endif
            //jogTank (60, CLOCKWISE) ;
            
            newClockwiseRotationAngle = 0 ;
            valid   = TRUE ;
            for (i = 0 ; i < 6 ; ++i)
            {
                c2 = Wire.read () ;
                if ((c2 < '0') || (c2 > '9'))
                    valid = FALSE ;
                else
                    newClockwiseRotationAngle = newClockwiseRotationAngle * 10 + (c2 - '0') ;
            }
            if (valid) {
                jogTank (newClockwiseRotationAngle, rotationSpeed,  CLOCKWISE) ;
#ifdef DEBUG
                Serial.print ("Clockwise Rotation Angle set to: ");
                Serial.println ( newClockwiseRotationAngle );
#endif
            }
            else {
#ifdef DEBUG
                Serial.println ("Invalid R data");
#endif
            }
            break ;
            
        case 'R':			// Rotate the tank acw with input in millidegrees
            //jogTank ( ANTI_CLOCKWISE) ;
            
            newAntiClockwiseRotationAngle = 0 ;
            valid   = TRUE ;
            for (i = 0 ; i < 6 ; ++i)
            {
                c2 = Wire.read () ;
                if ((c2 < '0') || (c2 > '9'))
                    valid = FALSE ;
                else
                    newAntiClockwiseRotationAngle = newAntiClockwiseRotationAngle * 10 + (c2 - '0') ;
            }
            if (valid) {
                jogTank (newAntiClockwiseRotationAngle, rotationSpeed, ANTI_CLOCKWISE) ;
#ifdef DEBUG
                Serial.print ("Anti-Clockwise Rotation Angle set to: ");
                Serial.println ( newAntiClockwiseRotationAngle );
#endif
            }
            else {
#ifdef DEBUG
                Serial.println ("Invalid R data");
#endif
            }
            break ;
            
        case 'F':                 // First Layer Print cycle
            firstLayerPrintCycle () ;
            break ;
            
        case 'f':
            cmd2 = Wire.read ();
            switch (cmd2)
        {
            case 'a':
                newFirstLayerSeparationSlideVelocity = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newFirstLayerSeparationSlideVelocity = newFirstLayerSeparationSlideVelocity * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    firstLayerSeparationSlideVelocity = newFirstLayerSeparationSlideVelocity;
#ifdef DEBUG
                    Serial.print ("firstLayerSeparationSlideVelocity set to: ");
                    Serial.println ( newFirstLayerSeparationSlideVelocity );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid fa data");
#endif
                }
                break ;
                
            case 'b':
                newFirstLayerApproachSlideVelocity = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newFirstLayerApproachSlideVelocity = newFirstLayerApproachSlideVelocity * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    firstLayerApproachSlideVelocity = newFirstLayerApproachSlideVelocity;
#ifdef DEBUG
                    Serial.print ("firstLayerApproachSlideVelocity set to: ");
                    Serial.println ( newFirstLayerApproachSlideVelocity );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid fb data");
#endif
                }
                break ;
                
            case 'c':
                newFirstLayerZaxisSep = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newFirstLayerZaxisSep = newFirstLayerZaxisSep * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    firstLayerZaxisSep = newFirstLayerZaxisSep;
#ifdef DEBUG
                    Serial.print ("firstLayerZaxisSep set to: ");
                    Serial.println ( newFirstLayerZaxisSep );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid fc data");
#endif
                }
                break ;
                
            case 'd':
                newFirstLayerSepZSpeed = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newFirstLayerSepZSpeed = newFirstLayerSepZSpeed * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    firstLayerSepZSpeed = newFirstLayerSepZSpeed;
#ifdef DEBUG
                    Serial.print ("firstLayerSepZSpeed set to: ");
                    Serial.println ( newFirstLayerSepZSpeed );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid fd data");
#endif
                }
                break ;
                
            case 'e':
                newFirstLayerAppZSpeed = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newFirstLayerAppZSpeed = newFirstLayerAppZSpeed * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    firstLayerAppZSpeed = newFirstLayerAppZSpeed;
#ifdef DEBUG
                    Serial.print ("firstLayerAppZSpeed set to: ");
                    Serial.println ( newFirstLayerAppZSpeed );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid fe data");
#endif
                }
                break ;
                
            case 'f':
                newFirstLayerAngleOfRotation = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newFirstLayerAngleOfRotation = newFirstLayerAngleOfRotation * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    firstLayerAngleOfRotation = newFirstLayerAngleOfRotation;
#ifdef DEBUG
                    Serial.print ("firstLayerAngleOfRotation set to: ");
                    Serial.println ( newFirstLayerAngleOfRotation );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid ff data");
#endif
                }
                break ;
                
            case 'g':
                newFirstLayerWaitExp = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newFirstLayerWaitExp = newFirstLayerWaitExp * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    firstLayerWaitExp = newFirstLayerWaitExp;
#ifdef DEBUG
                    Serial.print ("firstLayerWaitExp set to: ");
                    Serial.println ( newFirstLayerWaitExp );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid fg data");
#endif
                }
                break ;
                
            case 'h':
                newFirstLayerWaitSep = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newFirstLayerWaitSep = newFirstLayerWaitSep * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    firstLayerWaitSep = newFirstLayerWaitSep;
#ifdef DEBUG
                    Serial.print ("firstLayerWaitSep set to: ");
                    Serial.println ( newFirstLayerWaitSep );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid fh data");
#endif
                }
                break ;
                
            case 'i':
                newFirstLayerWaitApp = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newFirstLayerWaitApp = newFirstLayerWaitApp * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    firstLayerWaitApp = newFirstLayerWaitApp;
#ifdef DEBUG
                    Serial.print ("firstLayerWaitApp set to: ");
                    Serial.println ( newFirstLayerWaitApp );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid fi data");
#endif
                }
                break ;
                
                
                
        }
            break ;
            
        case 'B':                 // Burn In Layer Print cycle
            burnLayerPrintCycle () ;
            break ;
            
        case 'b':
            cmd2 = Wire.read ();
            switch (cmd2)
        {
            case 'a':
                newBurnLayerSeparationSlideVelocity = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newBurnLayerSeparationSlideVelocity = newBurnLayerSeparationSlideVelocity * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    burnLayerSeparationSlideVelocity = newBurnLayerSeparationSlideVelocity;
#ifdef DEBUG
                    Serial.print ("burnLayerSeparationSlideVelocity set to: ");
                    Serial.println ( newBurnLayerSeparationSlideVelocity );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid ba data");
#endif
                }
                break;
                
                
            case 'b':
                newBurnLayerApproachSlideVelocity = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newBurnLayerApproachSlideVelocity = newBurnLayerApproachSlideVelocity * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    burnLayerApproachSlideVelocity = newBurnLayerApproachSlideVelocity;
#ifdef DEBUG
                    Serial.print ("burnLayerApproachSlideVelocity set to: ");
                    Serial.println ( newBurnLayerApproachSlideVelocity );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid bb data");
#endif
                }
                break ;
                
            case 'c':
                newBurnLayerZaxisSep = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newBurnLayerZaxisSep = newBurnLayerZaxisSep * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    burnLayerZaxisSep = newBurnLayerZaxisSep;
#ifdef DEBUG
                    Serial.print ("burnLayerZaxisSep set to: ");
                    Serial.println ( newBurnLayerZaxisSep );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid bc data");
#endif
                }
                break ;
                
            case 'd':
                newBurnLayerSepZSpeed = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newBurnLayerSepZSpeed = newBurnLayerSepZSpeed * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    burnLayerSepZSpeed = newBurnLayerSepZSpeed;
#ifdef DEBUG
                    Serial.print ("burnLayerSepZSpeed set to: ");
                    Serial.println ( newBurnLayerSepZSpeed );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid bd data");
#endif
                }
                break ;
                
            case 'e':
                newBurnLayerAppZSpeed = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newBurnLayerAppZSpeed = newBurnLayerAppZSpeed * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    burnLayerAppZSpeed = newBurnLayerAppZSpeed;
#ifdef DEBUG
                    Serial.print ("burnLayerAppZSpeed set to: ");
                    Serial.println ( newBurnLayerAppZSpeed );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid be data");
#endif
                }
                break ;
                
            case 'f':
                newBurnLayerAngleOfRotation = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newBurnLayerAngleOfRotation = newBurnLayerAngleOfRotation * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    burnLayerAngleOfRotation = newBurnLayerAngleOfRotation;
#ifdef DEBUG
                    Serial.print ("burnLayerAngleOfRotation set to: ");
                    Serial.println ( newBurnLayerAngleOfRotation );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid bf data");
#endif
                }
                break ;
                
            case 'g':
                newBurnLayerWaitExp = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newBurnLayerWaitExp = newBurnLayerWaitExp * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    burnLayerWaitExp = newBurnLayerWaitExp;
#ifdef DEBUG
                    Serial.print ("BurnLayerWaitExp set to: ");
                    Serial.println ( newBurnLayerWaitExp );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid bg data");
#endif
                }
                break ;
                
            case 'h':
                newBurnLayerWaitSep = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newBurnLayerWaitSep = newBurnLayerWaitSep * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    burnLayerWaitSep = newBurnLayerWaitSep;
#ifdef DEBUG
                    Serial.print ("BurnLayerWaitSep set to: ");
                    Serial.println ( newBurnLayerWaitSep );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid bh data");
#endif
                }
                break ;
                
            case 'i':
                newBurnLayerWaitApp = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newBurnLayerWaitApp = newBurnLayerWaitApp * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    burnLayerWaitApp = newBurnLayerWaitApp;
#ifdef DEBUG
                    Serial.print ("BurnLayerWaitApp set to: ");
                    Serial.println ( newBurnLayerWaitApp );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid bi data");
#endif
                }
                break ;
                
                
        }
            break ;
            
        case 'P':                 // Model Layer Print cycle
            modelLayerPrintCycle () ;
            break ;
            
        case 'p':
            cmd2 = Wire.read ();
            switch (cmd2)
        {
            case 'a':
                newModelLayerSeparationSlideVelocity = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newModelLayerSeparationSlideVelocity = newModelLayerSeparationSlideVelocity * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    modelLayerSeparationSlideVelocity = newModelLayerSeparationSlideVelocity;
#ifdef DEBUG
                    Serial.print ("modelLayerSeparationSlideVelocity set to: ");
                    Serial.println ( newModelLayerSeparationSlideVelocity );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid pa data");
#endif
                }
                break;
            case 'b':
                newModelLayerApproachSlideVelocity = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newModelLayerApproachSlideVelocity = newModelLayerApproachSlideVelocity * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    modelLayerApproachSlideVelocity = newModelLayerApproachSlideVelocity;
#ifdef DEBUG
                    Serial.print ("ModelLayerApproachSlideVelocity set to: ");
                    Serial.println ( newModelLayerApproachSlideVelocity );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid pb data");
#endif
                }
                break ;
                
            case 'c':
                newModelLayerZaxisSep = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newModelLayerZaxisSep = newModelLayerZaxisSep * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    modelLayerZaxisSep = newModelLayerZaxisSep;
#ifdef DEBUG
                    Serial.print ("ModelLayerZaxisSep set to: ");
                    Serial.println ( newModelLayerZaxisSep );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid pc data");
#endif
                }
                break ;
                
            case 'd':
                newModelLayerSepZSpeed = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newModelLayerSepZSpeed = newModelLayerSepZSpeed * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    modelLayerSepZSpeed = newModelLayerSepZSpeed;
#ifdef DEBUG
                    Serial.print ("ModelLayerSepZSpeed set to: ");
                    Serial.println ( newModelLayerSepZSpeed );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid pd data");
#endif
                }
                break ;
                
            case 'e':
                newModelLayerAppZSpeed = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newModelLayerAppZSpeed = newModelLayerAppZSpeed * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    modelLayerAppZSpeed = newModelLayerAppZSpeed;
#ifdef DEBUG
                    Serial.print ("ModelLayerAppZSpeed set to: ");
                    Serial.println ( newModelLayerAppZSpeed );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid pe data");
#endif
                }
                break ;
                
            case 'f':
                newModelLayerAngleOfRotation = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newModelLayerAngleOfRotation = newModelLayerAngleOfRotation * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    modelLayerAngleOfRotation = newModelLayerAngleOfRotation;
#ifdef DEBUG
                    Serial.print ("ModelLayerAngleOfRotation set to: ");
                    Serial.println ( newModelLayerAngleOfRotation );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid pf data");
#endif
                }
                break ;
                
            case 'g':
                newModelLayerWaitExp = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newModelLayerWaitExp = newModelLayerWaitExp * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    modelLayerWaitExp = newModelLayerWaitExp;
#ifdef DEBUG
                    Serial.print ("ModelLayerWaitExp set to: ");
                    Serial.println ( newModelLayerWaitExp );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid pg data");
#endif
                }
                break ;
                
            case 'h':
                newModelLayerWaitSep = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newModelLayerWaitSep = newModelLayerWaitSep * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    modelLayerWaitSep = newModelLayerWaitSep;
#ifdef DEBUG
                    Serial.print ("ModelLayerWaitSep set to: ");
                    Serial.println ( newModelLayerWaitSep );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid ph data");
#endif
                }
                break ;
                
            case 'i':
                newModelLayerWaitApp = 0 ;
                valid   = TRUE ;
                for (i = 0 ; i < 6 ; ++i)
                {
                    c2 = Wire.read () ;
                    if ((c2 < '0') || (c2 > '9'))
                        valid = FALSE ;
                    else
                        newModelLayerWaitApp = newModelLayerWaitApp * 10 + (c2 - '0') ;
                }
                if (valid) {
                    
                    modelLayerWaitApp = newModelLayerWaitApp;
#ifdef DEBUG
                    Serial.print ("ModelLayerWaitApp set to: ");
                    Serial.println ( newModelLayerWaitApp );
#endif
                }
                else {
#ifdef DEBUG
                    Serial.println ("Invalid pi data");
#endif
                }
                break ;

                
        }
            break ;
            
        case 'l':			// Set lift distance in 1/1000ths mm
#ifdef DEBUG
            Serial.println ("Got l command");
#endif
            newLayerThickness = 0 ;
            valid   = TRUE ;
            for (i = 0 ; i < 4 ; ++i)
            {
                c2 = Wire.read () ;
                if ((c2 < '0') || (c2 > '9'))
                    valid = FALSE ;
                else
                    newLayerThickness = newLayerThickness * 10 + (c2 - '0') ;
            }
            if (valid) {
                layerThickness = newLayerThickness ;
#ifdef DEBUG
                Serial.print ("Lift distance set to: ");
                Serial.println ( layerThickness );
#endif
            }
            else {
#ifdef DEBUG
                Serial.println ("Invalid l data");
#endif
            }
            break ;
            
        default:
            validCommand = false;
#ifdef DEBUG
            Serial.print("Got unknown command: ");
            Serial.println(cmd);
            print_i2c();
#endif
            
    }
    
#ifdef DISABLE_MOTORS
    digitalWriteFast (M_ENABLEn, HIGH) ;
#endif
    
    flush_i2c();
    
}



