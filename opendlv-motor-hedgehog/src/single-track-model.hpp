#ifndef SINGLE_TRACK_MODEL
#define SINGLE_TRACK_MODEL

#include <mutex>

#include "opendlv-standard-message-set.hpp"

class SingleTrackModel {
 private:
  SingleTrackModel(SingleTrackModel const &) = delete;
  SingleTrackModel(SingleTrackModel &&) = delete;
  SingleTrackModel &operator=(SingleTrackModel const &) = delete;
  SingleTrackModel &operator=(SingleTrackModel &&) = delete;

 public:
  SingleTrackModel() noexcept;
  ~SingleTrackModel() = default;

 public:
  void setGroundSteeringAngle(opendlv::proxy::GroundSteeringRequest const &) noexcept;
  void setPedalPosition(opendlv::proxy::PedalPositionRequest const &) noexcept;
  opendlv::sim::KinematicState step(double) noexcept;

 private:
  std::mutex m_groundSteeringAngleMutex;
  std::mutex m_pedalPositionMutex;
  double m_longitudinalSpeed;
  double m_lateralSpeed;
  double m_yawRate;
  float m_groundSteeringAngle;
  float m_pedalPosition;
};

#endif 
