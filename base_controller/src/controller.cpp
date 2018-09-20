#include <ros/ros.h>
#include <laser_geometry/laser_geometry.h>
#include <std_msgs/String.h>
#include <pigpiod_if2.h>
#include <cstdio>
#include <cmath>

#define L 2
#define PI 3.141592

#define GPIO_SERVO 17
#define GPIO_ESC 18
/**
 * TODO config this with ros parameters
 */
using namespace std;

int _PI;
bool run = true; // mettre à false

void set_direction(float angle, float angle_max)
{
	//TODO add convertion form angle to servo plus limitation
	if(angle > angle_max)
		angle = angle_max;
	if(angle < -angle_max)
		angle = -angle_max;

	angle = 1000/angle_max * angle + 1500;

	cout<<"Angle = "<<angle<<endl;

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
	
        cout<<"Vitesse = "<<speed<<endl;
	set_servo_pulsewidth(_PI, GPIO_ESC, speed);
}



/***
 * realize speed asserv based on max speed and the distance from
 * the center of the circuit
 */


float asservSpeed(float speed_max, float x, float coef_speed)
{
	return (0.07+exp(-coef_speed * pow(x, 2)) * speed_max/100);
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
	float MUR = 0;
	float x;
	int i;

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
	
	for(int i = 0; i < 10; i++)
	{
		dist_center += scan_in->ranges[399 +i];
	}
	dist_center = dist_center/10;
	
	if (dist_center<0.3)
	{
		MUR=0;	
	}
	else
	{
		MUR=1;
	}		
	
	x = dist_right - dist_left;
	
	cout<<"Distance milieu ="<<dist_center<<endl;
	cout<<"Erreur = "<< x <<endl;
        // cout Debug tool only
	float vlim = 9;
	float thetalim = 10;
       	if(run)
        {
	  set_direction(MUR*asservDirection(thetalim, x, 5), thetalim);// 10 angle max en degre
	  set_speed(MUR*asservSpeed(vlim, x, 2.3), vlim);// 30 pourcentage de la vitesse maximale possible
        }
	else
	{
	        cout<<"STOP"<<endl;
		set_direction(0, thetalim);
		set_speed(0 , vlim);
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
	cout << "Début main" << endl;
	int speed;
	ros::init(argc, argv, "base_controller");
	ros::NodeHandle n;

	cout<<"debut\n"<<endl;

	_PI = pigpio_start(NULL, NULL);
	
	//TODO change init
	set_mode(_PI, GPIO_SERVO, PI_OUTPUT);
	set_mode(_PI, GPIO_ESC, PI_OUTPUT);

	ros::Subscriber sub = n.subscribe("scan", 1000, cmd_callback);
	ros::Subscriber sub2 = n.subscribe("control", 10, control_callback);

	ros::spin();

	pigpio_stop(_PI);

	return 0;
}
