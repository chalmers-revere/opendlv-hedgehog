
#include <math.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <fstream>
#include <sstream>

bool AutomowerSafe::initAutomowerBoard()
{
    std::cout << "INFO:Automower::initAutomowerBoard " << std::endl;

    hcp_tResult result;
    const char* msg = "DeviceInformation.GetDeviceIdentification()";
    if (!sendMessage(msg, sizeof(msg), result))
    {
        std::cout << "WARNING:Failed to initAutomowerBoard " << std::endl;
        return false;
    }

    uint8_t deviceType = result.parameters[0].value.u8;
    std::cout << "DeviceType: " << (int)deviceType << std::endl;

    uint8_t mowerType = result.parameters[1].value.u8;
    std::cout << "MowerType: " << (int)mowerType << std::endl;

    uint32_t mowerSerial = result.parameters[2].value.u32;
    std::cout << "SerialNo: " << (int)mowerSerial << std::endl;

    uint8_t mowerVariant = result.parameters[3].value.u8;
    std::cout << "MowerVariant: " << (int)mowerVariant << std::endl;

    if (deviceType != 10)
    {
        ROS_ERROR("MowerDevice not found...");
        return false;
    }

    if ((mowerType == 7) || (mowerType == 8))    // 430x OR 450X
    {
        // Get some stuff out from the mower...
        AUTMOWER_WHEEL_BASE_WIDTH = 0.464500;
        WHEEL_DIAMETER = 0.245;
        WHEEL_PULSES_PER_TURN = 349;
        WHEEL_METER_PER_TICK = (2.0 * M_PI * WHEEL_DIAMETER / 2.0) / (double)WHEEL_PULSES_PER_TURN;
    }
    else if (mowerType == 14)   // P0
    {
        AUTMOWER_WHEEL_BASE_WIDTH = 0.3314; // Jonathan Björn
        WHEEL_DIAMETER = 0.210;
        WHEEL_PULSES_PER_TURN = 1192;
        WHEEL_METER_PER_TICK = (2.0 * M_PI * WHEEL_DIAMETER / 2.0) / (double)WHEEL_PULSES_PER_TURN;
    }
    else if (mowerType == 10 || mowerType == 4) //P1
    {
        //Best guesses... todo check really
        AUTMOWER_WHEEL_BASE_WIDTH = 0.3314; // Jonathan Björn
        WHEEL_DIAMETER = 0.210;
        WHEEL_PULSES_PER_TURN = 1188; //12*1:99 according to floor one
        WHEEL_METER_PER_TICK = (2.0 * M_PI * WHEEL_DIAMETER / 2.0) / (double)WHEEL_PULSES_PER_TURN;
    }

    ROS_INFO("Automower::WHEEL_DIAMETER = %f", WHEEL_DIAMETER);
    ROS_INFO("Automower::AUTMOWER_WHEEL_BASE_WIDTH = %f", AUTMOWER_WHEEL_BASE_WIDTH);
    ROS_INFO("Automower::WHEEL_PULSES_PER_TURN = %d", WHEEL_PULSES_PER_TURN);
    ROS_INFO("Automower::WHEEL_METER_PER_TICK = %f", WHEEL_METER_PER_TICK);


    // PID parameters set for 50Hz
    // x factor Adjust to equal regulation as for 50 Hz


    double x = 50.0 / regulatorFreq;

    leftWheelPid.Init(50.0, 10.0*x, 1.0/x);
    rightWheelPid.Init(50.0, 10.0*x, 1.0/x);

    lastComtestWheelMotorPower = 15;

    char msg1[100];
    snprintf(msg1, sizeof(msg1), "MowerApp.SetMode(modeOfOperation:%d)",IMOWERAPP_MODE_AUTO);

    if (!sendMessage(msg1, sizeof(msg1), result))
    {
        ROS_ERROR("Automower::Failed setting Auto Mode.");
        cuttingDiscOn = lastCuttingDiscOn;
        return false;
    }

    if (startWithoutLoop)
    {
        requestedLoopOn = false;
        eventQueue->raiseEvent("/LOOPDETECTION_CHANGED");
        ROS_INFO("AutoMowerSafe: Loop detection off");
    }

    pauseMower();

    return true;
}
