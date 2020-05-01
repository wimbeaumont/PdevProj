/*
 * env_scpiparser.h
 *
 *  Created on: Apr 23, 2020
 *      Author: wimb
 *
Copyright (c) 2020 Wim Beaumont
Copyright (c) 2013 Lachlan Gunn

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

This is a wrapper arround the scpi wrappers lib for the envsens server
It shield all the scpi  lib calls from  the rest of the program
it initialise the SCPI tree for the envsens project and passes only the necessary functions.
*/

#ifndef ENV_SCPIPARSER_H_
#define ENV_SCPIPARSER_H_

void scpi_setup();

void env_scpi_execute_command(const char* l_buf,int length);
char* env_get_result() ;
#endif /* ENV_SCPIPARSER_H_ */
