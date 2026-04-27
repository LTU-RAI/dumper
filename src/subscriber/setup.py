from setuptools import setup

package_name = 'subscriber'

setup(
    name=package_name,
    version='0.0.1',
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools', 'rclpy', 'geometry_msgs', 'pyserial'],
    zip_safe=True,
    maintainer='roboticsandai',
    maintainer_email='manika.sharma@ltu.se',
    description='ROS 2 to Arduino serial bridge for truck velocity',
    license='MIT',
    entry_points={
        'console_scripts': [
            'subscriber = subscriber.subscriber:main',
        ],
    },
)
