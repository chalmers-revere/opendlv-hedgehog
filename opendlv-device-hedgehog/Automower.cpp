#include "automower.hpp"

AutomowerSafe::AutomowerSafe() :
{

m_linearSpeedRequest;
m_angularSpeedRequest;


}
  
void Automower::RequestLinearSpeed(opendlv::proxy::GroundMotionRequest const &linearSpeedRequest) noexcept
{
m_linearSpeedRequest=linearSpeedRequest
}

void Automower::RequestAngularSpeed(opendlv::proxy::GroundMotionRequest const &angularSpeedRequest) noexcept
{
m_angularSpeedRequest=angularSpeedRequest
}



int AutomowerSafe::sendMessage(unsigned char *msg, int len, unsigned char *ansmsg, int maxAnsLength, bool retry)
{
 /*
 	std::cout << "SEND: " << std::hex;
	for (int i=0; i<len; i++)
	{
		std::cout << "0x" << (int)msg[i] << " ";
	}
	std::cout << std::dec << std::endl;
*/	
	
	// Sending
	int cnt=0;
	cnt = write(serialFd, msg, len);
	
	if(cnt != len)
	{
		std::cerr << "ERROR : AutomowerHIL::Could not send on serial port!" << std::endl;
		exit(1);
	}

	cnt = 0;
	int res;
	int payloadLength = 0;
	
	// Clear answer buffer
	memset(ansmsg, 0, maxAnsLength);
	
	// Keep reading until we find an STX
	
	// receive STX
	while (ansmsg[cnt] != 0x02)
	{
		res = read(serialFd, &ansmsg[cnt], 1);
	}
	cnt++;

	// Read MESSAGE TYPE
	res = read(serialFd, &ansmsg[cnt], 1);
	//std::cout << "MESSAGE TYPE: " << (int)ansmsg[cnt] << std::endl;
	cnt++;
	
	// Read LENGTH
	res = read(serialFd, &ansmsg[cnt], 1);
	payloadLength = ansmsg[cnt];
	//std::cout << "PAYLOAD LENGTH: " << (int)payloadLength << std::endl;
	cnt++;
	
	// Read PAYLOAD
	unsigned char readBytes = 0;
	unsigned max_retries = 100;
	while ((readBytes < payloadLength) && (max_retries > 0))
	{
		int bytes = read(serialFd, &ansmsg[cnt], payloadLength-readBytes);
		if (bytes > 0)
		{
			readBytes += bytes;
			cnt += bytes;
		}
		max_retries--;
	}
	
	// Read CRC
	// TODO: Use it!
	res = read(serialFd, &ansmsg[cnt], 1);
	cnt++;

	// Read ETX
	res = read(serialFd, &ansmsg[cnt], 1);
	cnt++;

/*
	std::cout << "CNT: " << cnt << std::endl;
	std::cout << "RESPONSE: " << std::hex;
	for (int i=0; i<cnt; i++)
	{
		std::cout << "0x" << (int)ansmsg[i] << " ";
	}
	std::cout << std::dec << std::endl;
*/

	// Check some stuff
	if (ansmsg[0] != 0x02)
	{
		// FAILED
		return 0;
	}
	if (ansmsg[cnt-1] != 0x03)
	{
		// FAILED
		return 0;
	}

	return cnt;

}   


std::string AutomowerSafe::loadJsonModel(std::string fileName)
{
    std::string line;
    std::ifstream file(fileName.c_str());
    std::cout << "Loading from:" << fileName << "..." << std::endl;

    std::string str;
    std::string fileContents;
    if (file.is_open())
    {
        while (std::getline(file, str))
        {
            fileContents += str;
            fileContents.push_back('\n');
        }
        return fileContents;
    }
    else
    {
        std::cerr << "Could not load JSON file!!!" << std::endl;
        return str;
    }
}

bool AutomowerSafe::initAutomowerBoard()
{
  std::cout << "AutomowerSafe::initAutomowwerBoard" << std::endl;
  
  
void AutomowerSafe::velocityCallback(float const & )
{
    velocityRegulator = true;

    lin_vel = (double)vel->linear.x;
    ang_vel = (double)vel->angular.z;

    wanted_lv = lin_vel - ang_vel * AUTMOWER_WHEEL_BASE_WIDTH / 2;
    wanted_rv = lin_vel + ang_vel * AUTMOWER_WHEEL_BASE_WIDTH / 2;
    

AutomowerSafe::stopWheels()
{

    //DEBUG_LOG ("AutomowerSafe::stopWheels()")
    // Clear power
    
    power_l = 0;
    power_r = 0;

//    wheelPower.left = power_l;
//    wheelPower.right = power_r;

    hcp_tResult result;
    const char* powerOffMsg = "Wheels.PowerOff()";
    if (!sendMessage(powerOffMsg, sizeof(powerOffMsg), result))
    {
        return;
    }

    return;
}






void AutomowerSafe::sendWheelPower(double power_left, double power_right)
{
    if (userStop)
    {
        std::cerr << "User stop active, can't set power" << std::endl ;
        stopWheels();
        return;
    }

    if (power_l > 100)
    {
        power_l = 100;
    }

    if (power_l < -100)
    {
        power_l = -100;
    }

    if (power_r > 100)
    {
        power_r = 100;
    }

    if (power_r < -100)
    {
        power_r = -100;
    }

    // Send it out...
    hcp_tResult result;
    char powerMsg[100];
    snprintf(powerMsg, sizeof(powerMsg), "HardwareControl.WheelMotorsPower(leftWheelMotorPower:%d, rightWheelMotorPower:%d)", power_l, power_r);
    if (!sendMessage(powerMsg, sizeof(powerMsg), result))
    {
        std::cerr << "Can't set power, unknown reason" << std::endl;
        return;
    }
}

