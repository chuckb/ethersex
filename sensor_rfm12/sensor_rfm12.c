/*
 * Copyright (c) 2007 by Jochen Roessner <jochen@lugrot.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "../net/sensor_rfm12_state.h"
#include "../uip/uip.h"
#include "../config.h"
#include "sensor_rfm12.h"

#ifdef SENSOR_RFM12_SUPPORT

#include "lcd.h"
#include "kty81.h"

//static uint16_t sensorwert[4];
static uint8_t sensor_i = 0;
static uint8_t start = 0;
static uint8_t startok = 0;

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))
#define STATS (uip_udp_conn->appstate.sensor_rfm12)

void 
sensor_rfm12_core_init(uip_udp_conn_t *sensor_rfm12_conn)
{
  PORTD |= _BV(PD3); // Taster Pullup einschalten
  DDRB |= _BV(PB0) | _BV(PB1); //LED Alarm ausgang
  /* Init des ADC mit Taktteiler von 64 */
  ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1);
  sensor_i = SENSOR_RFM12_ADCMAX - 1;
  
  /* Aktivierung des Pin 0 (ADC0) fr die Messung 
  */
  ADMUX = sensor_i;
  lcd_init();
  char *text = "Japaadappadu :-)";
  lcd_print(text);
  lcd_goto_ddram(LCD_SECOND_LINE);
  lcd_print("Zerties.org v6 ");

}

void 
sensor_rfm12_core_periodic(void)
{
  /* Start der Konvertierung */
  ADCSRA |= _BV(ADSC);
  if((PIND & _BV(PD3)) == 0){
    start++;
  }
  
  /*  if ((rxlen = rfm12_rxdata(&rxaddr, rfdata)) > 0){
    if(rxlen == 10 && rfdata[0] > 0xF0){
      lcd_goto_ddram(LCD_SECOND_LINE+8);
      lcd_data(rfdata[2]);
      lcd_data(rfdata[3]);
      lcd_data(rfdata[4]);
      lcd_data(rfdata[5]);
      lcd_data(rfdata[6]);
      lcd_data(rfdata[7]);
      lcd_data(rfdata[8]);
      lcd_data(rfdata[9]);
    }
    if((rxlen == 2 || rxlen == 10) && rfdata[1] == 0x14){
      i2cbuf[0x1E] = rfm12_sendto(rxaddr,(unsigned char *) rfdata, (unsigned char *) &sensorwert, sizeof(sensorwert));
      
    }
    
  }
  if (kommando > 0){
      //lcd_clear();
    
    if (kommando == 8){
      lcd_goto_ddram(LCD_SECOND_LINE+8);
      lcd_data(i2cbuf[0x01]);
      lcd_data(i2cbuf[0x02]);
      lcd_data(i2cbuf[0x03]);
      lcd_data(i2cbuf[0x04]);
      lcd_data(i2cbuf[0x05]);
      lcd_data(i2cbuf[0x06]);
      lcd_data(i2cbuf[0x07]);
      lcd_data(i2cbuf[0x08]);
      i2cbuf[0x1E] = 1;
    }else
    {
      i2cbuf[0x1E] = 0xFF;
      i2cbuf[0x1E] = rfm12_sendto(i2cbuf[1],(unsigned char *) rfdata, (unsigned char *)i2cbuf+2, i2cbuf[0]-1);
    }
    kommando = 0;
  }
  */
  while (ADCSRA & _BV(ADSC));
  STATS.sensors.sensor[sensor_i].value = ADC; //sensorwert[sensor_i] = ADC;
  if(sensor_i < 2  && start != 0){
    //char textbuf[6];
    temp2text(STATS.sensors.sensor[sensor_i].valuetext, temperatur(STATS.sensors.sensor[sensor_i].value));
    lcd_goto_ddram(2 + (sensor_i * 8));
    lcd_print(STATS.sensors.sensor[sensor_i].valuetext);
  }
  if(sensor_i == 3  && start != 0){
    //char textbuf[6];
    uint16_t promille = STATS.sensors.sensor[sensor_i].value - (STATS.sensors.sensor[sensor_i].value >> 6) - (STATS.sensors.sensor[sensor_i].value >> 7) - (STATS.sensors.sensor[sensor_i].value >> 8);
    temp2text(STATS.sensors.sensor[sensor_i].valuetext, promille);
    if(promille > 610)
      PORTB |= _BV(PB0);
    if(promille < 570)
      PORTB &= ~_BV(PB0);
    if(promille < 510)
      PORTB |= _BV(PB1);
    if(promille > 530)
      PORTB &= ~_BV(PB1);
    lcd_goto_ddram(LCD_SECOND_LINE + 2);
    lcd_print(STATS.sensors.sensor[sensor_i].valuetext);
  }
  
  if(++sensor_i >= SENSOR_RFM12_ADCMAX) sensor_i = 0;
  ADMUX = sensor_i;
  
  
  if (start != startok){
    lcd_clear();
    lcd_home();
      //text[5] = i2cbuf[2];
      //_delay_ms(2);
    lcd_goto_ddram(0);
    lcd_print("T");
    lcd_goto_ddram(8);
    lcd_print("W");
    lcd_goto_ddram(LCD_SECOND_LINE);
    lcd_print("%");
    
    startok = start;
  }
  
}

void 
sensor_rfm12_setlcdtext(char *text, uint8_t len)
{
  uint8_t i;
  start=1;
  lcd_goto_ddram(LCD_SECOND_LINE+7);
  for(i = 0; i<len; i++)
  {
    lcd_data(text[i]);
  }
    //struct sensor_rfm12_request_t *REQ = uip_appdata;
		/*
		* ueberschreiben der connection info. 
		* port und adresse auf den remotehost begrenzen
		*/
    /*
			uip_ipaddr_copy(uip_udp_conn->ripaddr, BUF->srcipaddr);
			uip_udp_conn->rport = BUF->srcport;
    */


}

#endif