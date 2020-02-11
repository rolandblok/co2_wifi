#ifndef UTILS
#define UTILS

#include <Arduino.h>


/**
 * return a string that has traling spaces
 *   Number : number to be printed
 *   Digits : number of ditis to use
 *  Example : n = 40, d = 4 
 *   String : "  40"
 */
String trail1000print(int i);
String trail1000print(int i, boolean use_zeros);
String trail10print(int i);
String trail10print(int i, boolean use_zeros);

#endif
