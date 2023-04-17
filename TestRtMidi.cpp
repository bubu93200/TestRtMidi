//=============================================================================
//  Midi Instrument Training plugin for MuseScore
//  Copyright (C) 2014 Jean-Baptiste Delisle
//  Based on colornotes plugin :
//  Copyright (C) 2012 Werner Schweer
//  Copyright (C) 2013 Nicolas Froment, Joachim Schmitz
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================
// Mise à jour pour MuseScore 3
// Copyright (C) 2022 Bruno Donati
//=============================================================================
// TestRtMidi.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
// Version de test
// Fonctionnalités utilisables :
// - affichage sur l'écran de l'entrée midi 1 (piano)
// - écriture de la journalisation (logging) dans un ficher journalier ou sur la console selon ConsoleLog = true/false.
// TODO : Il faut pouvoir desactiver complètement la journalisation
// TODO : Table de traduction entre instrument et norme midi si l'instrument n'est pas tout à fait conforme
// TODO : Passage des paramètres Midi vers le programme appelant en utilisant la RAM
// TODO : Passage des autres paramètres du programme appelant vers ce programme
//=============================================================================

// Compilation mutliplateformes :
//    Windows : Les macros spécifiques à Windows incluent "_WIN32" pour les applications 32 bits et 64 bits, 
//              "_WIN64" pour les applications 64 bits uniquement, 
//              et "_WINDOWS" pour toutes les versions de Windows.
// *_WINDOWS ne fonctionne pas
//    Linux: Les macros spécifiques à Linux incluent "linux" et "gnu_linux".
//           Ces macros sont généralement utilisées pour la compilation de code sur des systèmes Linux et GNU.
//    Android : Les macros spécifiques à Android incluent "ANDROID" et "ANDROID_API".
//              "ANDROID" est défini lorsque le code est compilé pour une plate - forme Android,
//              et "ANDROID_API" est défini avec la version de l'API Android cible.


#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <signal.h>

#define __WINDOWS_MM__ // Ne fonctionne Pas. Il faut mettre cette définition dans RtMidi.h

// Platform-dependent sleep  routines.
#ifdef _WIN32 // OS Windows
#include <cstdlib> // Gestion dynamique de la mémoire
#include <windows.h>
#endif // _WINDOWS


#include <conio.h> // for _kbhit()
#include "RtMidi.h"
#define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 

#include "include/spdlog/spdlog.h" // Journalisation. logging
#include "include/spdlog/sinks/daily_file_sink.h" // Journalisation vers un fichier journalier
#include "include/spdlog/sinks/stdout_color_sinks.h" // Journalisation vers la console





// Variables définies par le programme appelant
constexpr double messageEOF = (double) -1.0;
bool LOG_CONSOLE = true; // visualisation du fichier de log sur la console
bool LOG_FILE = false; // enregistrement d'un fichier de log
bool MIDI = true; // enregistrement d'un fichier midi
bool done;
auto dailyfilelog = "TestRtMidi.log";
static void finish(int /*ignore*/) { done = true; }

// Functions  headers :
void usage(std::shared_ptr<spdlog::logger> logger);



int main(int argc, char* argv[])
{
	////////////////////////////////////////////////////////////////////////
	// Initialisation des variables
	RtMidiIn* midiin = 0;
	double deltaTime = 0.0;
	double absoluteTime = 0.0;
	////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////
	// INIT LOGGING
	// Create a daily logger - a new file is created every day on 0:00am

	auto logger = std::shared_ptr<spdlog::logger>(); // Define logger outside the switch statement

	switch (LOG_FILE ? 1 : 0) {
	case 1:
		logger = spdlog::daily_logger_mt("daily_logger", dailyfilelog, 0, 0);
		LOG_CONSOLE = false;
		break;
	case 0:
		logger = spdlog::stdout_color_mt("console_logger");
		if (!LOG_CONSOLE)
			// Désactiver complètement les logs en définissant le niveau de journalisation à "off"
			logger->set_level(spdlog::level::off);
		break;
	default:
		break;
	}

	spdlog::flush_every(std::chrono::seconds(3));
	// Log beginning messages using spdlog::info
	logger->info("**********************************************************");
	logger->info("******************** Welcome to spdlog! ******************");
	logger->info("************ Demarrage du programme TestRtMidi ***********");
	/////////////////////////////////////////////////////////////////////////////////


	// Minimal command-line check.
	if (argc > 2) usage(logger);

#ifdef _WIN32 // OS Windows
	//////////////////////////////////////////////
	//  Gestion dynamique de la mémoire         //
	// Define the shared memory segment name and size
	logger->info("Gestion dynamique de la memoire : debut initialisation");
	const char* midisharedmemory = "MidiSharedMemory";
	const int SIZE_BUFFER = 1024;
	// Reservation d'espace
	std::vector<unsigned char> message;
	message.reserve(SIZE_BUFFER); // Reservation de la totalité de la mémoire allouée


	// Open the shared memory segment // => crée la mémoire partagée
	HANDLE shared_memory_handle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SIZE_BUFFER, midisharedmemory); 
	if (shared_memory_handle == NULL) {
		std::cerr << "Error opening shared memory segment" << std::endl;
		logger->error("Error opening shared memory segment");
		return -1;
	}

	// Map the shared memory segment to the process's address space // => donne le pointeur vers l'adresse de la mémoire partagée
	void* shared_memory_ptr_begin = (void*)MapViewOfFile(shared_memory_handle, FILE_MAP_ALL_ACCESS, 0, 0, SIZE_BUFFER);
	void* shared_memory_ptr = shared_memory_ptr_begin; // Pointeur variable
	if (shared_memory_ptr_begin == NULL) {
		std::cerr << "Error mapping shared memory segment" << std::endl;
		logger->error("Error mapping shared memory segment");
		CloseHandle(shared_memory_handle);
		return -1;
	}
	logger->info("Gestion dynamique de la memoire : fin initialisation");
	//////////////////////////////////////////////
