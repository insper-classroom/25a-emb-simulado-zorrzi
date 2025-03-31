/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

 #include <stdio.h>
 #include <string.h> 
 #include "pico/stdlib.h"
 #include "hardware/gpio.h"
 #include "hardware/timer.h"
 #include "hardware/irq.h"
 
 #define TRIGGER_PIN_1 13
 #define ECHO_PIN_1 12
 
 #define TRIGGER_PIN_2 19
 #define ECHO_PIN_2 18
 
 volatile absolute_time_t echo_start_1;
 volatile absolute_time_t echo_end_1;
 
 volatile absolute_time_t echo_start_2;
 volatile absolute_time_t echo_end_2;
 
 volatile bool fim_echo_1 = false;
 volatile bool fim_echo_2 = false;
 
 volatile bool timeout_error_1 = false;
 volatile bool timeout_error_2 = false;
 
 void echo_callback(uint gpio, uint32_t events) {
     if (events & GPIO_IRQ_EDGE_RISE) {
         if(gpio == ECHO_PIN_1){
             echo_start_1 = get_absolute_time();
         }
         else if(gpio == ECHO_PIN_2){
             echo_start_2 = get_absolute_time();
         }
     }
     if (events & GPIO_IRQ_EDGE_FALL) {
         if(gpio == ECHO_PIN_1){
             echo_end_1 = get_absolute_time();
             fim_echo_1 = true;
         }
         else if(gpio == ECHO_PIN_2){
             echo_end_2 = get_absolute_time();
             fim_echo_2 = true;
         }
 
     }
 }
 
 int64_t alarm_timeout_callback_1(alarm_id_t id, void *user_data) {
     timeout_error_1 = true;
     return 0;
 }
 
 int64_t alarm_timeout_callback_2(alarm_id_t id, void *user_data) {
     timeout_error_2 = true;
     return 0;
 }
 
 void trigger_pulse_1() {
     gpio_put(TRIGGER_PIN_1, 1);
     sleep_us(10);
     gpio_put(TRIGGER_PIN_1, 0);
     timeout_error_1 = false;
 }
 
 void trigger_pulse_2() {
     gpio_put(TRIGGER_PIN_2, 1);
     sleep_us(10);
     gpio_put(TRIGGER_PIN_2, 0);
     timeout_error_1 = false;
 }
 
 
 
 int main() {
     stdio_init_all();
 
     gpio_init(TRIGGER_PIN_1);
     gpio_set_dir(TRIGGER_PIN_1, GPIO_OUT);
     gpio_put(TRIGGER_PIN_1, 0);
 
     gpio_init(ECHO_PIN_1);
     gpio_set_dir(ECHO_PIN_1, GPIO_IN);
     gpio_set_irq_enabled_with_callback(ECHO_PIN_1, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &echo_callback);
 
 
     gpio_init(TRIGGER_PIN_2);
     gpio_set_dir(TRIGGER_PIN_2, GPIO_OUT);
     gpio_put(TRIGGER_PIN_2, 0);
 
     gpio_init(ECHO_PIN_2);
     gpio_set_dir(ECHO_PIN_2, GPIO_IN);
     gpio_set_irq_enabled_with_callback(ECHO_PIN_2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &echo_callback);
 
     alarm_id_t timeout_alarm_1;
     alarm_id_t timeout_alarm_2;
 
     
     while (true) {
         trigger_pulse_1();
         trigger_pulse_2();
 
         timeout_alarm_1 = add_alarm_in_ms(30, alarm_timeout_callback_1, NULL, false);
         timeout_alarm_2 = add_alarm_in_ms(30, alarm_timeout_callback_1, NULL, false);
         sleep_ms(60);
 
         if(fim_echo_1){
             cancel_alarm(timeout_alarm_1);
             int64_t dt_1 = absolute_time_diff_us(echo_start_1, echo_end_1);
             int distancia_1 = (int) ((dt_1 * 0.0343) / 2.0);
 
             printf("Sensor 1 - dist: %d cm\n", distancia_1);
             fim_echo_1 = false;
 
         }
         else if (timeout_error_1) {
             printf("Falha\n");
         }
         sleep_ms(500);
 
 
         if(fim_echo_2){
 
             cancel_alarm(timeout_alarm_2);
             int64_t dt_2 = absolute_time_diff_us(echo_start_2, echo_end_2);
             int distancia_2 = (int) ((dt_2 * 0.0343) / 2.0);
 
             printf("Sensor 2 - dist: %d cm\n", distancia_2);
             fim_echo_2 = false;
             
         }
         else if (timeout_error_2) {
             printf("Falha\n");
         }
         sleep_ms(500);
 
 
       
     }
 
     return 0;
 }
 