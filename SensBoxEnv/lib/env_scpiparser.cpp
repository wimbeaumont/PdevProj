/*============================================================================
 Name        : env_scpiparser.cpp
 Author      : Wim
 Version     : V2.0
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

version history 
V 1.0  used on the Raspberry Pi
V 2.0  targeting to work with an mbed ( serial ) interface , but this version only works with dummy 
       the this_readtemperature is a patch and should been done on the make ( cmake) level to provide an dummy external readme 
V 2.2  add ADC / voltage support 
V 3.0  20230319 added reset and status info 
V 3.1  20250818 added IO readout
*/
//============================================================================
#include <stdio.h>
#include "scpiparser.h"
#include "env_scpiparser.h"
#include <string.h>
#include "Sens_hwstatus.h"

struct scpi_parser_context ctx;
// general device status
int dev_status=0;
static int nr_i2crst=0;
//#define __DUMMY__  // this is only for the dummy version

#if defined  __DUMMY__ 
int dummy_io[]={1,2,3,0,0,0,0,0,0,0}; // dummy io values
float this_read_temperature( int ch ) { return ch+21.34; }
float read_temperature_ntc(int ch) { return 11.34; } // dummy read temperature for channel
float this_read_humidity(int ch=0) {return 30.4; }
float read_dewpoint(int ch=0) {return 12.4; }
float this_read_luminosity(void) {return 0; }
float this_read_voltage( int i ) { return  2.25;}
float read_humidity_adc(int ch=0){return 20.2;};
int get_i2cstatus(void) {return 0;};
int reset_i2cbus(void){return 0;};
int this_read_io(int chan) { return dummy_io[chan]; }
int write_io(int chan, int setvalue ) { return dummy_io[chan]=setvalue; }
 #else 

extern  float read_temperature( int ch );
extern  float read_temperature_ntc(int ch);
extern  int   read_io(int chan);
extern  int   write_io(int chan,int setvalue);
extern  float read_humidity(int ch=0);
extern  float read_dewpoint(int ch=0);
extern  float read_humidity_adc(int ch=0);
extern  float read_luminosity(void);
extern  float read_voltage(int ch=0 );
extern  int   get_i2cstatus(void);
extern  int   reset_i2cbus(void);

float this_read_temperature( int i ) { return read_temperature( i); }
float this_read_humidity(int ch=0) {return read_humidity(ch); }
float this_read_luminosity(void) {return read_luminosity(); }
float this_read_voltage( int i ) { return read_voltage( i); }
int   this_read_io(int chan  ) { return read_io(chan); }

#endif 

