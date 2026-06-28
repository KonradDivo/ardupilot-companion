# Autonomous UAV Companion Computer Control (C++ / ArduPilot Lua)

Ce dépôt contient l'architecture logicielle complète pour un drone autonome industriel (Nvidia Jetson + Pixhawk). Le système utilise **C++17 (MAVSDK)** pour la logique de mission géolocalisée et un script **Lua réentrant** pour la sécurité à bord du contrôleur de vol.

## Spécifications Techniques
- **Langages :** C++17 (Ordinateur compagnon), Lua (Firmware de vol)
- **Protocole :** MAVLink v2.0
- **Framework de communication :** MAVSDK-C++
- **Système de Build :** CMake

---

## Guide de Validation en Environnement de Simulation (SITL)

### 1. Prérequis Système
Installez la bibliothèque de développement MAVSDK :
```bash
sudo apt-get update
sudo apt-get install libmavsdk-dev
```

### 2. Déploiement et Démarrage d'ArduPilot SITL
Lancez le simulateur officiel d'ArduPilot configuré pour héberger des scripts :
```bash
cd ardupilot/ArduCopter
sim_vehicle.py --vehicle ArduCopter --console --map --speedup 1
```

Injectez le script de sécurité dans l'environnement virtuel :
```bash
cp drone_scripts/delivery_mission.lua ~/.ardupilot/autotest/ArduCopter/scripts/
```
Activez le moteur de script dans la console MAVProxy :
```text
param set SCR_ENABLE 1
param set SCR_HEAP_SIZE 60000
reboot
```

### 3. Compilation et Exécution de l'Application C++ (Nvidia Jetson)
Dans un nouveau terminal, compilez le projet à l'aide de CMake :
```bash
mkdir build && cd build
cmake ..
make
```

Décollez manuellement ou via script, puis lancez le binaire C++. L'application se connectera automatiquement à la simulation via le port `14540` et calculera la distance GPS en temps réel :
```bash
./offboard_control
```

*Dès que le drone simulé passe à moins de 3 mètres des coordonnées cibles (-35.362881, 149.164137), le programme C++ injecte la commande `MAV_CMD_DO_SET_SERVO` pour libérer la charge utile.*
