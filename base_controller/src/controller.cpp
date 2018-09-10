#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <pigpiod_if2.h>
#include <cstdio>
#include <cmath>

#define PWM_0 12 //18
#define PWM_1 13 //19
/**
 * TODO config this with ros parameters
 */
using namespace std;

int PI;

void set_direction(int angle)
{
	//TODO add convertion form angle to servo plus limitation
	set_PWM_dutycycle(PI, PIN_SERVO, angle);
}

void set_speed(int speed)
{
	//TODO add converstion from speed to ESC plus limitation
	set_PWM_dutycycle(PI, PIN_ESC, speed);
}

}

void cmd_callback(const sensor_msgs::LaserScan::ConstPtr& scan_in)
{
	int angle;
	float speed;
	float angle = scan_in->angle_min;
	float increment = scan_in->scan_in->increment;

	//TODO add algorythm adrien
	

	set_direction(angle);
	set_speed(speed);
}

int main(int argc, char** argv)
{
	int speed;
	ros::init(argc, argv, "base_controller");
	ros::NodeHandle n;

	PI = pigpio_start(NULL, NULL);
	
	//TODO change init
	set_mode(PI, PIN_IN_1, PI_OUTPUT);
	set_mode(PI, PIN_IN_2, PI_OUTPUT);
	set_mode(PI, PIN_IN_3, PI_OUTPUT);
	set_mode(PI, PIN_IN_4, PI_OUTPUT);
	set_PWM_frequency(PI, PIN_EN_A, 100000);
	set_PWM_frequency(PI, PIN_EN_B, 100000);

	ros::Subscriber sub = n.subscribe("scan", 1000, cmd_callback);

	ros::spin();

	pigpio_stop(PI);

	return 0;
}
