"""
This file specifies motor types, which through XML tags are passed
to the Gazebo model plugin. In order to control the motor, it needs
to be recognized by a motor controller in the C++ model plugin.
When writing a new motor, this often involves creating a corresponding
C++ motor class.
"""
from sdfbuilder import Element
from sdfbuilder.joint import Joint
from sdfbuilder.util import number_format as nf


class Motor(Element):
    """
    Plugin motor base class, used to communicate motor configuration through
    the SDF plugin to the model controller in Gazebo.
    """
    # SDF tag name, should not be changed in subclass
    TAG_NAME = 'rv:motor'

    # Override in child class with the adequate motor type,
    # alternatively specify `motor_type` in constructor.
    MOTOR_TYPE = 'motor'

    def __init__(self, part_id, joint=None, motor_type=None):
        """
        :param part_id: ID of the part this motor belongs to. This is required in the XML to identify
                        the corresponding output neuron.
        :type part_id: str
        :param joint: It is common for motors to power a joint, this allows you to specify
                      which joint the motor controls if this is the case.
        :type joint: Joint
        :param motor_type:
        :type motor_type: str
        """
        super(Motor, self).__init__()
        self.joint = joint
        self.part_id = part_id
        self.type = motor_type

    def render_attributes(self):
        """
        Adds motor type to attributes
        """
        attrs = super(Motor, self).render_attributes()
        attrs.update({
            'type': self.type if self.type is not None else self.MOTOR_TYPE,
            'part_id': self.part_id
        })

        if self.joint is not None:
            attrs['joint'] = self.joint.name

        return attrs


class PID(Element):
    """
    PID element for motors that can have a specified PID
    controller. This matches Gazebo's PID controller.
    The PID controller should be a child of
    a motor element for it to be picked up by the
    plugin.
    """
    TAG_NAME = 'rv:pid'

    def __init__(self, proportional_gain=0.0, integral_gain=0.0, derivative_gain=0.0,
                 integral_max=None, integral_min=None, cmd_max=None, cmd_min=None):
        """
        :param proportional_gain:
        :param integral_gain:
        :param derivative_gain:
        :param integral_max:
        :param integral_min:
        :param cmd_max:
        :param cmd_min:
        """
        super(PID, self).__init__()
        self.p = proportional_gain
        self.i = integral_gain
        self.d = derivative_gain
        self.i_max = integral_max
        self.i_min = integral_min
        self.cmd_max = cmd_max
        self.cmd_min = cmd_min

    def render_elements(self):
        sub = []
        for attr in ['p', 'i', 'd', 'i_max', 'i_min', 'cmd_max', 'cmd_min']:
            v = getattr(self, attr, None)
            if v is not None:
                sub.append(Element(tag_name='rv:'+attr, body=nf(v)))

        return super(PID, self).render_elements() + sub


class PIDMotor(Motor):
    """
    General class for a motor controlled by
    a PID. The C++ code contains logic to use this PID to
    control either a joint's position or its velocity using
    this PID controller. To model for instance a servo which
    can exert a certain maximum force, add these force limits
    to the joint being controlled.
    """
    # PID motors have a general C++ controller
    MOTOR_TYPE = 'pid'

    def __init__(self, part_id, joint, pid=None, motor_type=None):
        """
        :param part_id:
        :param joint:
        :type joint: Joint
        :param pid:
        :type pid: PID
        """
        super(PIDMotor, self).__init__(part_id, joint=joint, motor_type=motor_type)
        self.pid = PID() if pid is None else pid

    def render_elements(self):
        """
        :return:
        """
        return super(PIDMotor, self).render_elements() + [self.pid]


class PositionMotor(PIDMotor):
    """
    A position based PID motor. The positional limits
    of this motor are generally derived from the
    corresponding joint.
    """
    MOTOR_TYPE = 'position'


class VelocityMotor(PIDMotor):
    """
    A velocity based PDI motor
    """
    MOTOR_TYPE = 'velocity'

    def __init__(self, part_id, joint, pid=None, motor_type=None, min_velocity=-100, max_velocity=100):
        """

        :param min_velocity:
        :param max_velocity:
        :return:
        """
        super(VelocityMotor, self).__init__(part_id, joint, pid, motor_type)
        self.min_velocity = min_velocity
        self.max_velocity = max_velocity

    def render_attributes(self):
        """
        :return:
        """
        attrs = super(VelocityMotor, self).render_attributes()
        attrs.update({'min_velocity': nf(self.min_velocity), 'max_velocity': nf(self.max_velocity)})
        return attrs
