// TestRtMidi.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
// Version de test
// Fonctionnalités utilisables :
// - affichage sur l'écran de l'entrée midi 1 (piano)
// - écriture de la journalisation (logging) dans un ficher journalier ou sur la console selon ConsoleLog = true/false.


#include <fstream>
#include <iostream>
#include <cstdlib>
#include <signal.h>

#define __WINDOWS_MM__ // Ne fonctionne Pas. Il faut mettre cette définition dan RtMidi.h

// Platform-dependent sleep routines.
#include <windows.h>
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
    

    //ignore le premier programme
    if (false) {

        //RtMidi
        // midiprobe.cpp

        RtMidiIn* midiin = 0;
        RtMidiOut* midiout = 0;
        // RtMidiIn constructor
        try {
            midiin = new RtMidiIn();
        }
        catch (RtMidiError& error) {
            error.printMessage();
            exit(EXIT_FAILURE);
        }
        // Check inputs.
        unsigned int nPorts = midiin->getPortCount();
        std::cout << "\nThere are " << nPorts << " MIDI input sources available.\n";
        std::string portName;
        for (unsigned int i = 0; i < nPorts; i++) {
            try {
                portName = midiin->getPortName(i);
            }
            catch (RtMidiError& error) {
                error.printMessage();
                goto cleanup;
            }
            std::cout << "  Input Port #" << i + 1 << ": " << portName << '\n';
        }
        // RtMidiOut constructor
        try {
            midiout = new RtMidiOut();
        }
        catch (RtMidiError& error) {
            error.printMessage();
            exit(EXIT_FAILURE);
        }
        // Check outputs.
        nPorts = midiout->getPortCount();
        std::cout << "\nThere are " << nPorts << " MIDI output ports available.\n";
        for (unsigned int i = 0; i < nPorts; i++) {
            try {
                portName = midiout->getPortName(i);
            }
            catch (RtMidiError& error) {
                error.printMessage();
                goto cleanup;
            }
            std::cout << "  Output Port #" << i + 1 << ": " << portName << '\n';
        }
        std::cout << '\n';
        // Clean up
    cleanup:
        delete midiin;
        delete midiout;

    }
    //////////////////////////////////////////////
    
    // 2eme programme
    RtMidiIn* midiin = 0;
    std::vector<unsigned char> message;
    double stamp;


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
        // Version 1
        stamp = midiin->getMessage(&message);
        // Version 2
        //midiin.getMessage(&message);
        //stamp = midiin.getMessageTimeStamp();
        ///////////

        int nBytes = (int) message.size();
        for (int i = 0; i < nBytes; i++) {
            std::cout << "Byte " << i << " = " << (int)message[i] << ", ";
            //Byte 0 = 144 (note on) / 144 (note off ? Normalement ça devrait être 128); Byte 1 = Note (pitch); Byte 2 = Velocity suivi de 0 (off); stamp = (en secondes) => note on = durée depuis le denier off. Note off = durée de la note
            // left pedal  : Byte 0 = 176 ; Byte 1 = 67; Byte 2 = velocity (127)
            // middle pedal  : Byte 0 = 176 ; Byte 1 = 66; Byte 2 = velocity (127)
            // right pedal  : Byte 0 = 176 ; Byte 1 = 64; Byte 2 = velocity (0 to 127)

            if (MIDI) {
                // TODO : Écrire le message MIDI dans le fichier de sortie
                // Format complexe. 
            }
            if (LOG) {
                // Write to file log
            }
        }

        if (nBytes > 0) {
            std::cout << "stamp = " << stamp << std::endl;
            logger->info("Entree Midi: {0:1d} Piano: 0x{1:x} Pitch: {2:03d} Velocity: {3:03d} Stamp: {4:02.3f}", port, message[0], message[1], message[2], stamp);
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
