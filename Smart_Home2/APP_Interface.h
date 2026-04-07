/*
 * APP_Interface.h
 *
 *  Created on: Sep 19, 2022
 *      Author: Hazem Mohamed
 */

#ifndef APP_INTERFACE_H_
#define APP_INTERFACE_H_

/* ---- Application Configuration ---- */
#define CREDENTIAL_LENGTH 4    /* Number of digits in username/password   */
#define CREDENTIAL_BUFF_SIZE 5 /* CREDENTIAL_LENGTH + null terminator     */
#define MAX_LOGIN_ATTEMPTS 2   /* System locks on the 3rd failed attempt  */
#define DEFAULT_USER "1234"    /* Default username                        */
#define DEFAULT_PASS "5678"    /* Default password                        */

#define FLAG_SET   1           /* Flag is active / condition is pending   */
#define FLAG_CLEAR 0           /* Flag is inactive / condition is done    */

void APP_voidReceiveUser(u8 Copy_Num);

void APP_voidReceivePassword(u8 Copy_Num);

u8 APP_u8UserCheck(u8 *ArrTrue, u8 *ArrCheck);

u8 APP_u8PassCheck(u8 *ArrTrue, u8 *ArrCheck);

#endif /* APP_INTERFACE_H_ */
