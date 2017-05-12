/*
* Software License Agreement (BSD License)
* Copyright (c) 2013, Georgia Institute of Technology
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**********************************************
 * @file StateEstimator.h
 * @author Brian Goldfain <bgoldfai@gmail.com>
 * @date May 1, 2017
 * @copyright 2017 Georgia Institute of Technology
 * @brief StateEstimator class definition
 *
 ***********************************************/

#ifndef StateEstimator_H_
#define StateEstimator_H_

#include <gtsam/slam/BetweenFactor.h>
#include <gtsam/slam/PriorFactor.h>
#include <gtsam/navigation/ImuFactor.h>
#include <gtsam/nonlinear/Values.h>
#include <gtsam/inference/Symbol.h>
#include <gtsam/navigation/ImuBias.h>
#include <gtsam/geometry/Pose3.h>
#include <gtsam/nonlinear/GaussNewtonOptimizer.h>
#include <gtsam/base/timing.h>
#include <GeographicLib/LocalCartesian.hpp>
#include <gtsam/nonlinear/ISAM2.h>
#include <gtsam/navigation/GPSFactor.h>

#include <list>
#include <iostream>
#include <fstream>
#include <queue>

#include <ros/ros.h>
#include <ros/time.h>
#include <std_msgs/Float64.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/NavSatFix.h>

#include "autorally_core/Diagnostics.h"
#include "BlockingQueue.h"

#include <autorally_msgs/wheelSpeeds.h>
#include <autorally_msgs/imageMask.h>
#include <autorally_msgs/stateEstimatorStatus.h>
#include <imu_3dm_gx4/FilterOutput.h>
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/Point.h>
#include <visualization_msgs/MarkerArray.h>

#define PI 3.14159265358979323846264338


namespace autorally_core
{
  class StateEstimator : public Diagnostics
  {
  private:
    ros::NodeHandle m_nh;
    ros::Subscriber m_gpsSub, m_imuSub, m_odomSub;
    ros::Publisher  m_posePub;
    ros::Publisher  m_biasAccPub, m_biasGyroPub;
    ros::Publisher  m_timePub;
    ros::Publisher m_statusPub;


    double m_lastImuT, m_lastImuTgps;
    unsigned char m_status;
    double m_AccelBiasSigma, m_GyroBiasSigma;
    double m_gpsSigma;
    int m_maxQSize;

    BlockingQueue<sensor_msgs::NavSatFixConstPtr> m_gpsOptQ;
    BlockingQueue<sensor_msgs::ImuConstPtr> m_ImuOptQ;
    BlockingQueue<nav_msgs::OdometryConstPtr> m_odomOptQ;

    boost::mutex m_optimizedStateMutex;
    gtsam::NavState m_optimizedState;
    double m_optimizedTime;
    boost::shared_ptr<gtsam::PreintegratedImuMeasurements> m_imuPredictor;
    double m_imuDt;
    gtsam::imuBias::ConstantBias m_optimizedBias, m_previousBias;
    sensor_msgs::ImuConstPtr m_lastIMU;
    boost::shared_ptr<gtsam::PreintegrationParams> m_preintegrationParams;

    std::list<sensor_msgs::ImuConstPtr> m_imuMeasurements, m_imuGrav;
    imu_3dm_gx4::FilterOutput m_initialPose;
    gtsam::Pose3 m_bodyPSensor, m_carENUPcarNED;
    gtsam::Pose3 m_imuPgps;

    bool m_fixedOrigin;
    GeographicLib::LocalCartesian m_enu;   /// Object to put lat/lon coordinates into local cartesian
    bool m_gotFirstFix;
    bool m_invertx, m_inverty, m_invertz;
    bool m_usingOdom;
    int m_frequency;
    double m_maxGPSError;
    double m_timeWithoutGPS;

    gtsam::SharedDiagonal priorNoisePose;
    gtsam::SharedDiagonal priorNoiseVel;
    gtsam::SharedDiagonal priorNoiseBias;
    gtsam::SharedDiagonal priorNoiseimuPgps;
    gtsam::Vector3 sigma_acc_bias_c;
    gtsam::Vector3 sigma_gyro_bias_c;
    gtsam::Vector noiseModelBetweenbias_sigma;
    gtsam::SharedDiagonal noiseModelBetweenbias;
    gtsam::ISAM2 *m_isam;

    nav_msgs::OdometryConstPtr m_lastOdom;

  public:
    StateEstimator();
    ~StateEstimator();
    void GpsCallback(sensor_msgs::NavSatFixConstPtr fix);
    void ImuCallback(sensor_msgs::ImuConstPtr imu);
    void WheelOdomCallback(nav_msgs::OdometryConstPtr odom);
    void GpsHelper();
    void GpsHelper_1();
    void diagnosticStatus(const ros::TimerEvent& time);
    gtsam::BetweenFactor<gtsam::Pose3> integrateWheelOdom(double prevTime, double stopTime, int curFactor);
    void GetAccGyro(sensor_msgs::ImuConstPtr imu, gtsam::Vector3 &acc, gtsam::Vector3 &gyro);
  };
};

#endif /* StateEstimator_H_ */