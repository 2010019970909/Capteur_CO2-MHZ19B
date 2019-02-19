#include <SoftwareSerial.h>

// Définition de la fonction qui retourne la concentration en CO2 en ppm
uint16_t getCO2(void);

// Création d'un port série software/logiciel (opposé à un port hardware/matériel)
SoftwareSerial Serial2(6, 7); // Rx/Tx (Arduino) . Tx/Rx

void setup()
{
  // Initialisation du port software/logiciel et hardware/matériel
  Serial2.begin(9600);
  Serial.begin(9600);

  // Intervalle 0 - 5000 au début
  Serial.println(String(0) + '\n' + String(5000));
}

void loop()
{
  // Définition des variables qui vont stocker la valeur mesurée et la dernière valeur différente
  uint16_t old_CO2_val = 0;
  uint16_t temp_CO2_val = 0;

  // Lecture de la valeur de la concentration en CO2 en ppm/ Lecture de la valeur du capteur
  temp_CO2_val = getCO2();

  // On n'affiche et on stocke la valeur lue que si elle est différente que old_CO2_val
  if (temp_CO2_val != old_CO2_val) {
    // Stocke la valeur
    old_CO2_val = temp_CO2_val;
    // Affiche la valeur
    Serial.println(temp_CO2_val);
  }
}

uint16_t getCO2(void)  {
  // Valeurs des bytes de la trame
  const int Start      = 0xFF;
  const int Sensor     = 0x01;
  const int Command    = 0x86;
  const int Zero       = 0x00;
  const int Checksum   = 0x79;

  // Trame pour demander la concentration en CO2
  byte request[9] = {
    Start,
    Sensor,
    Command,
    Zero,
    Zero,
    Zero,
    Zero,
    Zero,
    Checksum
  };

  // Variable; tableau de 9 bytes pour stocker la réponse du capteur
  byte response[9];

  // Envoie de la trame pour demander la concentration en CO2 en ppm
  Serial2.write(request, 9);

  // On attend que le buffer du port série (Serial2) soit vide (action bloquante)
  Serial2.flush();

  // On attend la réponse du capteur
  while (Serial2.available() && Serial2.peek() != 0xFF) {
    Serial2.read();
    delay(1);
  }

  // Lit la réponse et on la stocke dans le tableau 'response'
  Serial2.readBytes(response, 9);
  
  // Vérifie si la trame est correct (commence bien par '0xFF' et se termine par '0x86')
  if (response[0] != 0xFF && response[1] != 0x86)
  {
    return 5001;
  }

  // Fait le checksum
  byte checksum = 0;
  for (int i = 1; i < 8; i++) {
    checksum += response[i];
  }
  
  checksum = 0xFF - checksum;
  checksum++;

  // Erreur checksum
  if (response[8] != checksum) {
    return 5002;
  }


// Récupère une valeur basse (low) et haute (high) afin de calculer la concentration en CO2
// "(<type>) valeur" conversion de type. Type (implicite) de la valeur vers le type entre les parenthèses
// Dans notre cas, hexadécimal vers entier non signé encodé sur 16 bit.
  uint16_t low  = (uint16_t)response[3];
  uint16_t high = (uint16_t)response[2];
  
  uint16_t ppm = (256 * high) + low;

  return ppm;
}
