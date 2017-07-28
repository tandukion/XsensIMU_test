/*	Copyright (c) 2003-2016 Xsens Technologies B.V. or subsidiaries worldwide.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1.	Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

	2.	Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the documentation
		and/or other materials provided with the distribution.

	3.	Neither the names of the copyright holders nor the names of their contributors
		may be used to endorse or promote products derived from this software without
		specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
	THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
	OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
	HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR
	TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <xsens/xsportinfoarray.h>
#include <xsens/xsdatapacket.h>
#include <xsens/xstime.h>
#include <xcommunication/legacydatapacket.h>
#include <xcommunication/int_xsdatapacket.h>
#include <xcommunication/enumerateusbdevices.h>

#include "deviceclass.h"

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>

#ifdef __GNUC__
#include "conio.h" // for non ANSI _kbhit() and _getch()
#else
#include <conio.h>
#endif


/********************************************************************/
// init_IMU
// Desc  : initialization of Xsens IMU by opening port with baudrate
// Input : - portName : port of the Xsens IMU (default: /dev/ttyUSB0)
//         - baudRate
// Output: - *device : updated DeviceClass for the Xsens
//         - *mtPort : updated Port name and baudrate
/********************************************************************/
void init_IMU(DeviceClass *device, XsPortInfo *mtPort, char *portName, int baudRate){

  XsPortInfoArray portInfoArray;

  XsPortInfo portInfo(portName, XsBaud::numericToRate(baudRate));
  portInfoArray.push_back(portInfo);

  // Use the first detected device
  *mtPort = portInfoArray.at(0);

  // Open the port with the detected device
  std::cout << "Opening port..." << std::endl;
  device->openPort(*mtPort);
}


/********************************************************************/
// config_IMU
// Desc  : Configure the Xsens output mode (check manual or library)
//         Enter Config State then return to Measurement State
// Input : - outputMode
//         - outputSettings
// Output: - *device : updated DeviceClass for the Xsens
//         - *mtPort : updated Port name and baudrate
/********************************************************************/
void config_IMU(DeviceClass *device, XsPortInfo *mtPort, XsOutputMode outputMode, XsOutputSettings outputSettings){

  // Put the device in configuration mode
  std::cout << "Putting device into configuration mode..." << std::endl;
  device->gotoConfig();

  // Request the device Id to check the device type
  mtPort->setDeviceId(device->getDeviceId());

  // Print information about detected MTi / MTx / MTmk4 device
  std::cout << "Found a device with id: " << mtPort->deviceId().toString().toStdString() << " @ port: " << mtPort->portName().toStdString() << ", baudrate: " << mtPort->baudrate() << std::endl;
  std::cout << "Device: " << device->getProductCode().toStdString() << " opened." << std::endl;


  // Configure the device. Note the differences between MTix and MTmk4
  std::cout << "Configuring the device..." << std::endl;
  if (mtPort->deviceId().isMt9c() || mtPort->deviceId().isLegacyMtig())
    {
      /* Default Mode configuration */
      // XsOutputMode outputMode = XOM_Orientation; // output orientation data
      // XsOutputSettings outputSettings = XOS_OrientationMode_Quaternion; // output orientation data as quaternion

      // set the device configuration
      device->setDeviceMode(outputMode, outputSettings);
    }
  else if (mtPort->deviceId().isMtMk4() || mtPort->deviceId().isFmt_X000())
    {
      XsOutputConfiguration quat(XDI_Quaternion, 100);
      XsOutputConfigurationArray configArray;
      configArray.push_back(quat);
      device->setOutputConfiguration(configArray);
    }


  // Put the device in measurement mode
  std::cout << "Putting device into measurement mode..." << std::endl;
  device->gotoMeasurement();

}

/********************************************************************/
// measure_IMU
// Desc  : Measurement State, getting the data from Xsens
// Input : - device : updated DeviceClass for the Xsens
//         - mtPort : updated Port name and baudrate
//         - outputMode
//         - outputSettings
// Output: - quaternion
//         - euler
/********************************************************************/

void measure_IMU(DeviceClass *device, XsPortInfo *mtPort, XsQuaternion *quaternion, XsEuler *euler){

  XsByteArray data;
  XsMessageArray msgs;
  bool foundAck = false;

  do {
    device->readDataToBuffer(data);
    device->processBufferedData(data, msgs);

    for (XsMessageArray::iterator it = msgs.begin(); it != msgs.end(); ++it)
      {
	// Retrieve a packet
	XsDataPacket packet;
	if ((*it).getMessageId() == XMID_MtData) {
    printf("MTData\n");
	  LegacyDataPacket lpacket(1, false);
	  lpacket.setMessage((*it));
	  lpacket.setXbusSystem(false);
	  lpacket.setDeviceId(mtPort->deviceId(), 0);
	  lpacket.setDataFormat(XOM_Orientation, XOS_OrientationMode_Quaternion,0);//lint !e534
	  XsDataPacket_assignFromLegacyDataPacket(&packet, &lpacket, 0);
	  foundAck = true;
	}
	else if ((*it).getMessageId() == XMID_MtData2) {
    printf("MTData2\n");
	  packet.setMessage((*it));
	  packet.setDeviceId(mtPort->deviceId());
	  foundAck = true;
	}

	// Get the quaternion data
	*quaternion = packet.orientationQuaternion();

	// Convert packet to euler
	*euler = packet.orientationEuler();
      }
  } while (!foundAck);

}


/*********************************************************************************************/

int main(int argc, char* argv[])
{
	DeviceClass device;
	XsPortInfo mtPort;
	XsQuaternion quaternion;
	XsEuler euler;

	// Xsens Configuration
	char portName[20] = "/dev/ttyUSB0";
	int baudRate = 921600;
	XsOutputMode outputMode = XOM_Orientation; // output orientation data
	XsOutputSettings outputSettings = XOS_OrientationMode_Quaternion; // output orientation data as quaternion

	init_IMU(&device,&mtPort,portName,baudRate);
	config_IMU(&device,&mtPort, outputMode, outputSettings);

	/**/
	std::cout << "Looping Printing by accessing function each time.." << std::endl;
	while(1)
	  {
	  measure_IMU(&device,&mtPort,&quaternion,&euler);
	  std::cout  << "\r"
		    << "W:" << std::setw(5) << std::fixed << std::setprecision(2) << quaternion.w()
		    << ",X:" << std::setw(5) << std::fixed << std::setprecision(2) << quaternion.x()
		    << ",Y:" << std::setw(5) << std::fixed << std::setprecision(2) << quaternion.y()
		    << ",Z:" << std::setw(5) << std::fixed << std::setprecision(2) << quaternion.z()
	    ;
	  std::cout << ",Roll:" << std::setw(7) << std::fixed << std::setprecision(2) << euler.roll()
		    << ",Pitch:" << std::setw(7) << std::fixed << std::setprecision(2) << euler.pitch()
		    << ",Yaw:" << std::setw(7) << std::fixed << std::setprecision(2) << euler.yaw()
		   ;
	  }
	return 0;
}
