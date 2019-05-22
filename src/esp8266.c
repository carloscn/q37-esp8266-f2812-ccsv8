/*
 * esp8266.c
 *
 *  Created on:
 *      Author: delvis
 */
#include "esp8266.h"
bool strcmp_user(char *str1, char *str2, int len);
inline void esp8266_write_reg(const char *cmd);
inline void esp8266_write_str(const char *cmd);
inline void esp8266_write_byte(char e);
const char* cmd_at_mode = "AT\r\n";
const char* cmd_at_hoc_mode = "AT+CWMODE=3\r\n";
const char* cmd_at_connect_to_wifi = "AT+CWLAP\r\n";
const char* cmd_at_user_ssid_passwd = "AT+CWJAP=\"lienin\",\"11115556\"\r\n";
const char* cmd_at_air_mode = "AT+CIPMODE=1\r\n";
const char* cmd_at_start = "AT+CIPSEND\r\n";
const char* cmd_at_set_ip = "AT+CIPSTART=\"TCP\",\"192.168.155.4\",8080\r\n";
const char* cmd_at_reset = "AT_RST\r\n";
bool is_enable_send = false;
char rx_buffer[50];
uint16_t rx_count = 0;

void esp8266_init(void)
{
    esp8266_write_reg(cmd_at_mode);
    esp8266_write_reg(cmd_at_hoc_mode);
    esp8266_write_reg(cmd_at_connect_to_wifi);
    esp8266_write_reg(cmd_at_user_ssid_passwd);
    DELAY_MS(2000);
    esp8266_write_reg(cmd_at_air_mode);
    esp8266_write_reg(cmd_at_set_ip);
    esp8266_write_reg(cmd_at_start);
}

void esp8266_reset(void)
{
    esp8266_write_reg(cmd_at_reset);
}

inline void esp8266_write_reg(const char *cmd)
{
    scia_msg(cmd);
    DELAY_MS(2000);
}

inline void esp8266_write_str(const char *cmd)
{
    scia_msg(cmd);
}

inline void esp8266_write_byte(char e)
{
    scia_xmit(e);
}

void esp8266_write_to_host(float *data)
{
    union {
        uint16_t all;
        unsigned char bit[2];
    } value;
    // send the head of frame.
    esp8266_write_byte('&');
    esp8266_write_byte('&');
    // send temp sign;
    if (data[0] < 0)
        esp8266_write_byte((char)0x2D);
    else
        esp8266_write_byte((char)0x2B);
    //send temp value;
    value.all = (uint16_t) (data[0] * 100);
    esp8266_write_byte(value.bit[0]);
    esp8266_write_byte(value.bit[1]);
    // send rotation speed;
    value.all = (uint16_t) (data[1] * 1);
    esp8266_write_byte(value.bit[0]);
    esp8266_write_byte(value.bit[1]);
    // send I value
    value.all = (uint16_t) (data[2] * 1000);
    esp8266_write_byte(value.bit[0]);
    esp8266_write_byte(value.bit[1]);
    // send U value.
    value.all = (uint16_t) (data[3] * 1000);
    esp8266_write_byte(value.bit[0]);
    esp8266_write_byte(value.bit[1]);
    // send the tail of frame;
    esp8266_write_byte('*');
    esp8266_write_byte('*');
}
// &star
void esp8266_read_signal(char e)
{
    char cmd_1[] = "&start";
    char cmd_2[] = "&stops";
    char temp[7];
    memset(temp,'\n',7);
    rx_buffer[rx_count] = e;
    if (rx_count == 0 && rx_buffer[0] != '&') {
        memset(rx_buffer, 0, 50);
        return;
    }
    rx_count ++;
    if (rx_count == 6 ) {
        memcpy(temp, rx_buffer,6);
        if ( strcmp_user(temp, cmd_1, 6) == true ) {
            is_enable_send = true;
        }
        if ( strcmp_user(temp, cmd_2, 6) == true ) {
            is_enable_send = false;
        }
        if ( is_enable_send == true ) {
            printf("enable!\n");
        } else {
            printf("disble!\n");
        }
        rx_count = 0;
        memset(rx_buffer, 0, 50);
    }

}

bool strcmp_user(char *str1,char *str2, int len)
{
    int i = 0;

    for (i = 0; i < len; i ++) {
        if (str1[i] != str2[i] )
            return false;
    }
    return true;
}


