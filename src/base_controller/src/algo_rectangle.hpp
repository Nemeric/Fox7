#include <iostream>
#include <cmath>
#include <ros/ros.h>
#include <laser_geometry/laser_geometry.h>

#include "constantes.hpp"

float commandDirectionRectangle(const sensor_msgs::LaserScan::ConstPtr& scan_in)
{
	float consigne_angle=0; // [rad]

	/*
	-> cherche Dmax, angle max
		-> fixe une distance max ou on voit
		-> trouver imax
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

	return consigne_angle;
}

