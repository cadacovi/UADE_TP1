# 12. Avance recto

* La idea:
avanzar con perfil de velocidad
mantener heading con gyro
corregir centrado lateral si hay paredes útiles
```psc
FUNCTION straight_motion_controller()
    distance_error = robot.target_distance_mm - odom.cell_progress_mm

    target_speed_profile = trapezoidal_profile(distance_error,
                                               MAX_LINEAR_SPEED_MM_S,
                                               MAX_LINEAR_ACCEL_MM_S2)

    heading_error = normalize_angle_deg(robot.target_heading_deg - imu.yaw_deg)

    heading_correction = PID_heading(heading_error)

    lateral_correction = 0

    IF ir.left_wall_detected AND ir.right_wall_detected THEN
        wall_error = ir.filtered_left - ir.filtered_right
        lateral_correction = PID_lateral(wall_error)
    ELSE IF ir.left_wall_detected THEN
        wall_error = ir.filtered_left - LEFT_REFERENCE_VALUE
        lateral_correction = PID_lateral(wall_error)
    ELSE IF ir.right_wall_detected THEN
        wall_error = RIGHT_REFERENCE_VALUE - ir.filtered_right
        lateral_correction = PID_lateral(wall_error)
    END_IF

    robot.target_linear_speed_mm_s = target_speed_profile
    robot.target_angular_speed_dps = heading_correction + lateral_correction

    IF distance_error <= 0 THEN
        robot.motion_state = BRAKING
    END_IF

    IF tof.front_wall_detected THEN
        IF tof.filtered_mm < FRONT_STOP_TARGET_MM THEN
            robot.motion_state = BRAKING
        END_IF
    END_IF
END_FUNCTION
```

# 13. Frenado
```psc
FUNCTION braking_controller()
    robot.target_linear_speed_mm_s =
        ramp_towards(robot.target_linear_speed_mm_s, 0, MAX_LINEAR_ACCEL_MM_S2)

    robot.target_angular_speed_dps =
        ramp_towards(robot.target_angular_speed_dps, 0, MAX_ANGULAR_ACCEL_DPS2)

    IF abs(robot.target_linear_speed_mm_s) < small_speed_threshold
       AND abs(encoder_left.velocity_mm_s) < stop_threshold
       AND abs(encoder_right.velocity_mm_s) < stop_threshold THEN

        robot.motion_state = STOPPED
    END_IF
END_FUNCTION
```

# 14. Giro 90° / 180°

## 14.1. iniciar giro
```psc
FUNCTION begin_turn_left_90()
    robot.motion_state = TURNING_LEFT_90
    robot.target_heading_deg = normalize_angle_deg(imu.yaw_deg + 90)
END_FUNCTION

FUNCTION begin_turn_right_90()
    robot.motion_state = TURNING_RIGHT_90
    robot.target_heading_deg = normalize_angle_deg(imu.yaw_deg - 90)
END_FUNCTION

FUNCTION begin_turn_180()
    robot.motion_state = TURNING_180
    robot.target_heading_deg = normalize_angle_deg(imu.yaw_deg + 180)
END_FUNCTION
```

## 14.2. controlador de giro
```psc
FUNCTION turn_controller(target_heading_deg)
    angle_error = normalize_angle_deg(target_heading_deg - imu.yaw_deg)

    angular_cmd = PID_turn(angle_error)

    angular_cmd = limit(angular_cmd, -MAX_ANGULAR_SPEED_DPS, MAX_ANGULAR_SPEED_DPS)

    robot.target_linear_speed_mm_s = 0
    robot.target_angular_speed_dps = angular_cmd

    IF abs(angle_error) < 2.0 AND abs(imu.gyro_z_dps) < 10 THEN
        robot.motion_state = BRAKING
    END_IF
END_FUNCTION
```

# 15. Alineación frontal

Útil al llegar a una pared frontal antes de girar o al iniciar carrera.
```psc
FUNCTION begin_front_alignment()
    robot.motion_state = ALIGNING_FRONT
END_FUNCTION

FUNCTION front_align_controller()
    IF NOT tof.valid THEN
        robot.motion_state = STOPPED
        RETURN
    END_IF

    distance_error = tof.filtered_mm - FRONT_STOP_TARGET_MM

    cmd = PID_front(distance_error)

    cmd = limit(cmd, -200, 200)

    robot.target_linear_speed_mm_s = cmd
    robot.target_angular_speed_dps = 0

    IF abs(distance_error) < 3 THEN
        robot.motion_state = STOPPED
    END_IF
END_FUNCTION
```

# 16. Primitivas de movimiento

Estas funciones bloquean a nivel lógico, no necesariamente a nivel CPU. En firmware real se implementan como espera por estado.

## 16.1. avanzar una distancia

```psc
FUNCTION start_move_straight(distance_mm)
    odom.cell_progress_mm = 0
    robot.target_distance_mm = distance_mm
    robot.target_heading_deg = imu.yaw_deg
    reset_heading_pid()
    reset_lateral_pid()
    robot.motion_state = MOVING_STRAIGHT
END_FUNCTION
```

## 16.2. avanzar media celda
```psc
FUNCTION move_half_cell()
    start_move_straight(HALF_CELL_MM)
    wait_until_motion_complete()
END_FUNCTION
```

## 16.3. avanzar una celda
```psc
FUNCTION move_one_cell()
    start_move_straight(CELL_SIZE_MM)
    wait_until_motion_complete()
END_FUNCTION
```

## 16.4. girar 90°
```psc
FUNCTION turn_left_90()
    begin_turn_left_90()
    wait_until_motion_complete()
END_FUNCTION

FUNCTION turn_right_90()
    begin_turn_right_90()
    wait_until_motion_complete()
END_FUNCTION
```

## 16.5. girar 180°
```psc
FUNCTION turn_back_180()
    begin_turn_180()
    wait_until_motion_complete()
END_FUNCTION
```