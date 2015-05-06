/*
 * Motor.h
 *
 *  Created on: Mar 5, 2015
 *      Author: elte
 */

#ifndef REVOLVE_GAZEBO_MOTORS_MOTOR_H_
#define REVOLVE_GAZEBO_MOTORS_MOTOR_H_

#include <string>

#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>

#include <revolve/gazebo/Types.h>

namespace revolve {
namespace gazebo {

class Motor {
public:
	Motor(::gazebo::physics::ModelPtr model, std::string partId, unsigned int outputNeurons);
	virtual ~Motor();

	/**
	 * Updates the motor based on the attached output
	 * of the neural network.
	 *
	 * @param Output of the neural network
	 * @param Actuation time in nanoseconds
	 */
	virtual void update(float * networkOutput, unsigned int step) = 0;

	/**
	 * @return The part ID
	 */
	std::string partId();

	/**
	 * @return Number of output neurons connected to this motor
	 */
	unsigned int outputNeurons();

	/**
	 * @param Pointer to the rv:pid element
	 * @return Gazebo PID
	 */
	static ::gazebo::common::PID createPid(sdf::ElementPtr pid);

protected:
	/**
	 * The model this motor is part of
	 */
	::gazebo::physics::ModelPtr model_;

	/**
	 * ID of the body part the motor belongs to
	 */
	std::string partId_;

	/**
	 * Number of output neurons that should be connected
	 * to this motor.
	 */
	unsigned int outputNeurons_;
};

} /* namespace gazebo */
} /* namespace tol_robogen */

#endif /* TOL_ROBOGEN_GAZEBO_MOTORS_MOTOR_H_ */