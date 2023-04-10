// TestRtMidi.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
// Version de test
// Fonctionnalités utilisables :
// - affichage sur l'écran de l'entrée midi 1 (piano)
// - écriture de la journalisation (logging) dans un ficher journalier ou sur la console selon ConsoleLog = true/false.
// TODO : Table de traduction entre instrument et norme midi si l'instrument n'est pas tout à fait conforme
// TODO : Passage des paramètres Midi vers le programme appelant en utilisant la RAM
// TODO : Passage des autres paramètres du programme appelant vers ce programme


// Compilation mutliplateformes :
//    Windows : Les macros spécifiques à Windows incluent "_WIN32" pour les applications 32 bits et 64 bits, 
//              "_WIN64" pour les applications 64 bits uniquement, 
//              et "_WINDOWS" pour toutes les versions de Windows.
//    Linux: Les macros spécifiques à Linux incluent "linux" et "gnu_linux".
//           Ces macros sont généralement utilisées pour la compilation de code sur des systèmes Linux et GNU.
//    Android : Les macros spécifiques à Android incluent "ANDROID" et "ANDROID_API".
//              "ANDROID" est défini lorsque le code est compilé pour une plate - forme Android,
//              et "ANDROID_API" est défini avec la version de l'API Android cible.


#include <fstream>
#include <iostream>
#include <cstdlib>
#include <signal.h>

#define __WINDOWS_MM__ // Ne fonctionne Pas. Il faut mettre cette définition dans RtMidi.h

// Platform-dependent sleep routines.
#ifdef _WINDOWS
    #include <windows.h>
#endif // _WINDOWS


#include <conio.h> // for _kbhit()
#include "RtMidi.h"
#define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 

#include "include/spdlog/spdlog.h" // Journalisation. logging
#include "include/spdlog/sinks/daily_file_sink.h" // Journalisation vers un fichier journalier
#include "include/spdlog/sinks/stdout_color_sinks.h" // Journalisation vers la console





// Variables définies par le programme appelant

bool LOG = true; // enregistrement d'un fichier de log
bool MIDI = true; // enregistrement d'un fichier midi
bool ConsoleLog = false; // Enregistrement du fichier de log vers la console. Sinon enrtegistrement vers un fichier journalier
bool done;
auto dailyfilelog = "TestRtMidi.log";
static void finish(int /*ignore*/) { done = true; }

// Functions  headers :
void usage(std::shared_ptr<spdlog::logger> logger);



