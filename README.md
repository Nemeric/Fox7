# Fox7

## Présentation

Fox7 est la voiture du club 7Robot, club informatique de l'ENSEEIHT pour participer à la Toulouse Robot Race.

## Setup
Pour utiliser ce code, il vous faudra:

* Une raspberry pi 3 (non testé avec un autre mdoèle, le wifi est requis)
* Une voiture de modélisme (Nous utilisons une Desert Fox de chez Hobbyking)
* Une carte d'alimentation
* Un lidar (Nous utilisons un Sick TIM 561)

### Instalation de la raspberry pi
Par simplicité, nous utilisons une distribution linux intégrant déjà ROS. Elle est téléchargeable (ici)[https://downloads.ubiquityrobotics.com/pi.html].
Pour installer l'image sur la carte sd:

```
$ sudo dd if=sdimage.img of=/dev/carte-sd
```

En renplaçant sdimage.img par le fichier téléchargé et carte-sd par le prériphérique correspondant à votre carte SD.

Pour vous connecter la première fois, vous pouvez utiliser le Wifi `ubiquityrobotXXXX` avec le mot de passe `robotseverywhere`.
Une fois connecté au Wifi, il est possible de se connecter en ssh:

```
$ ssh ubuntu@10.42.0.1
ubuntu@10.42.0.1's password: ubuntu
```

Il est aussi possible de se connecter en utilisant un écran et un clavier.

### Configuration de la raspberry pi
Pour renommer la machine:

```
$ sudo echo "new-name">/etc/hostname
$ sudo /etc/init.d/hostname.sh start
```

Pour changer le mot de passe par défaut:

```
$ passwd
```

Il est fortement déconseillé de changer le nom d'utilisateur.

Pour configurer le Wifi, modifier le fichier *default_ap.em*. Le champ "ssid" permet de régler le nom du Wifi, et le champ "psk" permet de choisir le mot de passe du Wifi.

Pour des raisons de performance, il est possible de désinstaller des logiciels et de supprimer des servies actifs au démarrage.

### Installation de Fox7
Pour le site web, il faut créer un lien symbolique dans /var/www vers html:

```
$ sudo ln -s html /var/www/html
```

**TODO** Ajouter une configuration propre du site web

Pour le démarrage automatique, copier le fichier fox7.service dans /etc/systemd/system/ puis activer le service:

```
$ sudo cp fox7.service /etc/systemd/system/
$ sudo systemctl enable fox7.service
```

Attention après cette étape, le programme s'éxécute automatiquement au démarrage.

Il faut aussi copier le script fox7 dans /usr/sbin

```
$ sudo cp fox7 /usr/sbin
```

Pour compiler les sources, copier le dossier src dans ~/catkin_ws/src:


```
$ cp -r src ~/catkin_ws/src && cd ~/catkin_ws
$ catkin_make --pkg base_controller
```

Pour lancer le programme (attention, il se lance automatiquement au démarrage après les étapes précédentes):

```
$ roslaunch fox7_bringup minimal.launch
```

