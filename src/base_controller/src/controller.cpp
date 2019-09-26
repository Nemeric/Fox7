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

class AsservSpeed
{
public:
	AsservSpeed(float Kp, float Ki, float Kd)
		:m_previous_dist(0), m_integrale(0),
		m_Kp(Kp), m_Ki(Ki), m_Kd(Kd)
	{}

	float operator()(float dist_max, ros::Duration dt)
	{
		
	}

private:
	float m_previous_dist;	
	float m_integrale;
	const float m_Kp, m_Ki, m_Kd;
};

class AsservDirection
{
public:
	AsservDirection(float Kp, float Ki, float Kd)
		:m_previous_dist(0), m_integrale(0),
		m_Kp(Kp), m_Ki(Ki), m_Kd(Kd)
	{}

	float operator()(float dist_max, ros::Duration dt)
	{
		
	}

private:
	float m_previous_dist;	
	float m_integrale;
	const float m_Kp, m_Ki, m_Kd;
};

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

class CmdCallback
{
public:
	CmdCallback()
		:m_last_call(ros::Time::now()), m_dt(0),
		asservSpeed(1,1,1), asservDirection(1,1,1),
		m_d(5)
	{}

	void callback(const sensor_msgs::LaserScan::ConstPtr& scan_in)
	{
		m_dt=ros::Time::now()-m_last_call;
		m_last_call=ros::Time::now();

		m_imax=0;
		m_dist_max=0;
		m_angle_dist_max=0;
		
		float valeur=0;
		// moyenne glissante
		for(int i=ANGLE_MIN+(m_d-1)/2; i<ANGLE_MAX-(m_d-1)/2; i++)
		{
			m_acc=0;
			for(int j=i-(m_d-1)/2; j<i+(m_d-1)/2; j++)
			{
				// Verif si valeur pas aberante
				valeur=scan_in->ranges[j];
				if(valeur>DISTANCE_MAX || valeur==0)
					valeur=DISTANCE_MAX;
					
				m_acc+=valeur;
			}
			if(m_acc/m_d>m_dist_max)
			{
				m_imax=i;
				m_dist_max=m_acc/m_d;
			}
		}

		m_angle_dist_max=m_imax*PRECISION_LIDAR;

		m_cmd_angle = asservDirection(m_angle_dist_max, m_dt);
		m_cmd_speed = asservSpeed(m_dist_max, m_dt);
		setDirection(m_cmd_angle);
		setSpeed(m_cmd_speed);
	}
private:
	ros::Time m_last_call;
	ros::Duration m_dt;

	float m_acc;
	float m_dist_max;
	int m_imax;
	int m_d;
	float m_cmd_angle;
	float m_cmd_speed;
	float m_angle_dist_max;

	AsservSpeed asservSpeed;
	AsservDirection asservDirection;
};

/*void control_callback(const std_msgs::String::ConstPtr &msg)
{
	if((msg->data.c_str() != "Start")&&(run == false))
	{
		run = true;
	}
	else
	{
		run = false;
	}
}*/

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

	CmdCallback cmd_callback;

	ros::Subscriber sub = n.subscribe("scan", 1000, &CmdCallback::callback, &cmd_callback);
	//ros::Subscriber sub2 = n.subscribe("control", 10, control_callback);
	
	ros::spin();

	pigpio_stop(_PI);

	return 0;
}
