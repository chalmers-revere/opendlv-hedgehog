#include "behavior.hpp"

Behavior::Behavior() noexecpt:
  //m_frontUltrasonicReading{},
  //m_rearUltrasonicReading{},
  m_leftIrReading{},
  m_rightIrReading{},
  m_groundSteeringAngleRequest{},
  m_pedalPositionRequest{},
  //m_frontUltrasonicReadingMutex{},
  //m_rearUltrasonicReadingMutex{},
  //m_leftIrReadingMutex{},
  //m_rightIrReadingMutex{},
  //m_groundSteeringAngleRequestMutex{},
  //m_pedalPositionRequestMutex{}
{
}

void Behavior::setLeftIr(opendlv::proxy::VoltageReading const &leftIrReading) noexcept
{
  std::lock_guard<std::mutex> lock(m_leftIrReadingMutex);
  m_leftIrReading = leftIrReading;
}

void Behavior::setRightIr(opendlv::proxy::VoltageReading const &rightIrReading) noexcept
{
  std::lock_guard<std::mutex> lock(m_rightIrReadingMutex);
  m_rightIrReading = rightIrReading;
}

  
 