#endif // _WINDOWS

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
	(void)signal(SIGINT, finish); // Terminer le programme si CTRL-C. Ne fonctionne pas

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

		if (message.size() > 0) { // le message existe
			/////////////////////////////////////////////////////////////////////
			// Formatage du message MIDI
			// D'abord l'horodatage : temps depuis le début de l'écoute (double)
			// Puis les Message Midi (qui peuvent contenir 00)
			// Si on rencontre -1 sur l'horodatage (double), on est arrivé au dernier message

			auto isEOF = static_cast<decltype(absoluteTime)*>(shared_memory_ptr_begin);
			if (*isEOF == messageEOF) // Le premier horodatage est à -1. ça veut que le programme de lecture a vidé le buffer. Il faut revenir au début du buffer
				shared_memory_ptr = shared_memory_ptr_begin;

			auto data1 = (unsigned char)(message.size() > 0 ? message[0] : -1);
			auto data2 = (unsigned char)(message.size() > 1 ? message[1] : -1);
			auto data3 = (unsigned char)(message.size() > 2 ? message[2] : -1);
			// Stockage du message MIDI dans la zone de mémoire partagée
			auto *time_ptr = static_cast<decltype(absoluteTime)*>(shared_memory_ptr);
			auto *data1_ptr = static_cast<decltype(data1)*>(shared_memory_ptr) + sizeof(absoluteTime);
			auto *data2_ptr = static_cast<decltype(data2)*>(shared_memory_ptr) + sizeof(absoluteTime) + sizeof(data1);
			auto *data3_ptr = static_cast<decltype(data3)*>(shared_memory_ptr) + sizeof(absoluteTime) + sizeof(data1) + sizeof(data2);
			auto *EOF_ptr = data3_ptr  + sizeof(data3);
			auto *EOF_ptr_double =  reinterpret_cast<decltype(absoluteTime)*>(EOF_ptr);
			// Stockage du message MIDI daté dans la zone de mémoire partagée
			*time_ptr = absoluteTime;
			*data1_ptr = data1;
			*data2_ptr = data2;
			*data3_ptr = data3;
			*EOF_ptr_double = messageEOF;// -1 (double : taille de la datation)

			// Avance du pointeur pour attendre le message suivant
			shared_memory_ptr = (char*)shared_memory_ptr + sizeof(absoluteTime) + sizeof(data1) + sizeof(data2) + sizeof(data3);

			// Print the MIDI message to the console
			//std::cout << std::hex << (char*)message.data() << std::endl; // Problème d'affichage

			// Afficher chaque caractère du vecteur en hexadécimal
			for (const char& c : message) {
				std::cout << std::hex << static_cast<int>(c) << " ";
			}
			// Afficher les tailles des messages
			std::cout << " taille: " << sizeof(absoluteTime) << " " << sizeof(message[0]) << " " << sizeof(message[1]) << " " << sizeof(message[1]);
			std::cout << std::endl;

			// Afficher le contenu des pointeurs
			auto* ptr = (char*)shared_memory_ptr;
			for (int i = 0; i < 20; i++) {
				//std::cout << std::hex << (unsigned char) *ptr << "|";
				printf("%02x|", (char)*ptr);
				ptr++;
			}
			std::cout << std::endl;

			//TODO : ajouter une info log

		}

		int nBytes = (int)message.size();
		for (int i = 0; i < nBytes; i++) {
			std::cout << std::dec << "Byte " << i << " = " << (int)message[i] << ", ";
			//Byte 0 = 144 (0x90) (note on) / 144 (note off ? Normalement ça devrait être 128); Byte 1 = Note (pitch); Byte 2 = Velocity suivi de 0 (off); stamp = (en secondes) => note on = durée depuis le denier off. Note off = durée de la note
			// left pedal  : Byte 0 = 176 (0xb0); Byte 1 = 67; Byte 2 = velocity (127)
			// middle pedal  : Byte 0 = 176 (0xb0); Byte 1 = 66; Byte 2 = velocity (127)
			// right pedal  : Byte 0 = 176 (0xb0); Byte 1 = 64; Byte 2 = velocity (0 to 127)

			if (MIDI) {
				// TODO : Écrire le message MIDI dans le fichier de sortie
				// Format complexe. 
			}
			if (LOG_CONSOLE) {
				// Write to console log
			}
			if (LOG_FILE) {
				// Write to console log
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

	// Unmap the shared memory segment
	if (UnmapViewOfFile(shared_memory_ptr) == 0) {
		logger->error("Error unmapping shared memory segment");
		std::cerr << "Error unmapping shared memory segment" << std::endl;
		CloseHandle(shared_memory_handle);
		return -1;
	}

	// Close the shared memory handle
	logger->flush();
	logger->info("Gestion dynamique de la memoire : liberation de la memoire");
	CloseHandle(shared_memory_handle);


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
