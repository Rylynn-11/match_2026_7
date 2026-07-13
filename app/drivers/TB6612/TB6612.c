//
// Created by m1816 on 26-7-10.
//

#include "TB6612.h"
#include "main.h"

//电机的配置结构体
//根据电机的数量，分别配置每个电机的定时器，通道，两个GPIO端口和引脚；
//若需修改电机配置，直接修改该结构体，调用电机配置，也可直接调用该结构体；
motor_config Motor_Config[motor_count] = {
    {&htim1,TIM_CHANNEL_1,GPIOE,GPIO_PIN_7,GPIOE,GPIO_PIN_8 },
    {&htim1,TIM_CHANNEL_2,GPIOE,GPIO_PIN_12,GPIOE,GPIO_PIN_13 },
};


void motor_init(void) {
    uint32_t i;

    for (i = 0; i < motor_count; i++) {
        HAL_TIM_PWM_Start(Motor_Config[i].htim,Motor_Config[i].channel);
        motor_stop(Motor_Config[i]);
    }
}

void motor_set_speed(motor_config Motor_Config,float speed) {
    uint32_t compare;
    if (Motor_Config.htim == NULL) {
        return;
    }

    if (speed < 0.0f) {
        speed = 0.0f;
    } else if (speed > 1000.0f) {
        speed = 1000.0f;
    }
    compare = (uint32_t)(speed * 10.0f);
    __HAL_TIM_SET_COMPARE(Motor_Config.htim,Motor_Config.channel,compare);
}

void set_direction(motor_config Motor_Config,Motor_Direction direction) {
    switch(direction) {
        case CW:
            HAL_GPIO_WritePin(Motor_Config.in1_port, Motor_Config.in1_pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(Motor_Config.in2_port, Motor_Config.in2_pin, GPIO_PIN_RESET);
            break;
        case CCW:
            HAL_GPIO_WritePin(Motor_Config.in1_port, Motor_Config.in1_pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(Motor_Config.in2_port, Motor_Config.in2_pin, GPIO_PIN_SET);
            break;
        default:
            HAL_GPIO_WritePin(Motor_Config.in1_port, Motor_Config.in1_pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(Motor_Config.in2_port, Motor_Config.in2_pin, GPIO_PIN_RESET);
            break;
    }
}

void motor_stop(motor_config Motor_Config) {
    motor_set_speed(Motor_Config,0.0f);
    HAL_GPIO_WritePin(Motor_Config.in1_port, Motor_Config.in1_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Motor_Config.in2_port, Motor_Config.in2_pin, GPIO_PIN_RESET);
}

void motor_brake(motor_config Motor_Config) {
    motor_set_speed(Motor_Config,0.0f);
    HAL_GPIO_WritePin(Motor_Config.in1_port, Motor_Config.in1_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(Motor_Config.in2_port, Motor_Config.in2_pin, GPIO_PIN_SET);
}
