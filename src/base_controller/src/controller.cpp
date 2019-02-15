#include <ros/ros.h>
#include <laser_geometry/laser_geometry.h>
#include <std_msgs/String.h>
#include <pigpiod_if2.h>
#include <cstdio>
#include <cmath>

#include "SRTED.h"

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

/**
 * TODO config this with ros parameters
 */
using namespace std;

int _PI;
bool run =false ; // mettre à false
int nbTour=0;

SRTED_t *sonar;	

void set_direction(float angle, float angle_max)
{
	//TODO add convertion form angle to servo plus limitation
	if(angle > angle_max)
		angle = angle_max;
	if(angle < -angle_max)
		angle = -angle_max;

	angle = 1000/angle_max * angle + 1500;
	
	//cout<<"Angle = "<<angle<<endl;

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
	
        //cout<<"Vitesse = "<<speed<<endl;
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
	float speed;
	float angle = scan_in->angle_min;
	float increment = scan_in->angle_increment;

	float dist_left = 0;
	float dist_right = 0;
	float dist_center = 0;
	float x;
	int i;

	//cout<<"nbTour="<<nbTour<<endl;
	cout<<"run="<<run<<endl;

	for(int i = 0; i < 10; i++)
	{
		dist_left += scan_in->ranges[313 + i];
		//cout << "Distance gauche ="<<dist_left<<endl;
	}
	dist_left *= cos(60 * PI / 180.0)/10;

	for(int i = 0; i < 10; i++)
	{
		dist_right += scan_in->ranges[495 + i];
	}
	dist_right *= cos(60 * PI / 180.0)/10;
	
	for(int i = 0; i < 30; i++)
	{
		dist_center += scan_in->ranges[389 +i];
	}
	dist_center = dist_center/30;
	
	x = dist_right - dist_left;
	if(abs(x)<marge)
	{
		x=0;
	}
	//cout<<"x="<<x<<endl;
	
	
       	if(run==true && dist_center>=0.3 )
        {
	  set_direction(asservDirection(Var_Angle_Max, x, Ka), Var_Angle_Max);// 10 angle max en degre
	  set_speed(asservSpeed(Var_Speed_Max, x, Kv,dist_center), Var_Speed_Max);// 30 pourcentage de la vitesse maximale possible
	//cout<<"fonction normale"<<endl;        
}
	else
	{
		set_direction(0, Var_Angle_Max);
		set_speed(0 , Var_Speed_Max);
		//cout<<"arret demandé"<<endl;	
	}


	SRTED_manual_read(sonar);
}

void control_callback(const std_msgs::String::ConstPtr &msg)
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

void sonar_callback(SRTED_data_t r)
{
	static int arche=0;
	static int tour=0;
	cout << "DEBUG RANGE : " << r.range_cms << "STATUS :" << r.status << endl;

	if((r.range_cms < 100)&&(r.range_cms>0)&&(r.status==0))
	{
		arche++;
	}
	else
	{
		arche=0;
		tour=0;
	}	

	if(arche > 1)
	{
		if(tour==0)
			nbTour+=1;
		tour=1;
	}

	if(nbTour>1)
	{
		run=false;
	}
	cout<<"Arche ="<<arche<<endl;
	cout<<"Tour ="<<nbTour<<endl;
}

int main(int argc, char** argv)
{
	ros::init(argc, argv, "base_controller");
	ros::NodeHandle n;


	_PI = pigpio_start(NULL, NULL);
	
	//TODO change init
	set_mode(_PI, GPIO_SERVO, PI_OUTPUT);
	set_mode(_PI, GPIO_ESC, PI_OUTPUT);
	
	SRTED_config(sonar, 0, 150);
	sonar = SRTED(_PI, GPIO_TRIG, GPIO_ECHO, sonar_callback);

	ros::Subscriber sub = n.subscribe("scan", 1000, cmd_callback);
	ros::Subscriber sub2 = n.subscribe("control", 10, control_callback);
	
	
	
	ros::spin();

	SRTED_cancel(sonar);
	
	pigpio_stop(_PI);

	return 0;
}
