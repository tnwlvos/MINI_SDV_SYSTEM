/*
 * hal_lcd.c
 *
 * Created: 2025-12-09 오전 10:56:22
 *  Author: LEESANGHUN
 */ 
//
//// 값이 바뀐 경우에만 서브로 전송 (괜히 계속 보내지 말고)
//if (motor_mode != last_motor_mode) {
	//send_speed_to_sub_uart1(&motor_mode);
	//last_motor_mode = motor_mode;
//}
//
//if(rx_complete_flag)
//{
	//rx_complete_flag=false;
	//sub_rx_ultrasonic_uart1(&distance);
	//LCD_Pos(0,0);
	//LCD_Str("Measured Dist=");
	//sprintf(Message,"%01d   %03d cm",sizeof(distance) ,distance);
	//LCD_Pos(1,5);
	//LCD_Str(Message);
	//
//}