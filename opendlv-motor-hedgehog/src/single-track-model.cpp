#include <cmath>
#include <iostream>

#include "single-track-model.hpp"

SingleTrackModel::SingleTrackModel() noexcept:
  m_groundSteeringAngleMutex{},
  m_pedalPositionMutex{},
  m_longitudinalSpeed{0.0f},
  m_lateralSpeed{0.0f},
  m_yawRate{0.0f},
  m_groundSteeringAngle{0.0f},
  m_pedalPosition{0.0f}
{
}

void SingleTrackModel::setGroundSteeringAngle(opendlv::proxy::GroundSteeringRequest const &groundSteeringAngle) noexcept
{
  std::lock_guard<std::mutex> lock(m_groundSteeringAngleMutex);
  m_groundSteeringAngle = groundSteeringAngle.groundSteering();
}

void SingleTrackModel::setPedalPosition(opendlv::proxy::PedalPositionRequest const &pedalPosition) noexcept
{
  std::lock_guard<std::mutex> lock(m_pedalPositionMutex);
  m_pedalPosition = pedalPosition.position();
}

opendlv::sim::KinematicState SingleTrackModel::step(double dt) noexcept
{
  double const pedalSpeedGain{0.5};

  double const mass{1.0};
  double const momentOfInertiaZ{0.1};
  double const length{0.22};
  double const frontToCog{0.11};
  double const rearToCog{length - frontToCog};
  double const corneringStiffnessFront{1.0};
  double const corneringStiffnessRear{1.0};
  
  float groundSteeringAngleCopy;
  float pedalPositionCopy;
  {
    std::lock_guard<std::mutex> lock1(m_groundSteeringAngleMutex);
    std::lock_guard<std::mutex> lock2(m_pedalPositionMutex);
    groundSteeringAngleCopy = m_groundSteeringAngle;
    pedalPositionCopy = m_pedalPosition;
  }

  m_longitudinalSpeed = pedalPositionCopy * pedalSpeedGain;

  if (std::abs(m_longitudinalSpeed) > 0.01f) {
    double const slipAngleFront = groundSteeringAngleCopy 
      - (m_lateralSpeed + frontToCog * m_yawRate) 
      / std::abs(m_longitudinalSpeed);
    double const slipAngleRear = (rearToCog * m_yawRate - m_lateralSpeed) 
      / std::abs(m_longitudinalSpeed);

    double const lateralSpeedDot = (corneringStiffnessFront * slipAngleFront 
        + corneringStiffnessRear * slipAngleRear)
      / (mass - m_longitudinalSpeed * m_yawRate);

    double const yawRateDot = (frontToCog * corneringStiffnessFront * slipAngleFront 
        - rearToCog * corneringStiffnessRear * slipAngleRear)
      / momentOfInertiaZ;
    
    m_lateralSpeed += lateralSpeedDot * dt;
    m_yawRate += yawRateDot * dt;
  } else {
    m_lateralSpeed = 0.0f;
    m_yawRate = 0.0f;
  }

  opendlv::sim::KinematicState kinematicState;
  kinematicState.vx(static_cast<float>(m_longitudinalSpeed));
  kinematicState.vy(static_cast<float>(m_lateralSpeed));
  kinematicState.yawRate(static_cast<float>(m_yawRate));

  return kinematicState;
}
