#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>
#include <iostream>
#include <chrono>
#include <numbers>
#include <thread>
#include <cmath>

using namespace mavsdk;
using namespace std::this_thread;
using namespace std::chrono;

// Constantes pour la mission de livraison
constexpr uint16_t MAV_CMD_DO_SET_SERVO = 183;
constexpr uint8_t  SERVO_CHANNEL = 9;
constexpr uint16_t SERVO_PWM_DEPLOY = 2000;

// Coordonnées GPS de la cible de largage (Exemple en simulation)
constexpr double TARGET_LAT = -35.362881;
constexpr double TARGET_LON = 149.164137;
constexpr double DROP_RADIUS_METERS = 3.0;

// Fonction Haversine pour calculer la distance entre deux points GPS
double calculate_distance(double lat1, double lon1, double lat2, double lon2) {
    constexpr double R = 6371000.0; // Rayon de la Terre en mètres
    double dLat = (lat2 - lat1) * std::numbers::pi / 180.0;
    double dLon = (lon2 - lon1) * std::numbers::pi / 180.0;
    double a = std::sin(dLat/2) * std::sin(dLat/2) +
               std::cos(lat1 * std::numbers::pi / 180.0) * std::cos(lat2 * std::numbers::pi / 180.0) *
               std::sin(dLon/2) * std::sin(dLon/2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1-a));
    return R * c;
}

int main() {
    Mavsdk mavsdk{Mavsdk::Configuration{Mavsdk::ComponentType::CompanionComputer}};
    
    // Connexion locale via le port UDP de simulation SITL standard
    ConnectionResult conn_res = mavsdk.add_any_connection("udp://127.0.0.1:14540");
    if (conn_res != ConnectionResult::Success) {
        std::cerr << "[-] Échec connexion UDP MAVLink: " << conn_res << std::endl;
        return 1;
    }

    std::cout << "[+] En attente d'un drone sur le réseau..." << std::endl;
    auto system = mavsdk.first_autonomous_system();
    if (!system) {
        std::cerr << "[-] Aucun drone détecté." << std::endl;
        return 1;
    }

    auto telemetry = Telemetry{system.value()};
    auto mavlink_passthrough = MavlinkPassthrough{system.value()};

    std::cout << "[+] Connexion établie. Démarrage de la surveillance de zone..." << std::endl;
    bool payload_deployed = false;

    while (!payload_deployed) {
        Telemetry::Position current_pos = telemetry.position();
        
        double distance = calculate_distance(current_pos.latitude_deg, current_pos.longitude_deg, TARGET_LAT, TARGET_LON);
        std::cout << "[Jetson C++] Dist. Cible: " << distance << "m | Alt: " << current_pos.relative_altitude_m << "m" << std::endl;

        // Condition : À portée de la cible ET à une altitude sécurisée (supérieure à 5m)
        if (distance <= DROP_RADIUS_METERS && current_pos.relative_altitude_m > 5.0) {
            std::cout << "[!!!] ZONE ATTEINTE. Génération du message MAVLink de largage..." << std::endl;

            // Utilisation d'une lambda expression pour encapsuler le paquet MAVLink
            mavlink_passthrough.queue_message([&](mavlink_message_t& message) {
                mavlink_msg_command_long_pack(
                    mavlink_passthrough.get_our_sysid(), 
                    mavlink_passthrough.get_our_compid(),
                    &message,
                    mavlink_passthrough.get_target_sysid(),
                    mavlink_passthrough.get_target_compid(),
                    MAV_CMD_DO_SET_SERVO, 
                    0,                    
                    SERVO_CHANNEL,        
                    SERVO_PWM_DEPLOY,     
                    0, 0, 0, 0, 0         
                );
            });

            std::cout << "[+] Message MAV_CMD_DO_SET_SERVO poussé avec succès." << std::endl;
            payload_deployed = true;
        }

        sleep_for(milliseconds(500)); // Fréquence de calcul : 2Hz
    }

    std::cout << "[+] Mission accomplie, arrêt de l'ordinateur compagnon." << std::endl;
    return 0;
}
