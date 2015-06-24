#include "PositionMotor.h"
#include <iostream>
#include <gazebo/math/Rand.hh>

namespace gz = gazebo;

namespace revolve {
namespace gazebo {

PositionMotor::PositionMotor(gz::physics::ModelPtr model, std::string partId,
							 std::string motorId, sdf::ElementPtr motor):
	JointMotor(model, partId, motorId, motor, 1),
	minVelocity_(-10e5),
	maxVelocity_(10e5),
	noise_(0) {

	// Retrieve upper / lower limit from joint set in parent constructor
	// Truncate ranges to [-pi, pi]
	upperLimit_ = fmin(M_PI, joint_->GetUpperLimit(0).Radian());
	lowerLimit_ = fmax(-M_PI, joint_->GetLowerLimit(0).Radian());
	fullRange_ = (upperLimit_ - lowerLimit_ + 1e-12) >= (2 * M_PI);

	if (motor->HasElement("rv:pid")) {
		auto pidElem = motor->GetElement("rv:pid");
		pid_ = Motor::createPid(pidElem);
	}

	auto noiseParam = motor->GetAttribute("noise");
	if (noiseParam) {
		noiseParam->Get(noise_);
	}

	if (motor->HasAttribute("min_velocity")) {
		motor->GetAttribute("min_velocity")->Get(minVelocity_);
	}

	if (motor->HasAttribute("max_velocity")) {
		motor->GetAttribute("max_velocity")->Get(maxVelocity_);
	}
}

PositionMotor::~PositionMotor() { }

void PositionMotor::update(double *outputs, double step) {
	// Just one network output, which is the first
	double output = outputs[0];

	// Motor noise in range +/- noiseLevel * actualValue
	output += ((2 * gz::math::Rand::GetDblUniform() * noise_) -
					  noise_) * output;

	// Truncate output to [0, 1]
	output = fmin(fmax(0, output), 1);

	double positionTarget = lowerLimit_ + output * (upperLimit_ - lowerLimit_);
	auto positionAngle = joint_->GetAngle(0);
	positionAngle.Normalize();
	double position = positionAngle.Radian();

	if (fullRange_ && fabs(position - positionTarget) > M_PI) {
		// Both the position and the position target will be in the range [-pi, pi]
		// For a full range of motion joint, using an angle +- 2 PI might result
		// in a much shorter required movement. In this case we best correct the
		// current position to something outside the range.
		position += (position > 0 ? -2 * M_PI : 2 * M_PI);
	}

	gz::common::Time dt(step);
	double cmd = pid_.Update(position - positionTarget, dt);

	// Protection mechanism - don't apply additional force if the joint
	// velocity is already above the given values.
	double velocity = joint_->GetVelocity(0);
	if ((velocity > maxVelocity_ && cmd > 0) || (velocity < minVelocity_ && cmd < 0)) {
		cmd = 0;
	}

	joint_->SetForce(0, cmd);

	if (pid_.GetPGain() > 0) {
		std::cout << position << "/" << positionTarget << ": " << cmd << std::endl;
	}
}

} /* namespace gazebo */
} /* namespace revolve */
