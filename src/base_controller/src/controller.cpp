#include <ros/ros.h>
#include <laser_geometry/laser_geometry.h>
#include <std_msgs/String.h>
#include <pigpiod_if2.h>
#include <iostream>
#include <cmath>

#define GPIO_SERVO 17
#define GPIO_ESC 18

#define ANGLE_MIN 300
#define ANGLE_CENTRE 404
#define ANGLE_MAX 500
#define PRECISION_LIDAR 0.3

#define CMD_ANGLE_MAX 800
#define CMD_ANGLE_MIN 0

#define SPEED_MAX 0.8
#define SPEED_MIN 0.1
#define DISTANCE_MAX 20.0
#define DISTANCE_MIN 0.2
#define DIST_URGENCE 0.1

using namespace std;

float commandSpeed(float dist_max);
void setDirection(float angle);
void setSpeed(float speed);

int _PI;

float Kp=1;
float Ki=1;
float Kd=1;

const float a=(SPEED_MIN-SPEED_MAX)/(pow(DISTANCE_MAX-DISTANCE_MIN,2));
const float b=2*DISTANCE_MAX*(SPEED_MAX-SPEED_MIN)/(pow(DISTANCE_MAX-DISTANCE_MIN,2));
const float c=(SPEED_MIN*pow(DISTANCE_MAX,2)-2*DISTANCE_MAX*DISTANCE_MIN*SPEED_MAX+pow(DISTANCE_MIN,2)*SPEED_MAX)/(pow(DISTANCE_MAX-DISTANCE_MIN,2));

class AsservDirection
{
public:
	AsservDirection(float Kp_p, float Ki_p, float Kd_p)
		:m_consigne(0),
		m_integrale(0),m_previous_error(0),
		m_Kp(Kp_p), m_Ki(Ki_p), m_Kd(Kd_p )
	{}

	float operator()(float angle_dist_max, ros::Duration dt)
	{
		if(dt.toSec()==0)
			return 0;

		m_error = m_consigne - angle_dist_max;

		m_derivee = m_error - m_previous_error;
		m_previous_error = m_error;

		m_integrale += m_error;

		m_commande = m_Kp*m_error + m_Ki*dt.toSec()*m_integrale + m_Kd/dt.toSec()*m_derivee;

		return m_commande;
	}

	void reset()
	{
		m_consigne=0;
		m_integrale=0;
		m_previous_error=0;
	}

private:
	float m_commande;

	float m_previous_error;	

	float m_derivee;
	float m_integrale;
	const float m_Kp, m_Ki, m_Kd;

	float m_error;
	float m_consigne;
};

class CmdCallback
{
public:
	CmdCallback()
		:m_last_call(ros::Time::now()), m_dt(0),
		m_run(false), m_arret_urgence(false),
		asservDirection(Kp,Ki,Kd),
		m_d(5)
	{}

	void callback(const sensor_msgs::LaserScan::ConstPtr& scan_in)
	{
		m_dt=ros::Time::now()-m_last_call;
		m_last_call=ros::Time::now();

		if(m_run)
		{
			m_imax=0;
			m_dist_max=0;
			m_angle_dist_max=0;
			
			// moyenne glissante
			float valeur=0;
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

			ROS_INFO("\nAngleMax=%f\nDistMax=%f", m_dist_max, m_angle_dist_max);

			if(m_dist_max<DIST_URGENCE)
				m_arret_urgence=true;

			if(!m_arret_urgence)
			{
				m_cmd_angle = asservDirection(m_angle_dist_max, m_dt);
				m_cmd_speed = commandSpeed(m_dist_max);

				setDirection(m_cmd_angle);
				setSpeed(m_cmd_speed);
			}
		}
	}

	void runON()
	{
		m_run=true;
	}

	void runOFF()
	{
		m_run=false;
	}

	bool running()
	{
		return m_run;
	}

	void reset()
	{	
		m_last_call=ros::Time::now();
		m_dt=ros::Duration(0);
		m_run=false;
		m_arret_urgence=false;
		asservDirection.reset();
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

	bool m_run;
	bool m_arret_urgence;

	AsservDirection asservDirection;
};
CmdCallback cmd_callback;

float commandSpeed(float dist_max)
{
	float speed;

	if(dist_max>DISTANCE_MAX)
		speed = SPEED_MAX;
	else if(dist_max<DISTANCE_MIN)
		speed = SPEED_MIN;
	else
		speed = a*pow(dist_max,2)+b*dist_max+c;

	return speed;
}

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

void control_callback(const std_msgs::String::ConstPtr &msg)
{
	if(msg->data.c_str()=="Start" && !cmd_callback.running())
		cmd_callback.runON();

	else if(msg->data.c_str()=="Stop" && cmd_callback.running())
		cmd_callback.runOFF();

	else if(msg->data.c_str()=="Reset")
		cmd_callback.reset();
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
	
	set_mode(_PI, GPIO_SERVO, PI_OUTPUT);
	set_mode(_PI, GPIO_ESC, PI_OUTPUT);

	ros::Subscriber sub = n.subscribe("scan", 1000, &CmdCallback::callback, &cmd_callback);
	ros::Subscriber sub2 = n.subscribe("control", 10, control_callback);
	
	ros::spin();

	pigpio_stop(_PI);

	return 0;
}
