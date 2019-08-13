#include "cluon-complete.hpp"
#include "opendlv-standard-message-set.hpp"

#include <linux/joystick.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>


int32_t main(int32_t argc, char **argv)
{
  int32_t retCode{0};
  auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
  if ( (0 == commandlineArguments.count("cid")) ||
     (0 == commandlineArguments.count("device")) ||
     (0 == commandlineArguments.count("freq")) ||
     (0 == commandlineArguments.count("axis-left-updown")) ||
     (0 == commandlineArguments.count("axis-right-updown")) ) {
   std::cerr << argv[0] << " interfaces with the given PS3 controller to emit ActuationRequest messages to an OD4Session." << std::endl;
   retCode = 1;
 
  }
  else {
    const int32_t MIN_AXES_VALUE = -32768;
    const uint32_t MAX_AXES_VALUE = 32767;
    
    const bool VERBOSE{commandlineArguments.count("verbose") != 0};
    const uint8_t AXIS_LEFT_UPDOWN = std::stoi(commandlineArguments["axis-left-updown"]); //std::stoi convert string type to integer type
    const uint8_t AXIS_RIGHT_UPDOWN = std::stoi(commandlineArguments["axis-right-updown"]);
    const std::string DEVICE{commandlineArguments["device"]};
    
    const float FREQ = std::stof(commandlineArguments["freq"]); //convert string type to float type

    //This first part of the code just get data and command from the joystick : it manages the case of error and just recover axis, device and freq data. 
    
    int gamepadDevice;
    if ( -1 == (gamepadDevice = ::open(DEVICE.c_str(), O_RDONLY)) ) {
      std::cerr << "[opendlv-device-gamepad]: Could not open device: " << DEVICE << ", error: " << errno << ": " << strerror(errno) << std::endl;
    }
    else {
      int num_of_axes{0};
      int num_of_buttons{0};
      char name_of_gamepad[80];
      
   // Then this part initialize the variable which will be used in the next part to recover data from joystick move.
   // pragma controls implementation-specific behavior of the compiler : disabling compiler warnings , turn on or off some features.
  
#pragma GCC diagnostic push // Enable to remember the state of the diagnostics at each push 
#pragma GCC diagnostic ignored "-Woverflow" 
      ::ioctl(gamepadDevice, JSIOCGAXES, &num_of_axes); //ioctl() manipulates device parameters of special files as terminals: 1st argument is an open file descriptor, 2nd is a device-request code and 3rd is a pointer to memory
      ::ioctl(gamepadDevice, JSIOCGBUTTONS, &num_of_buttons);
#pragma GCC diagnostic pop //restore it at each pop
      if (::ioctl(gamepadDevice, JSIOCGNAME(80), &name_of_gamepad) < 0) {
        ::strncpy(name_of_gamepad, "Unknown", sizeof(name_of_gamepad));
      }
      std::clog << "[opendlv-device-gamepad]: Found " << std::string(name_of_gamepad) << ", number of axes: " << num_of_axes << ", number of buttons: " << num_of_buttons << std::endl;

      // Use non blocking reading.
      fcntl(gamepadDevice, F_SETFL, O_NONBLOCK);
      //fcntl() manipulate file descriptor , 1st argument is the file descriptor, 2nd determine the commmand 
      //It Sets the file status flags to the value specified by arg (3rd variable)
      
      std::mutex valuesMutex; //std::mutex is used to synchronize and protect shared data from being used simultaneously by multiple threads
      float left{0};
      float right{0};
      bool hasError{false}; 
      
      //Here we just initialize the main variable which will make move the lawnmover : right and left motors

      // Thread to read values.
      std::thread gamepadReadingThread([&AXIS_LEFT_UPDOWN,
          &AXIS_RIGHT_UPDOWN,
          &MIN_AXES_VALUE,
          &MAX_AXES_VALUE,
          &VERBOSE,
          &valuesMutex,
          &left,
          &right,
          &hasError,
          &gamepadDevice]() {
          struct timeval timeout {};
          fd_set setOfFiledescriptorsToReadFrom{}; //select() allow a program to monitor multiple file descriptors, waiting until one or more of the file descriptors become "ready" for some class of input/output operation
          // fd_set() adds a file descriptor from a set
        
          while (!hasError) {
            // Define timeout for select system call. The timeval struct must be
            // reinitialized for every select call as it might be modified containing
            // the actual time slept.
            timeout.tv_sec  = 0;
            timeout.tv_usec = 20 * 1000; // Check for new data with 50Hz.

            FD_ZERO(&setOfFiledescriptorsToReadFrom); //fd_zero clears a set : to initialize here
            FD_SET(gamepadDevice, &setOfFiledescriptorsToReadFrom);
            ::select(gamepadDevice + 1, &setOfFiledescriptorsToReadFrom, nullptr, nullptr, &timeout);

            if (FD_ISSET(gamepadDevice, &setOfFiledescriptorsToReadFrom)) {
              std::lock_guard<std::mutex> lck(valuesMutex);

            // select function is set on line 104. When the select statement exits, line 107 check which file descriptor has been active and if a file descriptor is part of the set. Then it makes some mutex managment
              
              
              struct js_event js; //joystick device
              while (::read(gamepadDevice, &js, sizeof(struct js_event)) > 0) { //reading joystick device
                float percent{0};
                switch (js.type & ~JS_EVENT_INIT) {
                  case JS_EVENT_AXIS:
                    {
                      if (AXIS_LEFT_UPDOWN == js.number) {
                        percent = static_cast<float>(js.value - MIN_AXES_VALUE)/static_cast<float>(MAX_AXES_VALUE-MIN_AXES_VALUE);
                        left = 1.0f - 2.0f * percent;
                      }

                      if (AXIS_RIGHT_UPDOWN == js.number) {
                        percent = static_cast<float>(js.value-MIN_AXES_VALUE)/static_cast<float>(MAX_AXES_VALUE-MIN_AXES_VALUE);
                        right = 1.0f - 2.0f * percent;
                      }
                      break;
                    }
                  case JS_EVENT_BUTTON:
                    break;
                  case JS_EVENT_INIT:
                    break;
                  default:
                    break;
                }
              }
              if (errno != EAGAIN) {
                std::cerr << "[opendlv-device-gamepad]: Error: " << errno << ": " << strerror(errno) << std::endl;
                hasError = true;
              }
            }
          }
          });
    
      // Explanation of joystick js : js_event corresponds to what happens, if you press a button JS_EVENT_BUTTON or turn the joystick JS_EVENT_AXIS
      // Then the values of js.number correspond to the axis or button that generated the event. There is a specific number for one axis. It's just a way to know in which direction/axis we put the joystick
      // Finally js.value is a signed integer between -32767and 32767 which recover the postion of the joystick.
      
      // Then the next code just manages and sends data to the lawnmover via standard-messages. We have to convert these messages into hrp-json messages which are the messages understable by hedgehog
      
      cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"]))};
      if (od4.isRunning()) {
        od4.timeTrigger(FREQ, [&VERBOSE, &valuesMutex, &left, &right, &hasError, &od4](){
            std::lock_guard<std::mutex> lck(valuesMutex);


            opendlv::proxy::PedalPositionRequest pprl;
            pprl.position(left);
            od4.send(pprl, cluon::time::now(), 0);

            opendlv::proxy::PedalPositionRequest pprr;
            pprr.position(right);
            od4.send(pprr, cluon::time::now(), 10);

            return !hasError;
            });

        opendlv::proxy::PedalPositionRequest ppr;
        ppr.position(0.0);
        od4.send(ppr, cluon::time::now(), 0);
        od4.send(ppr, cluon::time::now(), 10);
      }

      {
        std::lock_guard<std::mutex> lck(valuesMutex);
        hasError = true;
        gamepadReadingThread.join();
      }

      ::close(gamepadDevice);
      retCode = 0;
    }
  }
  return retCode;
}
