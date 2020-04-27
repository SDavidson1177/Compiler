/*
 * startup.h
 *
 *  Created on: Apr. 22, 2020
 *      Author: solomon
 */

#ifndef STARTUP_H_
#define STARTUP_H_

#include <fstream>
#include <iostream>

/*
 * This HEAP_SIZE constant determines the size of the heap.
 * The heap size is constant and does not change throughout
 * the run time of the program. The heap size must be a power
 * of two.
 * */
const int HEAP_SIZE = 0x1000000;

void initHelpers(std::ostream& file);
void initializeHeap(std::ostream& file);

#endif /* STARTUP_H_ */
