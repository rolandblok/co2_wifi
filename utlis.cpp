#include "utils.h"

/**
 * return a string that has traling spaces
 *   Number : number to be printed
 *   Digits : number of ditis to use
 *  Example : n = 40, d = 4 
 *   String : "  40"
 */
String trail1000print(int i){
  return trail1000print(i, false);
}

String trail1000print(int i, boolean use_zeros) {
  String filler_str = "";
  if (use_zeros) filler_str = "0";
  else           filler_str = " ";
  String s = "";
  if (i < 1000) s = s + filler_str;
  if (i < 100)  s = s + filler_str;
  if (i < 10)   s = s + filler_str;
  s = s + String(i);
  return s;
}

String trail10print(int i){
  return trail10print(i, false);
}

String trail10print(int i, boolean use_zeros){
  String filler_str = "";
  if (use_zeros) filler_str = "0";
  else           filler_str = " ";
  String s = "";
  if (i < 10)   s = s + filler_str;
  s = s + String(i);
  return s;
}
