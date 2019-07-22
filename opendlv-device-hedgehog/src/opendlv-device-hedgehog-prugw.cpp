#include <ncurses.h>
#include <poll.h>
#include <thread>
#include <sys/stat.h> // checking if file/dir exist
#include <fstream>
#include <string>


#include "cluon-complete.hpp"
#include "opendlv-standard-message-set.hpp"

#include "PwmMotors.h"

void write2file(std::string const &a_path, std::string const &a_str)
{
  std::ofstream file(a_path, std::ofstream::out);
  if (file.is_open()) {
    file << a_str;
  } else {
    std::cerr << " Could not open " << a_path << "." << std::endl;
    exit(1);
  }    
  file.flush();
  file.close();
}

int32_t main(int32_t argc, char **argv) {
  auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
  if (0 == commandlineArguments.count("cid") ||
      0 == commandlineArguments.count("names") ||
      0 == commandlineArguments.count("types") ||
      0 == commandlineArguments.count("channels") ||
      0 == commandlineArguments.count("offsets") ||
      0 == commandlineArguments.count("maxvals") ||
      0 == commandlineArguments.count("angleconversion")) {
    std::cerr << argv[0] << " interfaces to the motors of the Kiwi platform." << std::endl;
    std::cerr << "Usage:   " << argv[0] << " --cid=<OpenDaVINCI session> --names=<Strings> --types=<esc or servo> --channels=<1...8>  --offsets<-1...1> --maxvals=<floats> --angleconversion=<const> [--verbose]" << std::endl;
    std::cerr << "Example: " << argv[0] << " --cid=111 --names=steering,propulsion --types=servo,esc --channels=1,2 --offsets=0,0 --maxvals=0.5,0 --angleconversion=1" << std::endl;
    return 1;
  } else {

    // Setup
    int32_t VERBOSE{commandlineArguments.count("verbose") != 0};
    if (VERBOSE) {
      VERBOSE = std::stoi(commandlineArguments["verbose"]);
    }
    float const FREQ = 20;

    std::vector<std::string> names = stringtoolbox::split(commandlineArguments["names"],',');
    std::vector<std::string> types = stringtoolbox::split(commandlineArguments["types"],',');
    std::vector<std::string> channels = stringtoolbox::split(commandlineArguments["channels"],',');
    std::vector<std::string> offsets = stringtoolbox::split(commandlineArguments["offsets"],',');
    std::vector<std::string> maxvals = stringtoolbox::split(commandlineArguments["maxvals"],',');
    float const angleConversion = std::stof(commandlineArguments["angleconversion"]);

    if (names.empty())
    {
      names.push_back(commandlineArguments["names"]);
      types.push_back(commandlineArguments["types"]);
      channels.push_back(commandlineArguments["channels"]);
      offsets.push_back(commandlineArguments["offsets"]);
      maxvals.push_back(commandlineArguments["maxvals"]);
    }

    if (names.size() != types.size() ||
        names.size() != types.size() ||
        names.size() != channels.size() ||
        names.size() != offsets.size() ||
        names.size() != maxvals.size()) {
      std::cerr << "Number of arguments do not match, use ',' as delimiter." << std::endl;
      return 1;
    }

    // PwmMotors pwmMotors(names, types, channels, offsets, maxvals);
    std::shared_ptr<PwmMotors> pwmMotors = std::make_shared<PwmMotors>(names, types, channels, offsets, maxvals);

    auto onGroundSteeringRequest{[&pwmMotors, &angleConversion](cluon::data::Envelope &&envelope)
    {
      if (envelope.senderStamp() == 0){
        auto const gst = cluon::extractMessage<opendlv::proxy::GroundSteeringRequest>(std::move(envelope));
        float const groundSteering = gst.groundSteering() / angleConversion;
        pwmMotors->setMotorPower(1, groundSteering);
      } else if (envelope.senderStamp() == 9999) {
        auto const gst = cluon::extractMessage<opendlv::proxy::GroundSteeringRequest>(std::move(envelope));
        float const groundSteering = gst.groundSteering();
        pwmMotors->setMotorOffset(1, groundSteering);
      }
    }};
    auto onPedalPositionRequest{[&pwmMotors](cluon::data::Envelope &&envelope)
    {
      opendlv::proxy::PedalPositionRequest const ppr = cluon::extractMessage<opendlv::proxy::PedalPositionRequest>(std::move(envelope));
      float val = (ppr.position()+1)/2.0f;
      if (val > 1.0f) {
        val = 1.0f;
      } else if (val < 0.1f){
        val = 0.1f;
      }
      pwmMotors->setMotorPower(2, val);
    }};

    cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"]))};
    od4.dataTrigger(opendlv::proxy::GroundSteeringRequest::ID(), onGroundSteeringRequest);
    od4.dataTrigger(opendlv::proxy::PedalPositionRequest::ID(), onPedalPositionRequest);

