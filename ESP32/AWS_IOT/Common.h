#ifndef COMMON_H
#define COMMON_H

// General
#include <stdio.h>    // Standard I/O functions
#include <stdlib.h>   // Standard library functions
#include <time.h>     // Time-related functions
#include <SPI.h>      // SPI communication library
#include <cmath>      // C++ standard math library for sqrt calculations

// Main Func Variables
bool isFrontGateOpen = false;           // Variable to track if the front gate is open
bool isBackGateOpen = false;            // Variable to track if the back gate is open
bool publishedAfterFrontGate = false;   // Variable to track if a message has been published after opening the front gate
String tagData = "";                    // String to store data related to tags
int potentiometerValue;                 // Integer variable to store potentiometer value
String receivedDataSerial = "";         // String to store data received from Serial communication
int totalWeight;                        // Integer variable to store total weight
int distributedWeight;                 // Integer variable to store distributed weight
int addVariation;                       // Integer variable for adding variation
int isCellBroken = 0;                  // Integer variable to track if a cell is broken
int weightCell[6];                     // Integer array to store weight data for 6 cells
int brokeCellIndex;                    // Integer variable to store the index of a broken cell
double stdDeviation;                   // Double variable for standard deviation calculation
int toBreakCellCounter = 0;            // Integer variable to count cells to be broken
int isCellValueSetted = 0;             // Integer variable to track if a cell value is set
int removedWeight;                     // Integer variable to store removed weight

#endif