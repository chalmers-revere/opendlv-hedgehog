#include <iostream>
#include <thread>
#include <cstring>

#include <zmq.h>
#include <json.hpp>

#include "cluon-complete.hpp"
#include "opendlv-standard-message-set.hpp"

int32_t main(int32_t argc, char **argv)
{
  int32_t retCode{0};
  auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
  if (0 == commandlineArguments.count("cid") || 0 == commandlineArguments.count("port") || 0 == commandlineArguments.count("http-root")) {
    std::cerr << argv[0] << " is the the graphical interface for the OpenDLV simulation environment." << std::endl;
    std::cerr << "Usage:   " << argv[0] << " --cid=<libcluon session> --port=<the port where HTTP/WebSocket is served> --http-root=<folder where HTTP content can be found> [--id=<Identifier in case of multiple running instances>] [--verbose]" << std::endl;
    std::cerr << "Example: " << argv[0] << " --cid=111 --port=8000 --http-root=./http" << std::endl;
    retCode = 1;
  } else {
    bool const VERBOSE{commandlineArguments.count("verbose") != 0};
    uint16_t const CID = static_cast<uint16_t>(std::stoi(commandlineArguments["cid"]));

    uint32_t const HTTP_PORT = static_cast<uint32_t>(std::stoi(commandlineArguments["port"]));
    std::string const HTTP_ROOT = commandlineArguments["http-root"];
    
    std::string const SSL_CERT_PATH{(commandlineArguments["ssl-cert-path"].size() != 0) ? commandlineArguments["ssl-cert-path"] : ""};
    std::string const SSL_KEY_PATH{(commandlineArguments["ssl-key-path"].size() != 0) ? commandlineArguments["ssl-key-path"] : ""};

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
           
       

