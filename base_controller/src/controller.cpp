#include <ros/ros.h>
#include <laser_geometry/laser_geometry.h>
#include <std_msgs/String.h>
#include <pigpiod_if2.h>
#include <cstdio>
#include <cmath>

#include "SRTED.h"

#define L 2
#define PI 3.141592

#define GPIO_SERVO 12
#define GPIO_ESC 13

#define SPEED_MAX 0.2
#define ANGLE_MAX 20

/**
 * TODO config this with ros parameters
 */
using namespace std;

int _PI;
bool run =false ; // mettre à false
int nbTour=0;

float angle, velocity;

void set_direction(float angle)
{
	//add convertion form angle to servo plus limitation
	if(angle > ANGLE_MAX)
		angle = ANGLE_MAX;
	if(angle < -ANGLE_MAX)
		angle = -ANGLE_MAX;

	angle = 1000/ANGLE_MAX * angle + 1500;	
	//if angle < 1500, La voiture tourne à gauche
	//if angle > 1500, La voiture tourne à droite
	
	//cout<<"Angle = "<<angle<<endl;
	set_servo_pulsewidth(_PI, GPIO_SERVO, angle);
}

void set_speed(float speed)
{
	//add converstion from speed to ESC plus limitation
	if(speed > SPEED_MAX)
		speed = SPEED_MAX;
	if(speed < -SPEED_MAX)
		speed = -SPEED_MAX;
 	
	speed = 1000 * speed + 1500;
	
        //cout<<"Vitesse = "<<speed<<endl;
	set_servo_pulsewidth(_PI, GPIO_ESC, speed);
}

void start_callback(const std_msgs::String::ConstPtr &msg)
{
	if((msg->data.c_str() != "Start")&&(run == false))
	{
		run = true;
		nbTour=0;
	}
	else
	{
		run = false;
	}
}

void command_callback(const geometry_msgs::Twist::ConstPtr &cmd)
{
	angle = ANGLE_MAX * cmd->angular.z;
	velocity = SPEED_MAX * cmd->linear.x;
}

void reg_callback(const ros::TimerEvent& trash)
{
	set_speed(velocity);
	set_direction(angle);
}

int main(int argc, char** argv)
{
	ros::init(argc, argv, "base_controller");
	ros::NodeHandle n;

	_PI = pigpio_start(NULL, NULL);
	
	//change init
 	set_mode(_PI, GPIO_SERVO, PI_OUTPUT);
	set_mode(_PI, GPIO_ESC, PI_OUTPUT);
	
	//sonar = SRTED(_PI, GPIO_TRIG, GPIO_ECHO, sonar_callback);

	ros::Subscriber sub_ctrl = n.subscribe("control", 10, start_callback);
	ros::Subscriber sub_cmd = n.subscribe("cmd",10, command_callback);
	ros::Timer timer_reg = n.createTimer(ros::Duration(0.01), reg_callback);

	
	
	ros::spin();
	
	pigpio_stop(_PI);

	return 0;
}
