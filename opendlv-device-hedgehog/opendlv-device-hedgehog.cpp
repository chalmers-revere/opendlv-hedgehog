#include <ncurses.h>
#include <poll.h>
#include <thread>
#include <sys/stat.h> // checking if file/dir exist
#include <fstream>
#include <string>

#include "opendlv-standard-message-set.hpp"
#include "automower_hrp.json"
#include "behavior.hpp"

  
}

int32_t main(int32_t argc, char **argv) {
  int32_t retCode{0};
  auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
  if (0 == commandlineArguments.count("cid") || 0 == commandlineArguments.count("freq")) {
    std::cerr << argv[0] << " tests the Hedgehog platform by sending actuation commands and reacting to sensor input." << std::endl;
    std::cerr << "Usage:   " << argv[0] << " --freq=<Integration frequency> --cid=<OpenDaVINCI session> [--verbose]" << std::endl;
    std::cerr << "Example: " << argv[0] << " --freq=10 --cid=111" << std::endl;
    retCode = 1;
  } else {
    bool const VERBOSE{commandlineArguments.count("verbose") != 0};
    uint16_t const CID = std::stoi(commandlineArguments["cid"]);
    uint32_t const FRAME_ID = std::stoi(commandlineArguments["frame-id"]);
   

Behavior behavior

auto onGroundMotionRequest{[&behavior](cluon::data::envelope &&envelope)
{
  if (envelope.senderStamp() == 0){
     auto const groudMotion = cluon::extractMessage<opendlv::proxy::GroundMotionRequest>(std::move(envelope));
     behavior.linearspeed(groudMotion.vx);
     behavior.angularspeed(groundMotion.rollRate);
     
  } else if (envelope.senderStamp() == 9999) {
     auto const gst = cluon::extractMessage<opendlv::proxy::GroundMotionRequest>(std::move(envelope));
     float const groundMotion = gst.groundMotion();
     
     }
  }};
			   
