#include <iostream>
#include <cmath>
#include <ros/ros.h>
#include <laser_geometry/laser_geometry.h>

#include "constantes.hpp"

float calculOuverture(float dmax)
{
	float ouverture;
	// a dvp evidemment
	ouverture=45*PI/180;

	return ouverture;
}

float genererRectangle(float angle, float angle_ecart, float* longueur_rectangle, const sensor_msgs::LaserScan::ConstPtr& scan_in)
{
	//trouve dmin dans plage [angle-angle_ecart; angle+angle_ecart]
	float dmin=scan_in->ranges[INDICE_CENTRE+(angle-angle_ecart)/scan_in->angle_increment]; // longueur_rectangle
	for(float angle_plage=angle-angle_ecart+scan_in->angle_increment; angle_plage<angle+angle_ecart; angle_plage+=scan_in->angle_increment)
	{
		if(scan_in->ranges[INDICE_CENTRE+angle_plage/scan_in->angle_increment]<dmin)
			dmin=scan_in->ranges[INDICE_CENTRE+scan_in->angle_increment];
	}
	*longueur_rectangle=dmin;

	// calcule largeur_rectangle
	return 2*dmin*tan(angle_ecart);
}

float commandDirection(const sensor_msgs::LaserScan::ConstPtr& scan_in)
{
	float consigne_angle=0; // [rad]

	/*
	-> cherche Dmax
		-> fixe une distance max ou on voit
		-> trouver dmax
		-> recuperer tous indices supérieurs a un pourcentage de la distance max
		-> centre de cette zone (moyenne), on obtient meilleur angle max

	-> determiner angle d'ouverture en fonction de Dmax
		-> au plus distance courte au plus angle grand

	-> pour chaque angle dans la zone d'ouverture (ou avec un pas defini) trouver rectangle max
		-> generer rectangle (i_angle_ecart=1)
			-> trouver distance min dans plage (formee par angle ecart autour de l'angle)
			-> calculer largeur_plage en fonction de dmin et angle_ecart

		-> tant que largeur_plage<largeur_voiture (ou autre condition de fin au cas ou ?)
			-> agrandir plage/angle_ecart
			-> generer rectangle

		-> si ce rectangle est le plus grand qu'on ai calculé jusque la
			-> long_rectangle_max
			-> imax et donc rectangle_angle_max

	-> on vise la direction en rectangle_angle_max
	*/

	// cherche dmax
	float dmax=scan_in->ranges[0];
	float dist=0;
	for(int i=INDICE_MIN+1; i<INDICE_MAX; i++)
	{
		// on borne distance vu
		dist=scan_in->ranges[i];
		if(scan_in->ranges[i]>DISTANCE_MAX)
			dist=DISTANCE_MAX;

		// cherche max
		if(dist>dmax)
			dmax=scan_in->ranges[i];
	}

	// on fait la moyenne sur un certain pourcentage
	int pourcentage=95;
	int imoymax=0;
	int nbi=0;
	for(int i=INDICE_MIN; i<INDICE_MAX; i++)
	{
		if(scan_in->ranges[i]>dmax*pourcentage/100)
		{
			imoymax+=i;
			++nbi;
		}
	}
	imoymax=imoymax/nbi;
	float angle_moy_max=(INDICE_CENTRE-imoymax)*scan_in->angle_increment;

	// Determine angle d'ouverture
	float angle_ouverture=calculOuverture(dmax);

	// Pour chaque angle dans l'ouverture on trouve le rectangle associé dans le but de trouver le rectangle le plus long
	float angle_ecart=scan_in->angle_increment;
	float angle_rectangle_max=-angle_ouverture;
	float largeur_rectangle=0;
	float longueur_rectangle=0;
	float longueur_rectangle_max=0;
	for(float angle=angle_moy_max-angle_ouverture; angle<angle_moy_max+angle_ouverture; angle+=scan_in->angle_increment)
	{
		// generer 1er rectangle
		largeur_rectangle=genererRectangle(angle, angle_ecart, &longueur_rectangle, scan_in);
		// on continue de l'agrandir jusqu'a ce que sa largeur depasse celle de la voiture
		while(largeur_rectangle<LARGEUR_VOITURE)
		{
			angle_ecart+=scan_in->angle_increment;
			largeur_rectangle=genererRectangle(angle, angle_ecart, &longueur_rectangle, scan_in);
		}

		// on verif si on a trouve le plus long jusque la
		if(longueur_rectangle>longueur_rectangle_max)
		{
			longueur_rectangle_max=longueur_rectangle;
			angle_rectangle_max=angle;
		}
	}

	// on a maintenant la direction a viser
	consigne_angle=angle_rectangle_max;

	return consigne_angle;
}

