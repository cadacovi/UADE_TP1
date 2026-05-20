# FUNCIONES DE MOVIMIENTO
## AVANZAR UNA CELDA
FUNCTION move_forward_one_cell()

    reset_encoders()

    WHILE TRUE
        left  = read_left_encoder()
        right = read_right_encoder()

        avg_ticks = (left + right) / 2
// correccion del motor por si una rueda gira mas que la otra con la misma senhal de pwm base
        error = left - right

        correction = Kp * error (Kp es una constante de ajuste llamada ganancia proporcional y su valor se determina empiricamente, empezar probando con un valor chico ~0.3)

        left_pwm  = PWM_BASE_SPEED - correction
        right_pwm = PWM_BASE_SPEED + correction

        set_motors(left_pwm, FORWARD, right_pwm, FORWARD)
//
        SI avg_ticks >= ENCODER_TICKS_PER_CELL
            BREAK
        FIN SI
    END WHILE
// actualizacion de posicion
    robot_pose.x = robot_pose.x + dx[robot_pose.dir]
    robot_pose.y = robot_pose.y + dy[robot_pose.dir]
//
    stop_motors()

END FUNCTION
## GIRAR 90 GRADOS
FUNCTION turn_left_90()

    reset_encoders()

    turn_left_pwm(PWM_TURN_SPEED)

    WHILE TRUE
        left  = read_left_encoder()
        right = read_right_encoder()

        avg_ticks = (ABS(left) + ABS(right)) / 2

        SI avg_ticks >= ENCODER_TICKS_TURN_90
            BREAK
        FIN SI
    END WHILE

        robot_pose.dir = (robot_pose.dir + 3) mod 4

    stop_motors()

END FUNCTION
(...)
## 180 GRADOS
FUNCTION turn_back_180()
    turn_left_90()
    turn_left_90()
END FUNCTION
## FUNCION GENERAL EN FUNCION DE LA DIRECCION DE MOVIMIENTO
FUNCTION execute_move(target_dir)

    SI target_dir == robot_pose.dir
        move_forward_one_cell()

    SI target_dir == (robot_pose.dir + 1)
        turn_right_90()
        move_forward_one_cell()

    SI target_dir == (robot_pose.dir + 3)
        turn_left_90()
        move_forward_one_cell()

    SI target_dir == (robot_pose.dir + 2)
        turn_back_180()
        move_forward_one_cell()

END FUNCTION