#ifndef BEHAVIOR
#define BEHAVIOR

#include <mutex>

#include "opendlv-standard-message-set.hpp"

class Behavior {
  private:
    Behavior(Behavior const &) = delete;
    Behavior(Behavior &&) = delete;
    Behavior &operator=(Behavior const &) = delete;
    Behavior &operator=(Behavior &&) = delete; 
    
  public:
    Behavior() noexcept;
    ~Behavior() = default;
    
  public:
    opendlv::proxy::GroundSteeringRequest getGroundSteeringAngle() noexcept;
    opendlv::proxy::PedalPositionRequest getPedalPositionRequest() noexcept;
   // void setFrontUltrasonic(opendlv::proxy::DistanceReading const &) noexcept;
   // void setRearUltrasonic(opendlv::proxy::DistanceReading const &) noexcept;
    void setLeftIr(opendlv::proxy::VoltageReading const &) noexcept;
    void setRightIr(opendlv::proxy::VoltageReading const &) noexcept;
    void step() noexcept; 
    
   private:
    //opendlv::proxy::DistanceReading m_frontUltrasonicReading;
    //opendlv::proxy::DistanceReading m_rearUltrasonicReading;
    opendlv::proxy::VoltageReading m_leftIrReading;
    opendlv::proxy::VoltageReading m_rightIrReading;
    opendlv::proxy::GroundSteeringRequest m_groundSteeringAngleRequest;
    opendlv::proxy::PedalPositionRequest m_pedalPositionRequest;
    //std::mutex m_frontUltrasonicReadingMutex;
    //std::mutex m_rearUltrasonicReadingMutex;
    std::mutex m_leftIrReadingMutex;
    std::mutex m_rightIrReadingMutex;
    std::mutex m_groundSteeringAngleRequestMutex;
    std::mutex m_pedalPositionRequestMutex;
};

#endif
    
    
  
  
   
   
   
    
