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
		ROS_ERROR("AutomowerHIL::Could not send on serial port!");
		return -1;
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
