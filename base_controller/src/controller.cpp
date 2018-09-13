#include <ros/ros.h>
#include <laser_geometry/laser_geometry.h>
#include <std_msgs/String.h>
#include <pigpiod_if2.h>
#include <cstdio>
#include <cmath>

#define L 2
#define PI 3.141592

#define PIN_SERVO 3
#define PIN_ESC 5
/**
 * TODO config this with ros parameters
 */
using namespace std;

int _PI;
bool run = false;

void set_direction(float angle, float angle_max)
{
	//TODO add convertion form angle to servo plus limitation
	if(angle > angle_max)
		angle = angle_max;
	if(angle < -angle_max)
		angle = -angle_max;

	angle = 1000/angle_max * angle + 1500;
	set_servo_pulsewidth(_PI, PIN_SERVO, angle);
}

void set_speed(float speed, float speed_max)
{
	//TODO add converstion from speed to ESC plus limitation
	if(speed > speed_max)
		speed = speed_max;
	if(speed < -speed_max)
		speed = -speed_max;
	
	speed = 1000/speed_max * speed + 1500;	
	
	set_servo_pulsewidth(_PI, PIN_ESC, speed);
}



/**
 * realize speed asserv based on max speed and the distance from
 * the center of the circuit
 */
float asservSpeed(float speed_max, float x, float coef_speed)
{
	return exp(-coef_speed * pow(x, 2)) * speed_max;
}

/**
 * realize angle asserv based on max angle and the distance from
 * the center of the circuit
 */
float asservDirection(float angle_max, float x, float coef_angle)
{
	float a = angle_max/(1 - exp(-coef_angle * pow(L, 2)/4));
	float angle;	

	if(x < 0)
	{
		angle = -a * (1 - exp(-coef_angle * pow(x, 2)));
	}
	else
	{
		angle = a * (1 - exp(-coef_angle * pow(x, 2)));
	}

	return angle;
}

void cmd_callback(const sensor_msgs::LaserScan::ConstPtr& scan_in)
{
	float speed;
	float angle = scan_in->angle_min;
	float increment = scan_in->angle_increment;
	
	float dist_left = 0;
	float dist_right = 0;
	float x;
	int i;

	//TODO add algorythm adrien
	for(int i = 0; i < 10; i++)
	{
		dist_left += scan_in->ranges[313 + i];
	}
	dist_left *= cos(120 * PI / 180.0)/10;
	
	for(int i = 0; i < 10; i++)
	{
		dist_right += scan_in->ranges[495 + i];
	}
	dist_right *= cos(60 * PI / 180.0)/10;

	x = dist_right - dist_left;
	
	if(run)
	{
		set_direction(asservDirection(10, x, 1), 10);
		set_speed(asservSpeed(30, x, 2.3), 30);
	}
	else
	{
		set_direction(0, 10);
		set_speed(0 , 30);
	}
}

void control_callback(const std_msgs::String::ConstPtr &msg)
{
	if(msg->data.c_str() == "Start")
		run = true;
	else
		run = false;
}



int main(int argc, char** argv)
{
	int speed;
	ros::init(argc, argv, "base_controller");
	ros::NodeHandle n;

	_PI = pigpio_start(NULL, NULL);
	
	//TODO change init
	set_mode(_PI, PIN_SERVO, PI_OUTPUT);
	set_mode(_PI, PIN_ESC, PI_OUTPUT);

	ros::Subscriber sub = n.subscribe("scan", 1000, cmd_callback);
	ros::Subscriber sub2 = n.subscribe("control", 10, control_callback);

	ros::spin();

	pigpio_stop(_PI);

	return 0;
}