int main( int argc, char* argv[] )
{   
    ////////////////////////////////////////////////////////////////////////
    // Initialisation des variables
    RtMidiIn* midiin = 0;
    std::vector<unsigned char> message;
    double deltaTime = 0.0;
    double absoluteTime = 0.0;
    ////////////////////////////////////////////////////////////////////////
    
    ////////////////////////////////////////////////////////////////////////
    // INIT LOGGING
    // Create a daily logger - a new file is created every day on 0:00am

    auto logger = std::shared_ptr<spdlog::logger>(); // Define logger outside the switch statement

    switch (ConsoleLog ? 1 : 0) {
    case 0:
        logger = spdlog::daily_logger_mt("daily_logger", dailyfilelog, 0, 0);
        break;
    case 1:
        logger = spdlog::stdout_color_mt("console_logger");
        break; 
    default:
        break;
    }
    
    spdlog::flush_every(std::chrono::seconds(3));
    // Log beginning messages using spdlog::info
    logger->info("******************** Welcome to spdlog! ******************");
    logger->info("************ Demarrage du programme TestRtMidi ***********");
    /////////////////////////////////////////////////////////////////////////////////
    

    // Minimal command-line check.
    if (argc > 2) usage(logger);


    // RtMidiIn constructor
    try {
        midiin = new RtMidiIn();
    }
    catch (RtMidiError& error) {
        logger->error("Impossible de creer un flux midi en entree"); 
        error.printMessage();
        exit(EXIT_FAILURE);
    }

    if (MIDI) { // préparation du fichier au format midi
          // TODO : Format complexe. A écrire plus tard   
    }

    // Check available ports vs. specified.
    unsigned int port = 1; // port=1 pour le piano sinon ça ne fonctionne pas 
    unsigned int nPorts = midiin->getPortCount();
    if (argc == 2) port = (unsigned int)atoi(argv[1]); // midiin->openPort(port); port 1 pour que ça fonctionne avec le piano
    if (port >= nPorts) {
        delete midiin;
        std::cout << "Invalid port specifier!\n";
        logger->error("Invalid port specifier. port midi: {}", port);
        usage(logger);
    }

    try {
        midiin->openPort(port); 
    }
    catch (RtMidiError& error) {
        logger->error("Impossible d'ouvrir le port midi: {}", port);
        error.printMessage();
        delete midiin;
        return -1;
        //goto cleanup;
    }

    // Don't ignore sysex, timing, or active sensing messages.
    midiin->ignoreTypes(false, false, false);

    // Install an interrupt handler function.
    done = false;
    (void)signal(SIGINT, finish);

    // Periodically check input queue.
    std::cout << "Reading MIDI from port " << midiin->getPortName() << " ... quit with Ctrl-C.\n";
    logger->flush();

    while (!done) {
        
        // TODO : stamp a remplacer ?
        // midiin.getMessage(&message);
        // stamp = midiin.getMessageTimeStamp();

        ///////// Faire le choix de la version 1 ou 2
        // Version 1 Fonctionne
        //stamp = midiin->getMessage(&message);
        try { // Rtmidi hasn't absolut datation time
            deltaTime = midiin->getMessage(&message); // It's the delta time of an event
        }
        catch (RtMidiError& error) {
            logger->error("Impossible de lire le message midi");
            error.printMessage();
        }
        // Datation absolue depuis le début
        absoluteTime += deltaTime;

        // Version 2
        //midiin->getMessage(&message, &stamp);
        // Version 3
        //midiin.getMessage(&message);
        //stamp = midiin.getMessageTimeStamp();
        ///////////

        int nBytes = (int) message.size();
        for (int i = 0; i < nBytes; i++) {
            std::cout << "Byte " << i << " = " << (int)message[i] << ", ";
            //Byte 0 = 144 (0x90) (note on) / 144 (note off ? Normalement ça devrait être 128); Byte 1 = Note (pitch); Byte 2 = Velocity suivi de 0 (off); stamp = (en secondes) => note on = durée depuis le denier off. Note off = durée de la note
            // left pedal  : Byte 0 = 176 (0xb0); Byte 1 = 67; Byte 2 = velocity (127)
            // middle pedal  : Byte 0 = 176 (0xb0); Byte 1 = 66; Byte 2 = velocity (127)
            // right pedal  : Byte 0 = 176 (0xb0); Byte 1 = 64; Byte 2 = velocity (0 to 127)

            if (MIDI) {
                // TODO : Écrire le message MIDI dans le fichier de sortie
                // Format complexe. 
            }
            if (LOG) {
                // Write to file log
            }
        }

        if (nBytes > 0) {
            std::cout << "deltaTime = " << deltaTime << " absoluteTime = " << absoluteTime << std::endl;
            logger->info("Entree Midi: {0:2d} Piano: 0x{1:x} Pitch: {2:03d} Velocity: {3:03d} DeltaTime: {4:09.3f} AbsoluteTime: {5:10.3f}", port, message[0], message[1], message[2], deltaTime, absoluteTime);
            logger->flush();
        }
            
        // Sleep for 10 milliseconds.
        SLEEP(10);

        // Test touche entrée pour sortir
        if (_kbhit())
            done = true;

    }

    

    // TODO : Ecritude du fichier midi
  

    //delete midiin;
    midiin->closePort();


    return 0;

}

///////////////////////////////////////////////////////////////////////////////////////
// functions
void usage(std::shared_ptr<spdlog::logger> logger) {
    // Erreur s'il y a plus de 1 argument
    // usage normal : qmidiin 0 ou qmidiin 1
    logger->error("Lancement du programme. Nombre d'arguments incorrect");
    std::cout << "\nusage: qmidiin <port>\n";
    std::cout << "    where port = the device to use (first / default = 0).\n\n";
    exit(-1);
}
