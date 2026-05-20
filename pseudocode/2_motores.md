# DEFINICIONES
ENUM MOTOR_DIR
    FORWARD
    BACKWARD
    BRAKE
END ENUM

STRUCT MotorCommand
    left_pwm
    right_pwm
    left_dir
    right_dir
END STRUCT

motor_cmd : MotorCommand
# INICIALIZACION
## MOTOR
FUNCTION motor_init()
    configurar PIN_MOTOR_LEFT_PWM como salida
    configurar PIN_MOTOR_LEFT_IN1 como salida
    configurar PIN_MOTOR_LEFT_IN2 como salida

    configurar PIN_MOTOR_RIGHT_PWM como salida
    configurar PIN_MOTOR_RIGHT_IN1 como salida
    configurar PIN_MOTOR_RIGHT_IN2 como salida

    stop_motors()
END FUNCTION
## ENCODER
FUNCTION encoder_init()
    configurar PIN_ENCODER_LEFT_A como entrada
    configurar PIN_ENCODER_LEFT_B como entrada
    configurar PIN_ENCODER_RIGHT_A como entrada
    configurar PIN_ENCODER_RIGHT_B como entrada

    encoder_data.left_ticks  = 0
    encoder_data.right_ticks = 0
END FUNCTION
# FUNCIONES PARA EL ENCODER
## RESET
FUNCTION reset_encoders()
    encoder_data.left_ticks  = 0
    encoder_data.right_ticks = 0
END FUNCTION
## LECTRURA SIMPLE
FUNCTION read_left_encoder()
    RETURN encoder_data.left_ticks
END FUNCTION

FUNCTION read_right_encoder()
    RETURN encoder_data.right_ticks
END FUNCTION
## EN INTERRUPCION
_la interrupcion refiere a un evento que interrumpe el loop, estas funciones hacen q el conteo de ticks no se detenga si el loop se interrumpe_
INTERRUPCION on_left_encoder_tick()
    encoder_data.left_ticks = encoder_data.left_ticks + 1
END INTERRUPCION

INTERRUPCION on_right_encoder_tick()
    encoder_data.right_ticks = encoder_data.right_ticks + 1
END INTERRUPCION
# ESCRITURA DEL MOTOR
## IZQUIERDO
FUNCTION set_left_motor(pwm, dir)
    SI dir == FORWARD
        escribir IN1 = HIGH
        escribir IN2 = LOW
    SI_NO SI dir == BACKWARD
        escribir IN1 = LOW
        escribir IN2 = HIGH
    SI_NO
        escribir IN1 = LOW
        escribir IN2 = LOW
    FIN SI

    escribir PWM al canal izquierdo
END FUNCTION
## DERECHO
FUNCTION set_right_motor(pwm, dir)
    SI dir == FORWARD
        escribir IN1 = HIGH
        escribir IN2 = LOW
    SI_NO SI dir == BACKWARD
        escribir IN1 = LOW
        escribir IN2 = HIGH
    SI_NO
        escribir IN1 = LOW
        escribir IN2 = LOW
    FIN SI

    escribir PWM al canal derecho
END FUNCTION
## GENERAL
FUNCTION set_motors(left_pwm, left_dir, right_pwm, right_dir)
    set_left_motor(left_pwm, left_dir)
    set_right_motor(right_pwm, right_dir)

    motor_cmd.left_pwm  = left_pwm
    motor_cmd.right_pwm = right_pwm
    motor_cmd.left_dir  = left_dir
    motor_cmd.right_dir = right_dir
END FUNCTION
## FRENAR
FUNCTION stop_motors()
    set_motors(0, BRAKE, 0, BRAKE)
END FUNCTION
## PRIMITIVAS DE MOVIMIENTO
### AVANZAR DERECHO
FUNCTION drive_forward_pwm(pwm)
    set_motors(pwm, FORWARD, pwm, FORWARD)
END FUNCTION
### RETROCEDER
FUNCTION drive_backward_pwm(pwm)
    set_motors(pwm, BACKWARD, pwm, BACKWARD)
END FUNCTION
### GIRAR HACIA LA IZQUIERDA
FUNCTION turn_left_pwm(pwm)
    set_motors(pwm, BACKWARD, pwm, FORWARD)
END FUNCTION
### GIRAR HACIA LA DERECHA
FUNCTION turn_right_pwm(pwm)
    set_motors(pwm, FORWARD, pwm, BACKWARD)
END FUNCTION
# FUNCION GENERAL PARA EL MOVIMIENTO
FUNCTION setup_motion_hardware()
    motor_init()
    encoder_init()
    reset_encoders()
    stop_motors()
END FUNCTION