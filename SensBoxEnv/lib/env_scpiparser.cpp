/*============================================================================
 Name        : env_scpiparser.cpp
 Author      : Wim
 Version     : V1.0
 Copyright   :
 Description : SCPI parser  for the environment sensor server
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




*/
//============================================================================
#include <stdio.h>
#include "scpiparser.h"
#include "env_scpiparser.h"
#include <string.h>

struct scpi_parser_context ctx;


extern  float read_temperature( int );
extern  float read_humidity(void);
extern  float read_luminosity(void);



scpi_error_t identify(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t send_stop(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t get_temperature(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t get_temperature_ch(struct scpi_parser_context* context,struct scpi_token* command);  // for channel given as parameter
scpi_error_t get_temperature_ch0(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t get_temperature_ch1(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t get_temperature_ch2(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t get_luminosity(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t get_humidity(struct scpi_parser_context* context,struct scpi_token* command);

void scpi_setup() {

	struct scpi_command* measure;
	struct scpi_command* meas_temp;

	/* First, initialise the parser. */
	scpi_init(&ctx);

	/*
	 * After initialising the parser, we set up the command tree.  Ours is
	 *
	 *  *IDN?         -> identify
	 *  *STOP	  -> stop the server  
	 *  :MEASure
	 *    :HUMIdity?   -> get humidity in % , float
	 *    :LUMInosity? -> get luminosity in LUX
	 *    :TEMPerature?  -> get temperature of ch 0
	 *    :TEMPerature?: CH X  -> get temperature of ch min (x,2)
	 *    :TEMPerature? CH0  -> get temperature of ch0  ( or CH1  or CH2)
	 *    :TEMPerature? 0   -> get temperature of ch0  ( or ch  1  or ch 2)
	 *    :TEMP1?  -> get temperature of ch1   (or 0 or 2 )
	 */
	scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*IDN?", 5,"*IDN?", 5, identify);
	scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*STOP", 5,"*STP", 3, send_stop);
	measure = scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "MEASURE", 7, "MEAS", 4, NULL);
	meas_temp=scpi_register_command(measure, SCPI_CL_CHILD, "TEMPERATURE?", 12, "TEMP?", 5,get_temperature);
	scpi_register_command(measure, SCPI_CL_CHILD, "TEMPERATURE0?", 13, "TEMP0?", 6,get_temperature_ch0);
	scpi_register_command(measure, SCPI_CL_CHILD, "TEMPERATURE1?", 13, "TEMP1?", 6,get_temperature_ch1);
	scpi_register_command(measure, SCPI_CL_CHILD, "TEMPERATURE2?", 13, "TEMP2?", 6,get_temperature_ch2);
	scpi_register_command(meas_temp, SCPI_CL_CHILD, "CHANNEL#", 6, "CH#", 2,get_temperature_ch);
	scpi_register_command(measure, SCPI_CL_CHILD, "HUMIDITY?", 9, "HUMI?", 5,get_humidity);
	scpi_register_command(measure, SCPI_CL_CHILD, "LUMINOSITY?", 11, "LUMI?", 5,get_luminosity);



	//Serial.begin(9600);
}

/*
 * Respond to *IDN?
 */
scpi_error_t identify(struct scpi_parser_context* context,struct scpi_token* command) {
	scpi_free_tokens(command);

	add2result("EnvServ V1.2");
	return SCPI_SUCCESS;
}

scpi_error_t send_stop(struct scpi_parser_context* context,struct scpi_token* command) {
	scpi_free_tokens(command);

	add2result("STOP done");
	return SCPI_SUCCESS;
}

/**
 * Read the temperature of ch 0
 */
scpi_error_t get_temperature_ch0(struct scpi_parser_context* context,struct scpi_token* command) {
	float temperature;

	temperature= read_temperature(0);

	add2resultf(temperature);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}

scpi_error_t get_temperature_ch1(struct scpi_parser_context* context,struct scpi_token* command) {
	float temperature;

	temperature= read_temperature(1);

	add2resultf(temperature);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}
scpi_error_t get_temperature_ch2(struct scpi_parser_context* context,struct scpi_token* command) {
	float temperature;

	temperature= read_temperature(2);

	add2resultf(temperature);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}


/**
 * Read the humidity
 */
scpi_error_t get_humidity(struct scpi_parser_context* context,struct scpi_token* command) {
	float humidity=read_humidity();


	add2resultf(humidity);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}

/**
 * Read the luminosity
 */
scpi_error_t get_luminosity(struct scpi_parser_context* context,struct scpi_token* command) {
	float luminosity= read_luminosity();


	add2resultf(luminosity);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}

scpi_error_t get_temperature(struct scpi_parser_context* context,struct scpi_token* command){
	struct scpi_token* args;
	struct scpi_numeric output_numeric;
	unsigned char chan;// hc

	args = command;
	while (args != NULL && args->type == 0) {
		args = args->next;
	}
	if ( args != NULL)	{
		chan=0; // default if  wrong argument is given
		if ( strcmp(args->value , "CH0") == 0  ) chan=0;
		if ( strcmp(args->value , "CH1") == 0  ) chan=1;
		if ( strcmp(args->value , "CH2") == 0  ) chan=2;
		if ( strcmp(args->value , "0") == 0  ) chan=0;
		if ( strcmp(args->value , "1") == 0  ) chan=1;
		if ( strcmp(args->value , "2") == 0  ) chan=2;
	} else { chan=0;}  // no parameter given
	if( chan > 2 ) chan=2; // unsigned so can not  < 0
	float temperature=0;
	temperature=read_temperature(chan);
	add2resultf(temperature);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}






/**
 * Read the temperature of ch X
 */

scpi_error_t get_temperature_ch(struct scpi_parser_context* context,struct scpi_token* command) {
	struct scpi_token* args;
	struct scpi_numeric output_numeric;
	unsigned char chan;// hc

	args = command;

	while (args != NULL && args->type == 0) {
		args = args->next;
	}

	output_numeric = scpi_parse_numeric(args->value, args->length, 0, 0, 2);
	chan=(unsigned char)output_numeric.value;
	if( chan > 2 ) chan=2; // unsigned so can not  < 0

	 float temperature=0;
	//printf("read temperature for ch : %d \n\r", chan);
	temperature=read_temperature(chan);
	//printf("got  temperature  %f for ch : %d \n\r",temperature , chan );
	add2resultf(temperature);
	//printf("added temperature  %f for ch : %d to result \n\r",temperature , chan );
	scpi_free_tokens(command);

	return SCPI_SUCCESS;
}

void env_scpi_execute_command(const char* l_buf,int length){ scpi_execute_command(&ctx, l_buf,length);}
char* env_get_result() { return get_result();}

