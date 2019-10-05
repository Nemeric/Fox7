#include <ros/ros.h>
#include <laser_geometry/laser_geometry.h>
#include <std_msgs/String.h>
#include <std_msgs/Int32.h>
#include <pigpiod_if2.h>
#include <iostream>
#include <cmath>

#include "constantes.hpp"

// Definition de l'aglorithme utilisé, choisir :
// ALGO_RECTANGLE, ALGO_OBSTACLE ou ALGO_PROFIL
#define ALGO_RECTANGLE

#if defined ALGO_RECTANGLE
	#include "algo_rectangle.hpp"
#elif defined ALGO_OBSTACLE
	#include "algo_obstacle.hpp"
#elif defined ALGO_PROFIL
	#include "algo_profil.hpp"
#endif

//#define K2000

// LIDAR
// ranges[0] -> ranges[810]
// centre 405

using namespace std;

float commandSpeed(float dist_max);
void setDirection(float angle);
void setSpeed(float speed);

int _PI;

// pour debug
int n=0;

// Coef d'asserv en direction
float Kp=1;
float Ki=0;
float Kd=0;

// Coef commande vitesse
const float a=(CMD_SPEED_MIN-CMD_SPEED_MAX)/(pow(DISTANCE_MAX-DISTANCE_MIN,2));
const float b=2*DISTANCE_MAX*(CMD_SPEED_MAX-CMD_SPEED_MIN)/(pow(DISTANCE_MAX-DISTANCE_MIN,2));
const float c=(CMD_SPEED_MIN*pow(DISTANCE_MAX,2)-2*DISTANCE_MAX*DISTANCE_MIN*CMD_SPEED_MAX+pow(DISTANCE_MIN,2)*CMD_SPEED_MAX)/(pow(DISTANCE_MAX-DISTANCE_MIN,2));

#if defined K2000
const unsigned int K2000_pins[5] = {5,6,13,12,26};
const int K2000_freq = 10;
const int K2000_cycle = 10;

const int K2000_pattern[K2000_cycle][5] = {
	{1,0,0,0,0},{1,1,0,0,0},{0,1,1,0,0},{0,0,1,1,0},{0,0,0,1,1},
	{0,0,0,0,1},{0,0,0,1,1},{0,0,1,1,0},{0,1,1,0,0},{1,1,0,0,0}
}

void K2000_set_pins(int pattern[5])
{
	for(int i(0);i<5;i++)
	{
		gpio_write(_PI, K2000_pins[i],pattern[i]);
	}
}

void K2000_update()
{
	static int freq_counter = 0;
	static int cycle_counter = 0;
	
	K2000_set_pins(K2000_pattern[cycle_counter]);
	
	freq_counter = (freq_counter+1)%K2000_freq;
	if(freq_counter == 0)
	{
		cycle_counter = (cycle_counter+1)%K2000_cycle;
	}
}

#endif


class AsservDirection
{
public:
	AsservDirection()
		:m_consigne(0),
		m_integrale(0),m_previous_error(0)
	{}

	float operator()(float angle_dist_max, ros::Duration dt)
	{
		if(dt.toSec()==0)
			return 0;

		m_error = m_consigne - angle_dist_max;

		m_derivee = m_error - m_previous_error;
		m_previous_error = m_error;

		m_integrale += m_error;

		m_commande = Kp*m_error + Ki*dt.toSec()*m_integrale + Kd/dt.toSec()*m_derivee;

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

	float m_error;
	float m_consigne;
};

class CmdCallback
{
public:
	CmdCallback()
		:m_dt(0),
		m_run(false), m_arret_urgence(false), m_compteur_urgence(0),
		asservDirection()
	{}

