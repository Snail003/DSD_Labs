// Lab3.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

HANDLE hEvent1;
HANDLE hEvent2;
HANDLE hTextMutex;
HANDLE hOutputMutex;
std::vector<std::string> text;
int wordsCount = 0;
int symbolsCount = 0;

void WriteToConsole(std::string text) {
	WaitForSingleObject(hOutputMutex, INFINITE);
	std::cout << text;
	ReleaseMutex(hOutputMutex);
}

int countWords(std::string line) {
	if (line.empty()) return 0;

	int wordCount = 0;
	std::istringstream iss(line);
	std::string word;

	while (getline(iss, word, ' ')) {
		if (!word.empty()) {
			wordCount++;
		}
	}
	return wordCount;
}

DWORD WINAPI Worker1(LPVOID lpParam) {
	WriteToConsole("Thread " + std::to_string(GetCurrentThreadId()) + " started working!\n");
	WriteToConsole("Thread " + std::to_string(GetCurrentThreadId()) + " waits for file path...\n");
	std::string filePath;
	std::cin >> filePath;
	WaitForSingleObject(hTextMutex, INFINITE);
	std::ifstream inputFile(filePath);
	// check stream status
	if (!inputFile) {
		WriteToConsole("Can't open input file!");
		ReleaseMutex(hTextMutex);
	}
	WriteToConsole("\nFile content:\n");
	std::string line;
	while (getline(inputFile, line)) {
		text.push_back(line);
		WriteToConsole(line + "\n");
	}
	WriteToConsole("\n");
	inputFile.close();
	ReleaseMutex(hTextMutex);
	SetEvent(hEvent1);
	WriteToConsole("Thread " + std::to_string(GetCurrentThreadId()) + " finished working!\n");
	return 0;
}

DWORD WINAPI Worker2(LPVOID lpParam) {
	WriteToConsole("Thread " + std::to_string(GetCurrentThreadId()) + " waits for signal 1...\n");
	WaitForSingleObject(hEvent1, INFINITE);
	WriteToConsole("Thread " + std::to_string(GetCurrentThreadId()) + " got signal 1 and started working!\n");
	WaitForSingleObject(hTextMutex, INFINITE);
	WriteToConsole("Thread " + std::to_string(GetCurrentThreadId()) + " is accessing shared resource!\n");
	for (int i = 0; i < text.size(); i++) {
		wordsCount = wordsCount + countWords(text[i]);
		symbolsCount = symbolsCount + text[i].length();
	}
	ReleaseMutex(hTextMutex);
	SetEvent(hEvent2);
	WriteToConsole("Thread " + std::to_string(GetCurrentThreadId()) + " finished working!\n");
	return 0;
}

DWORD WINAPI Worker3(LPVOID lpParam) {
	WriteToConsole("Thread " + std::to_string(GetCurrentThreadId()) + " waits for signal 2...\n");
	WaitForSingleObject(hEvent2, INFINITE);
	WriteToConsole("Thread " + std::to_string(GetCurrentThreadId()) + " got signal 2 and started working!\n");
	WaitForSingleObject(hTextMutex, INFINITE);
	WriteToConsole("Thread " + std::to_string(GetCurrentThreadId()) + " is accessing shared resource!\n");
	WriteToConsole("Amount of lines: " + std::to_string(text.size()) + "\n");
	WriteToConsole("Amount of words: " + std::to_string(wordsCount) + "\n");
	WriteToConsole("Amount of symbols: " + std::to_string(symbolsCount) + "\n");
	ReleaseMutex(hTextMutex);
	WriteToConsole("Thread " + std::to_string(GetCurrentThreadId()) + " finished working!\n");
	return 0;
}

int main() {
	const int NUM_THREADS = 3;
	HANDLE threadHandles[NUM_THREADS];

	// Create sync stuff

	hEvent1 = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent1 == NULL) {
		std::cerr << "Couldn't create event 1.\n";
		return 1;
	}
	hEvent2 = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent2 == NULL) {
		std::cerr << "Couldn't create event 2.\n";
		return 1;
	}
	hTextMutex = CreateMutex(NULL, FALSE, NULL);
	if (hTextMutex == NULL) {
		std::cerr << "Couldn't create mutex.\n";
		return 1;
	}
	hOutputMutex = CreateMutex(NULL, FALSE, NULL);
	if (hOutputMutex == NULL) {
		std::cerr << "Couldn't create output mutex.\n";
		return 1;
	}

	// Create threads

	threadHandles[0] = CreateThread(NULL, 0, Worker1, NULL, 0, NULL);
	threadHandles[1] = CreateThread(NULL, 0, Worker2, NULL, 0, NULL);
	threadHandles[2] = CreateThread(NULL, 0, Worker3, NULL, 0, NULL);

	// Wait till all threads finish

	DWORD dwWaitResult = WaitForMultipleObjects(NUM_THREADS, threadHandles, TRUE, INFINITE);

	switch (dwWaitResult) {
		case WAIT_OBJECT_0:
			std::cout << "All threads completed successfully." << std::endl;
		break;

		default:
			std::cerr << "An error occurred during wait." << std::endl;
		break;
	}

	// Close handles

	for (HANDLE handle : threadHandles) {
		CloseHandle(handle);
	}

	CloseHandle(hEvent1);
	CloseHandle(hEvent2);
	CloseHandle(hTextMutex);
	CloseHandle(hOutputMutex);
	return 0;
}
