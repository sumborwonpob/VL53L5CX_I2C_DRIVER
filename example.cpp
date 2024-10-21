#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "VL53L5CX_I2C_DRIVER/vl53l5cx_api.h"
#include "VL53L5CX_I2C_DRIVER/vl53l5cx_api.c"
#include "VL53L5CX_I2C_DRIVER/platform.h"
#include "VL53L5CX_I2C_DRIVER/platform.c"

// :::::::::::::::::::::::::::::::::: MAIN :::::::::::::::::::::::::::::::::::::::
int main(int argc, char * argv[])
{
	/*********************************/
	/*   VL53L5CX ranging variables  */
	/*********************************/

	uint8_t 				status, loop, isAlive, isReady, i;
	VL53L5CX_Configuration 	Dev;			/* Sensor configuration */
	VL53L5CX_ResultsData 	Results;		/* Results data from VL53L5CX */


	/*********************************/
	/*      Customer platform        */
	/*********************************/

	/* Fill the platform structure with customer's implementation. For this
	* example, only the I2C address is used.
	*/
	Dev.platform.address = 0x52;
	auto p_dev = &Dev.platform;

    /* Initialize sensor's file descriptor */
	int init_ret = VL53L5CX_Comms_Init(p_dev, "/dev/i2c-5");
	if(init_ret != 0) exit(init_ret);

	/* (Optional) Reset sensor toggling PINs (see platform, not in API) */
	//VL53L5CX_Reset_Sensor(&(Dev.platform));

	/* (Optional) Set a new I2C address if the wanted address is different
	* from the default one (filled with 0x20 for this example).
	*/
	//status = vl53l5cx_set_i2c_address(&Dev, 0x20);


	/*********************************/
	/*   Power on sensor and init    */
	/*********************************/

	/* (Optional) Check if there is a VL53L5CX sensor connected */
	status = vl53l5cx_is_alive(&Dev, &isAlive);
	if(!isAlive || status)
	{
		printf("VL53L5CX not detected at requested address\n");
		return status;
	}
	else
	{
		printf("VL53L5CX is there\n");
	}

	/* (Mandatory) Init VL53L5CX sensor */
	status = vl53l5cx_init(&Dev);
	if(status)
	{
		printf("VL53L5CX ULD Loading failed\n");
		return status;
	}

	printf("VL53L5CX ULD ready ! (Version : %s)\n",
			VL53L5CX_API_REVISION);

	/*********************************/
	/*         Ranging loop          */
	/*********************************/

	status = vl53l5cx_start_ranging(&Dev);

	loop = 0;
	while(loop < 10)
	{
		/* Use polling function to know when a new measurement is ready.
		 * Another way can be to wait for HW interrupt raised on PIN A1
		 * (INT) when a new measurement is ready */
 
		status = vl53l5cx_check_data_ready(&Dev, &isReady);

		if(isReady)
		{
			vl53l5cx_get_ranging_data(&Dev, &Results);

			/* As the sensor is set in 4x4 mode by default, we have a total 
			 * of 16 zones to print. For this example, only the data of first zone are 
			 * print */
			printf("Print data no : %3u\n", Dev.streamcount);
			for(i = 0; i < 16; i++)
			{
				printf("Zone : %3d, Status : %3u, Distance : %4d mm\n",
					i,
					Results.target_status[VL53L5CX_NB_TARGET_PER_ZONE*i],
					Results.distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*i]);
			}
			printf("\n");
			loop++;
		}

		/* Wait a few ms to avoid too high polling (function in platform
		 * file, not in API) */
		VL53L5CX_WaitMs(&(Dev.platform), 5);
	}

	status = vl53l5cx_stop_ranging(&Dev);

	/* Close file descriptor */
	VL53L5CX_Comms_Close(p_dev);
}