	void callback(const sensor_msgs::LaserScan::ConstPtr& scan_in)
	{
		m_dt=ros::Time::now()-m_last_call;
		m_last_call=ros::Time::now();

		if(m_run)
		{
			#if defined K2000
			K2000_update()
			#endif
			// ALGO
			// renvoit consigne_angle

			m_consigne_angle=commandDirection(scan_in);

			// distance devant ou celle de l'angle de consigne, a voir
			m_dist_max=scan_in->ranges[405];
			//m_dist_max=scan_in->ranges[405+m_consigne_angle/scan_in->angle_increment];
			
			// Arret d'urgence
			if(scan_in->ranges[405]<DIST_URGENCE)
			{
				if(m_compteur_urgence<3)
					++m_compteur_urgence;
				else
					m_arret_urgence=true;
			}

			// commande
			if(!m_arret_urgence)
			{
				m_cmd_angle = asservDirection(m_consigne_angle, m_dt);
				m_cmd_speed = commandSpeed(m_dist_max);

				setDirection(m_cmd_angle);
				setSpeed(m_cmd_speed);
			}
		}

		else
			ROS_INFO("ranges[%d]=%f", n, scan_in->ranges[n]);
	}

	void runON() {m_run=true;}
	void runOFF() {m_run=false;}
	bool running() {return m_run;}
	void initTime() {m_last_call=ros::Time::now();}

	void reset()
	{	
		m_last_call=ros::Time::now();
		m_dt=ros::Duration(0);
		m_run=false;
		m_arret_urgence=false;
		m_compteur_urgence=0;
		asservDirection.reset();
	}


private:
	ros::Time m_last_call;
	ros::Duration m_dt;

	float m_consigne_angle;
	float m_dist_max;
	float m_cmd_angle;
	float m_cmd_speed;

	bool m_run;
	bool m_arret_urgence;
	int m_compteur_urgence;

	AsservDirection asservDirection;
};
CmdCallback cmd_callback;

float commandSpeed(float dist_max)
{
	// genere une commande en vitesse en fonction de la distance max selon une parabole telle que:
	// f(DISTANCE_MIN)=CMD_SPEED_MIN, f(DISTANCE_MAX)=CMD_SPEED_MAX et f'(DISTANCE_MAX)=0

	float speed;

	if(dist_max>DISTANCE_MAX)
		speed = CMD_SPEED_MAX;
	else if(dist_max<DISTANCE_MIN)
		speed = CMD_SPEED_MIN;
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

	angle = angle/CMD_ANGLE_MAX * 1000 + 1500;
	set_servo_pulsewidth(_PI, GPIO_SERVO, angle);
}

void setSpeed(float speed)
{
	//TODO add converstion from speed to ESC plus limitation
	if(speed > CMD_SPEED_MAX)
		speed = CMD_SPEED_MAX;
	if(speed < CMD_SPEED_MIN)
		speed = CMD_SPEED_MIN;
 	
	speed = 1000 * speed + 1500;
	set_servo_pulsewidth(_PI, GPIO_ESC, speed);
}

void control_callback(const std_msgs::String::ConstPtr &msg)
{
	if(msg->data=="Start" && !cmd_callback.running())
		cmd_callback.runON();

	else if(msg->data=="Stop" && cmd_callback.running())
		cmd_callback.runOFF();

	else if(msg->data=="Reset")
		cmd_callback.reset();
}

void n_callback(const std_msgs::Int32::ConstPtr &msg)
{
	n=msg->data;
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

	cmd_callback.initTime();
	
	set_mode(_PI, GPIO_SERVO, PI_OUTPUT);
	set_mode(_PI, GPIO_ESC, PI_OUTPUT);
	
	#if defined K2000
	for(int i(0);i<5;i++){
	set_mode(_PI, K2000_pins[i], PI_OUTPUT);}
	#endif

	ros::Subscriber sub = n.subscribe("scan", 1000, &CmdCallback::callback, &cmd_callback);
	ros::Subscriber sub2 = n.subscribe("control", 10, control_callback);
	ros::Subscriber sub3 = n.subscribe("n", 10, n_callback);
	
	ros::spin();

	pigpio_stop(_PI);

	return 0;
}
