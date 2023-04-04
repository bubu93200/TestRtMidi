// TestRtMidi.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <cstdlib>
#include <signal.h>

#define __WINDOWS_MM__ // Ne fonctionne Pas. Il faut mettre cette définition dan RtMidi.h
#include "RtMidi.h"

// Platform-dependent sleep routines.
#include <windows.h>
#define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 


//#include <conio.h>
//#include<windows.h>

//#include <MMSystem.h>


bool done;
static void finish(int /*ignore*/) { done = true; }

void usage(void) {
    // Error function in case of incorrect command-line
    // argument specifications.
    std::cout << "\nusage: qmidiin <port>\n";
    std::cout << "    where port = the device to use (first / default = 0).\n\n";
    exit(0);
}

int main( int argc, char* argv[] )
{   
    
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
    int nBytes, i;
    double stamp;

    // Minimal command-line check.
    if (argc > 2) usage();

    // RtMidiIn constructor
    try {
        midiin = new RtMidiIn();
    }
    catch (RtMidiError& error) {
        error.printMessage();
        exit(EXIT_FAILURE);
    }

    // Check available ports vs. specified.
    unsigned int port = 0;
    unsigned int nPorts = midiin->getPortCount();
    if (argc == 2) port = (unsigned int)atoi(argv[1]);
    if (port >= nPorts) {
        delete midiin;
        std::cout << "Invalid port specifier!\n";
        usage();
    }

    try {
        midiin->openPort(1); // midiin->openPort(port); port 1 pour que ça fonctionne avec le piano
    }
    catch (RtMidiError& error) {
        error.printMessage();
        delete midiin;
        return 1;
        //goto cleanup;
    }

    // Don't ignore sysex, timing, or active sensing messages.
    midiin->ignoreTypes(false, false, false);

    // Install an interrupt handler function.
    done = false;
    (void)signal(SIGINT, finish);

    // Periodically check input queue.
    std::cout << "Reading MIDI from port " << midiin->getPortName() << " ... quit with Ctrl-C.\n";
    while (!done) {
        
        // TODO : stamp a remplacer ?
        // midiin.getMessage(&message);
        // stamp = midiin.getMessageTimeStamp();
        stamp = midiin->getMessage(&message);
        
        
        nBytes = message.size();
        for (i = 0; i < nBytes; i++)
            std::cout << "Byte " << i << " = " << (int)message[i] << ", "; //Byte 0 = 144 (note on) / 144 (note off ? Normalement ça devrait être 128); Byte 1 = Note (pitch); Byte 2 = Velocity suivi de 0 (off); stamp = (en secondes) => note on = durée depuis le denier off. Note off = durée de la note
        if (nBytes > 0)
            std::cout << "stamp = " << stamp << std::endl;

        // Sleep for 10 milliseconds.
        SLEEP(10);
    }

    
    delete midiin;

    return 0;


    /////////////////////////////////////////////



    std::cout << "Hello World!\n";
    return 0;
}

// Exécuter le programme : Ctrl+F5 ou menu Déboguer > Exécuter sans débogage
// Déboguer le programme : F5 ou menu Déboguer > Démarrer le débogage

// Astuces pour bien démarrer : 
//   1. Utilisez la fenêtre Explorateur de solutions pour ajouter des fichiers et les gérer.
//   2. Utilisez la fenêtre Team Explorer pour vous connecter au contrôle de code source.
//   3. Utilisez la fenêtre Sortie pour voir la sortie de la génération et d'autres messages.
//   4. Utilisez la fenêtre Liste d'erreurs pour voir les erreurs.
//   5. Accédez à Projet > Ajouter un nouvel élément pour créer des fichiers de code, ou à Projet > Ajouter un élément existant pour ajouter des fichiers de code existants au projet.
//   6. Pour rouvrir ce projet plus tard, accédez à Fichier > Ouvrir > Projet et sélectionnez le fichier .sln.
