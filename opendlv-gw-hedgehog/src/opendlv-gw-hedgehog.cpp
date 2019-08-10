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


    
    
    
    
    
  
    
int16_t leftwheelmotor=0; //-100 (max reverse), 100 (max forwards) 
int16_t rightwheelmotor=0; //-100 (max reverse), 100 (max forwards) 

cluon::UDPSender sender{ ; }

auto onPedalPositionRequest{[&VERBOSE, &leftwheelmotor, &rightwheelmotor, &sender](cluon::data::Envelope &&envelope)
      {
        uint32_t const SENDER_STAMP = envelope.senderStamp();
        auto const PEDAL_POSITION_REQUEST=cluon::extractMessage<opendlv::proxy::PedalPositionRequest>(std::move(envelope));
        if ( == SENDER_STAMP) {
          rightwheelmotor = static_cast<int16_t>(PEDAL_POSITION_REQUEST.position() * 100.0);
        } else if ( == SENDER_STAMP) {
          leftwheelmotor = static_cast<int16_t>(PEDAL_POSITION_REQUEST.position() * 100.0);
        }
        
       if (VERBOSE) {
          std::cout << "Sending:" << std::endl;
          std::cout << " .. right wheel request: " << rightwheelmotor << std::endl;
          std::cout << " .. left wheel request: " << leftwheelmotor << std::endl;
        }
       };
  
  cluon::OD4Session od4{CID};
  od4.dataTrigger(opendlv::proxy::PedalPositionRequest::ID(), onPedalPositionRequest);
           
       

