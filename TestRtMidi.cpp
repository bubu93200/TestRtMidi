// TestRtMidi.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <cstdlib>
#include <signal.h>

#define __WINDOWS_MM__ // Ne fonctionne Pas. Il faut mettre cette définition dan RtMidi.h
#include "RtMidi.h"


//#include <conio.h>
//#include<windows.h>

//#include <MMSystem.h>

int main()
{

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
        

    //////////////////////////////////////////////
    


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
