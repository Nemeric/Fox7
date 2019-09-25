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
#define SPEED_MAX 2.f
#define DISTANCE_MAX 20.f
#define PRECISION_LIDAR 0.3

#define CMD_ANGLE_MAX 800
#define CMD_ANGLE_MIN 0

/**
 * TODO config this with ros parameters
 */
using namespace std;

int _PI;
bool run =false ; // mettre à false

void setDirection(float angle)
{
	//TODO add convertion form angle to servo plus limitation
	if(angle > CMD_ANGLE_MAX)
		angle = CMD_ANGLE_MAX;
	if(angle < CMD_ANGLE_MIN)
		angle = CMD_ANGLE_MIN;

	angle = 1000/CMD_ANGLE_MAX * angle + 1500;
	set_servo_pulsewidth(_PI, GPIO_SERVO, angle);
}

void setSpeed(float speed)
{
	//TODO add converstion from speed to ESC plus limitation
	if(speed > SPEED_MAX)
		speed = SPEED_MAX;
	if(speed < -SPEED_MAX)
		speed = -SPEED_MAX;
 	
	speed = 1000 * speed + 1500;
	set_servo_pulsewidth(_PI, GPIO_ESC, speed);
}

/***
 * realize speed asserv based on max speed and the distance from
 * the center of the circuit
 */

float asservSpeed(float speed)
{
	float c_speed=0;
	return c_speed;
}

/**
 * realize angle asserv based on max angle and the distance from
 * the center of the circuit
 */

float asservDirection(float angle_dist_max)
{
	float angle=0;
	return angle;
}

void cmd_callback(const sensor_msgs::LaserScan::ConstPtr& scan_in)
{
	cout << endl;
	cout << "scan300=" << scan_in->ranges[ANGLE_MIN] << endl;
	cout << "scan404=" << scan_in->ranges[ANGLE_CENTRE] << endl;
	cout << "scan500=" << scan_in->ranges[ANGLE_MAX] << endl;
	
	float acc=0;
	int d=5;
	int imax=0;
	float dist_max=0;
	float valeur=0;
	for(int i=ANGLE_MIN+(d-1)/2; i<ANGLE_MAX-(d-1)/2; i++)
	{
		acc=0;
		for(int j=i-(d-1)/2; j<i+(d-1)/2; j++)
		{
			valeur=scan_in->ranges[j];
			if(valeur>DISTANCE_MAX || valeur==0)
				valeur=DISTANCE_MAX;
				
			acc+=valeur;
		}
		if(acc/d>dist_max)
		{
			imax=i;
			dist_max=acc/d;
		}
	}

	float angle_dist_max=imax*PRECISION_LIDAR;

	float angle=asservDirection(angle_dist_max);
	float speed=asservSpeed(dist_max);
	setDirection(angle);
	setSpeed(speed);
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