scpi_error_t identify(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t gethwversion(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t get_error(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t send_stop(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t get_temperature(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t get_temperature_ch(struct scpi_parser_context* context,struct scpi_token* command);  // for channel given as parameter
scpi_error_t get_temperature_ch0(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t get_temperature_ch1(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t get_temperature_ch2(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t get_temperature_ch3(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t get_luminosity(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t get_humidity(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t get_humidity_ch(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t get_dewpoint_ch(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t get_voltage_ch(struct scpi_parser_context* context,struct scpi_token* command);
//scpi_error_t get_io(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t get_io_ch(struct scpi_parser_context* context,struct scpi_token* command);
//scpi_error_t set_io(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t set_io_ch(struct scpi_parser_context* context,struct scpi_token* command);

scpi_error_t dummyget(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t getstatus(struct scpi_parser_context* context,struct scpi_token* command);
scpi_error_t dummymode(struct scpi_parser_context* context,struct scpi_token* command);
char*  hwversion;

void scpi_setup(char* hwversion_ ) {
	hwversion=hwversion_;
	struct scpi_command* measure;
	struct scpi_command* meas_temp;
	struct scpi_command* meas_hum;
	struct scpi_command* meas_dp;
	struct scpi_command* meas_volt;
	struct scpi_command* get_gen;
	struct scpi_command* get_gen_io;
	struct scpi_command* get_gen_status;
	struct scpi_command* set_gen;
	struct scpi_command* set_gen_io;
	struct scpi_command* set_gen_mode;
	/* First, initialise the parser. */
	scpi_init(&ctx);

	/*
	 * After initialising the parser, we set up the command tree.  Ours is
	 *
	 *  *IDN?         -> identify
	 *  *STOP	  -> stop the server  
	 *  *ERRor?      -> get the error status of the server
	 *  *HWVer?      -> get the hardware version
	 *  :MEASure
	 *    :HUMIdity?:CH X    -> get humidity in % , float 
	 *    :LUMInosity? -> get luminosity in LUX
	 *    :TEMPerature?  -> get temperature of ch 0
	 *    :TEMPerature?: CH X  -> get temperature of ch min (x,2)
	 *    :TEMPerature? CH0  -> get temperature of ch0  ( or CH1  or CH2)
	 *    :TEMPerature? 0   -> get temperature of ch0  ( or ch  1  or ch 2)
	 *    :TEMP1?  -> get temperature of ch1   (or 0 or 2 )
	 * :GET 
	 *    :IO?:CH X  -> X is io ,, :GET:IO? X  gives an error 
	 *    :STATUS  -> get fix message 
	 * :SET
	 *   :IO:CH X  -> X is io ,, :SET:IO? X  gives an error 
	 *    :MODE -> get fix message
	 */
	
	scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*IDN?", 5,"*IDN?", 5, identify);
	scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*HWVer?", 7,"*HWver?", 7, gethwversion);
	scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*STOP", 5,"*STP", 3, send_stop);
	scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*ERRor?", 7,"*ERR?", 5, get_error);
	measure = scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "MEASURE", 7, "MEAS", 4, NULL);
	meas_temp=scpi_register_command(measure, SCPI_CL_CHILD, "TEMPERATURE?", 12, "TEMP?", 5,get_temperature);
	scpi_register_command(measure, SCPI_CL_CHILD, "TEMPERATURE0?", 13, "TEMP0?", 6,get_temperature_ch0);
	scpi_register_command(measure, SCPI_CL_CHILD, "TEMPERATURE1?", 13, "TEMP1?", 6,get_temperature_ch1);
	scpi_register_command(measure, SCPI_CL_CHILD, "TEMPERATURE2?", 13, "TEMP2?", 6,get_temperature_ch2);
	scpi_register_command(measure, SCPI_CL_CHILD, "TEMPERATURE3?", 13, "TEMP3?", 6,get_temperature_ch3);
	scpi_register_command(meas_temp, SCPI_CL_CHILD, "CHANNEL", 7, "CH", 2 ,get_temperature_ch);
	meas_hum=scpi_register_command(measure, SCPI_CL_CHILD, "HUMIDITY?", 9, "HUMI?", 5,get_humidity);
	scpi_register_command(meas_hum, SCPI_CL_CHILD, "CHANNEL", 7, "CH", 2 ,get_humidity_ch);
	meas_dp=scpi_register_command(measure, SCPI_CL_CHILD, "DEWPOINT?", 8, "DP?", 3,NULL);
	scpi_register_command(meas_dp, SCPI_CL_CHILD, "CHANNEL", 7, "CH", 2 ,get_dewpoint_ch);
	scpi_register_command(measure, SCPI_CL_CHILD, "LUMINOSITY?", 11, "LUMI?", 5,get_luminosity);
	meas_volt=scpi_register_command(measure, SCPI_CL_CHILD, "VOLTAGE?", 11, "VOLT?", 5,NULL);
	scpi_register_command(meas_volt, SCPI_CL_CHILD, "CHANNEL", 7, "CH", 2 ,get_voltage_ch);
	get_gen = scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "GETGEN", 6, "GET", 3,NULL);
	set_gen = scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "SETGEN", 6, "SET", 3,NULL);
	get_gen_io = scpi_register_command(get_gen, SCPI_CL_CHILD, "INOUT?", 6, "IO?", 3 ,NULL); 
	set_gen_io = scpi_register_command(set_gen, SCPI_CL_CHILD, "INOUT", 5, "IO", 2 ,NULL); 
	scpi_register_command(get_gen_io, SCPI_CL_CHILD, "CHANNEL", 7 , "CH", 2 , get_io_ch);
	scpi_register_command(set_gen_io, SCPI_CL_CHILD, "CHANNEL", 7 , "CH", 2 , set_io_ch);
    get_gen_status = scpi_register_command(get_gen, SCPI_CL_CHILD, "STATUS?", 7, "STAT?", 5 ,getstatus);
    set_gen_mode = scpi_register_command(set_gen, SCPI_CL_CHILD, "MODE", 4, "MOD", 3 ,dummymode);
	//Serial.begin(9600);
}

/*
 * Respond to *IDN?
 */
scpi_error_t identify(struct scpi_parser_context* context,struct scpi_token* command) {
	scpi_free_tokens(command);
	add2result("EnvServ V");add2result(ENV_SCPIPARSER_VER);
	return SCPI_SUCCESS;
}

scpi_error_t gethwversion(struct scpi_parser_context* context,struct scpi_token* command) {
	scpi_free_tokens(command);
	add2result(hwversion);
	return SCPI_SUCCESS;
}

scpi_error_t get_error(struct scpi_parser_context* context,struct scpi_token* command) {
	scpi_free_tokens(command);
	dev_status=get_i2cstatus(); 
	add2resulti(dev_status);
	return SCPI_SUCCESS;
}

scpi_error_t send_stop(struct scpi_parser_context* context,struct scpi_token* command) {
	scpi_free_tokens(command);
	reset_i2cbus();
	nr_i2crst=0;
	add2result("STOP done");
	return SCPI_SUCCESS;
}

/**
 *  Read the voltage of 
*/ 
scpi_error_t get_voltage_ch(struct scpi_parser_context* context,struct scpi_token* command) {
	float volt;

	volt= this_read_voltage(0);
	add2resultf(volt);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}

scpi_error_t dummyget(struct scpi_parser_context* context,struct scpi_token* command) {
	char volt[18]= "dum got called";
	add2result(volt);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}

scpi_error_t getstatus(struct scpi_parser_context* context,struct scpi_token* command) {
	add2resulti(get_hwstatus());
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}


scpi_error_t dummymode(struct scpi_parser_context* context,struct scpi_token* command) {
	char volt[18]= "mod get called";
	add2result(volt);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}


/**
 * Read the temperature of ch 0
 */
scpi_error_t get_temperature_ch0(struct scpi_parser_context* context,struct scpi_token* command) {
	float temperature;

	temperature= this_read_temperature(0);

	add2resultf(temperature);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}

scpi_error_t get_temperature_ch1(struct scpi_parser_context* context,struct scpi_token* command) {
	float temperature;

	temperature= this_read_temperature(1);

	add2resultf(temperature);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}
scpi_error_t get_temperature_ch2(struct scpi_parser_context* context,struct scpi_token* command) {
	float temperature;

	temperature= this_read_temperature(2);

	add2resultf(temperature);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}

scpi_error_t get_temperature_ch3(struct scpi_parser_context* context,struct scpi_token* command) {
	float temperature;

	temperature= read_temperature_ntc(0);

	add2resultf(temperature);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}


/**
 * Read the humidity
 */
scpi_error_t get_humidity_ch(struct scpi_parser_context* context,struct scpi_token* command) {
	struct scpi_token* args = command;
	struct scpi_numeric output_numeric;
	unsigned char chan;
	float  readvalue=0;
	while (args != NULL && args->type == 0) {
		args = args->next;
	}
	output_numeric = scpi_parse_numeric(args->value, args->length, 0, 0, 2);
	chan=(unsigned char)output_numeric.value;
	if( chan > 1 ) chan=1; // unsigned so can not  < 0
	if ( chan == 0 ) {
		readvalue=this_read_humidity();
	} else if ( chan == 1 ) {
		readvalue=read_humidity_adc();
	}
	add2resultf(readvalue);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}

scpi_error_t get_humidity(struct scpi_parser_context* context,struct scpi_token* command) {

	float humidity= this_read_humidity(0);

	add2resultf(humidity);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}
/**
 * Read the humidity
 */
scpi_error_t get_dewpoint_ch(struct scpi_parser_context* context,struct scpi_token* command) {
	struct scpi_token* args = command;
	struct scpi_numeric output_numeric;
	unsigned char chan;
	float  readvalue=0;
	while (args != NULL && args->type == 0) {
		args = args->next;
	}
	output_numeric = scpi_parse_numeric(args->value, args->length, 0, 0, 2);
	chan=(unsigned char)output_numeric.value;
	if( chan > 0 ) chan=0; // unsigned so can not  < 0
	readvalue=read_dewpoint();
	add2resultf(readvalue);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}



/**
 * Read the luminosity
 */
scpi_error_t get_luminosity(struct scpi_parser_context* context,struct scpi_token* command) {
	float luminosity= this_read_luminosity();


	add2resultf(luminosity);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}


/**
 * Read the status of IO  ch X
 */

scpi_error_t get_io_ch(struct scpi_parser_context* context,struct scpi_token* command) {
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
	int  io_status=0;
	io_status=this_read_io(chan);
	add2resulti(io_status);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}


/**
 * Read the status of IO  ch X
 */

scpi_error_t set_io_ch(struct scpi_parser_context* context,struct scpi_token* command) {
	struct scpi_token* args;
	struct scpi_numeric output_numeric;
	unsigned char chan;// hc
	args = command;
	while (args != NULL && args->type == 0) {
		args = args->next;
	}
	//printf("set_io_ch args->value %s :",args->value);
    char arg1[10];
	char arg2[10];
	int i;
    for ( i = 0; i < args->length;i++ ) {
		if ( args->value[i] == ' ' ){
			arg1[i]='\0';
			break;
		}
		arg1[i]=args->value[i];
	}
	int arg1length=strlen(arg1);
	//printf("set_io_ch arg1lenth  %d %d  %d :",arg1length,i,args->length);
	int istart=i++;
	for (i=0 ;  i < args->length ;  i++ ) {
		arg2[i]=args->value[i+istart];
	} 
	
	int arg2length=strlen(arg2);
    //printf("set_io_ch called with arg1 %s  arg2 %s \n\r",arg1,arg2);
	//output_numeric = scpi_parse_numeric(args->value, args->length, 0, 0, 2);
    output_numeric = scpi_parse_numeric(arg1, arg1length, 0, 0, 2);
	chan=(unsigned char)output_numeric.value;
	if( chan > 2 ) chan=2; // unsigned so can not  < 0
	output_numeric = scpi_parse_numeric(arg2, arg2length, 0, 0, 2);
	int  io_status=(unsigned char)output_numeric.value;
	io_status=write_io(chan, io_status); //for the  pico implementation  the write is including a read 
	//io_status=this_read_io(chan);
	add2resulti(io_status);
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
		if ( strcmp(args->value , "CH3") == 0  ) chan=3;
		if ( strcmp(args->value , "0") == 0  ) chan=0;
		if ( strcmp(args->value , "1") == 0  ) chan=1;
		if ( strcmp(args->value , "2") == 0  ) chan=2;
		if ( strcmp(args->value , "3") == 0  ) chan=3;
	} else { chan=0;}  // no parameter given
	if( chan > 3 ) chan=0; // unsigned so can not  < 0
	float temperature=0;
	if ( chan == 3 ) {
		temperature=read_temperature_ntc(0);
	}
	else{
		temperature=this_read_temperature(chan);
	}
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
	temperature=this_read_temperature(chan);
	//printf("got  temperature  %f for ch : %d \n\r",temperature , chan );
	add2resultf(temperature);
	//printf("added temperature  %f for ch : %d to result \n\r",temperature , chan );
	scpi_free_tokens(command);

	return SCPI_SUCCESS;
}


void env_scpi_execute_command(const char* l_buf,int length){
	// check i2c bus status and reset i2c bus if status is < 0
	dev_status=get_i2cstatus(); 
	if ( dev_status < 0) {
		reset_i2cbus();
		nr_i2crst++;
	}
	if( nr_i2crst > 3) { dev_status=-100; }
	 {	scpi_execute_command(&ctx, l_buf,length);}
	 
}

char* env_get_result() { return get_result();}

