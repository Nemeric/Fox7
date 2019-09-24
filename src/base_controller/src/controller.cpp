#include <ros/ros.h>
#include <laser_geometry/laser_geometry.h>
#include <std_msgs/String.h>
#include <pigpiod_if2.h>
//#include <cstdio>
#include <iostream>
#include <cmath>

#define L 2
#define PI 3.141592

#define GPIO_SERVO 17
#define GPIO_ESC 18
#define GPIO_TRIG 23
#define GPIO_ECHO 22

#define Base_Speed 0.055
#define Var_Speed_Max 4
#define Var_Angle_Max 20
#define Ka 4
#define Kv 3
#define marge 0.1
#define divang 3.5

#define ANGLE_MIN 300
#define ANGLE_CENTRE 404
#define ANGLE_MAX 500

/**
 * TODO config this with ros parameters
 */
using namespace std;

int _PI;
bool run =false ; // mettre à false

void set_direction(float angle, float angle_max)
{
	//TODO add convertion form angle to servo plus limitation
	if(angle > angle_max)
		angle = angle_max;
	if(angle < -angle_max)
		angle = -angle_max;

	angle = 1000/angle_max * angle + 1500;
	set_servo_pulsewidth(_PI, GPIO_SERVO, angle);
}

void set_speed(float speed, float speed_max)
{
	//TODO add converstion from speed to ESC plus limitation
	if(speed > speed_max)
		speed = speed_max;
	if(speed < -speed_max)
		speed = -speed_max;
 	
	speed = 1000 * speed + 1500;
	set_servo_pulsewidth(_PI, GPIO_ESC, speed);
}

/***
 * realize speed asserv based on max speed and the distance from
 * the center of the circuit
 */

float asservSpeed(float speed_max, float x, float coef_speed,float dist_center)
{
	return (Base_Speed +exp(-coef_speed *abs(x) / pow(dist_center, 2)) * speed_max/100);
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
		angle = a * (1 - exp(-coef_angle * pow(x, 2)));
	}
	else
	{
		angle = -a * (1 - exp(-coef_angle * pow(x, 2)));
	}

	angle=angle/divang;
	return angle;
}

void cmd_callback(const sensor_msgs::LaserScan::ConstPtr& scan_in)
{
	cout << endl;
	cout << "scan300=" << scan_in->ranges[AGNLE_MIN] << endl;
	cout << "scan404=" << scan_in->ranges[ANGLE_CENTRE] << endl;
	cout << "scan500=" << scan_in->ranges[ANGLE_MAX] << endl;
	
	float acc=0;
	int d=5;
	int imax=0;
	float max=0;
	for(int i=ANGLE_MIN+(d-1)/2; i<ANGLE_MAX-(d-1)/2; i++)
	{
		acc=0;
		for(int j=i-(d-1)/2; j<i+(d-1)/2; j++)
		{
			// Traiter valeur inf
			// > a valeur max = valeur max
			// =0 = valeur max 

			acc+=scan_in->ranges[j];
		}
		if(acc/d>max)
		{
			imax=i;
			max=acc/d;
		}
	}
}

void control_callback(const std_msgs::String::ConstPtr &msg)
{
	if((msg->data.c_str() != "Start")&&(run == false))
	{
		run = true;
	}
	else
	{
		run = false;
	}
}

int main(int argc, char** argv)
{
	ros::init(argc, argv, "base_controller");
	ros::NodeHandle n;

	_PI = pigpio_start(NULL, NULL);
	if(_PI<0)
	{
		ROS_ERROR("Failed to initialize pigpiod _PI=%d", _PI);
		return -1;
	}
	ROS_INFO("Pigpiod initialized _PI=%d", _PI);
	
	//TODO change init
	set_mode(_PI, GPIO_SERVO, PI_OUTPUT);
	set_mode(_PI, GPIO_ESC, PI_OUTPUT);
	
	ros::Subscriber sub = n.subscribe("scan", 1000, cmd_callback);
	ros::Subscriber sub2 = n.subscribe("control", 10, control_callback);
	
	ros::spin();

	pigpio_stop(_PI);

	return 0;
}
