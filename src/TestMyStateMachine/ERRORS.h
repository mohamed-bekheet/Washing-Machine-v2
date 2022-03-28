#ifndef _ERRORS_h_
#define _ERRORS_h_

/**
 * @brief ANY ERROR IN A TASK MAY CAUSE:
 * > DEAD TASK
 * > PAUSE TASK
 * > ASK USER (WAIT FOR INPUT KEY PRESS)
 * >> USER CHOOSE > DEAD 
 *                > CONTINUE
 *                > START AGAIN (REPEAT)
 * 
 */
typedef enum{

    ERROR_ALTERNATING_NO_PROBLEM,
    ERROR_ALTERNATING_PROBLEM,

    ERROR_WATER_FULL_WATER_STILL_IN_TANK,
    ERROR_WATER_FULL_TANK_NOT_FILLED,//MAY WATER WEAK OR NO WATER
    ERROR_WATER_FULL_FILLED,
    ERROR_WATER_FILL_NO_PROBLEM,


    ERROR_TEMP_SENSOR_NOT_FOUND,
    ERROR_HEATER_IS_NOT_WORKING,
    
    ERROR_DOOR_OPEN,
    ERROR_OVERLOAD_MOTOR,
    ERROR_TANK_VIBRATION,

    
}WASHING_MACHINE_ERRORS_e;

WASHING_MACHINE_ERRORS_e AlternatingError;//

WASHING_MACHINE_ERRORS_e WaterLevelError;

#endif