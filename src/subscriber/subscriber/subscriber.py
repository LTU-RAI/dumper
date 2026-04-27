#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
import serial

class Subscriber(Node):
    def __init__(self):
        super().__init__('subscriber')

        self.ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)

        self.subscription = self.create_subscription(
            Twist,
            '/cmd_vel',
            self.twist_to_pwm,
            10)

    def twist_to_pwm(self, msg):
    
        print(f"Received Twist → linear.x: {msg.linear.x:.2f}, angular.z: {msg.angular.z:.2f}, linear.y: {msg.linear.y:.2f}")

        steering = 90 + (msg.angular.z * 45)
        steering = max(45, min(135, int(steering)))

        throttle = 90 + (msg.linear.x * 20)
        throttle = max(70, min(110, int(throttle)))

        dumper = msg.linear.y  # this will be -1.0 … 1.0 exactly
        
        print(f"Converted → Steering PWM: {steering}, Throttle PWM: {throttle}, Dumper PWM: {dumper}")

        command = f"S:{steering},T:{throttle},D:{dumper}\n"
        self.ser.write(command.encode('utf-8'))

def main(args=None):
    rclpy.init(args=args)
    node = Subscriber()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
