# 8. Lectura de sensores

## 8.1. lazo de sensores
```psc
FUNCTION sensor_loop_200hz()
    read_ir_sensors_differential()
    filter_ir_sensors()

    read_tof()
    filter_tof()

    detect_walls()
END_FUNCTION
```

## 8.2. IR diferencial
```psc
FUNCTION read_ir_sensors_differential()
    // izquierda
    set_left_ir_led(OFF)
    wait_us(150)
    ir.ambient_left_sample = average_adc_left(IR_SAMPLES)

    set_left_ir_led(ON)
    wait_us(150)
    ir.active_left_sample = average_adc_left(IR_SAMPLES)

    ir.signal_left = ir.active_left_sample - ir.ambient_left_sample

    // derecha
    set_right_ir_led(OFF)
    wait_us(150)
    ir.ambient_right_sample = average_adc_right(IR_SAMPLES)

    set_right_ir_led(ON)
    wait_us(150)
    ir.active_right_sample = average_adc_right(IR_SAMPLES)

    ir.signal_right = ir.active_right_sample - ir.ambient_right_sample

    set_left_ir_led(OFF)
    set_right_ir_led(OFF)
END_FUNCTION
```

## 8.3. filtro IR
```psc
FUNCTION filter_ir_sensors()
    ir.filtered_left  = low_pass(ir.filtered_left,  ir.signal_left,  alpha_ir)
    ir.filtered_right = low_pass(ir.filtered_right, ir.signal_right, alpha_ir)
END_FUNCTION
```

## 8.4. Read TOF
```psc
FUNCTION read_tof()
    tof.raw_mm = vl53l0x_read_distance()
    tof.valid = is_valid_tof_reading(tof.raw_mm)
END_FUNCTION

FUNCTION filter_tof()
    IF tof.valid THEN
        tof.filtered_mm = low_pass(tof.filtered_mm, tof.raw_mm, alpha_tof)
    END_IF
END_FUNCTION
```

## 8.5. deteccion de paredes
```psc
FUNCTION detect_walls()
    ir.left_wall_detected  = (ir.filtered_left  > SIDE_WALL_THRESHOLD_LEFT)
    ir.right_wall_detected = (ir.filtered_right > SIDE_WALL_THRESHOLD_RIGHT)

    tof.front_wall_detected = FALSE
    IF tof.valid AND tof.filtered_mm < FRONT_WALL_THRESHOLD_MM THEN
        tof.front_wall_detected = TRUE
    END_IF
END_FUNCTION
```

# 9. Lectura rápida de encoders e IMU

* lazo de control rapido
```psc
FUNCTION control_loop_1khz()
    read_imu_fast()
    update_yaw_from_gyro()

    read_encoders_fast()
    update_odometry()

    update_motion_controller()
    apply_motor_output()
END_FUNCTION
```

## 9.1. IMU
```psc
FUNCTION read_imu_fast()
    read_mpu6050_raw()

    imu.gyro_z_dps = imu.gyro_z_dps - imu.gyro_bias_z
END_FUNCTION

FUNCTION update_yaw_from_gyro()
    imu.yaw_deg = imu.yaw_deg + imu.gyro_z_dps * CONTROL_DT_MS / 1000.0
    imu.yaw_deg = normalize_angle_deg(imu.yaw_deg)
END_FUNCTION
```

## 9.2. AS5600
```psc
FUNCTION read_encoders_fast()
    update_encoder_left()
    update_encoder_right()
END_FUNCTION

FUNCTION update_encoder_left()
    encoder_left.raw_angle_deg = read_as5600_left_angle()

    delta = encoder_left.raw_angle_deg - encoder_left.prev_raw_angle_deg

    IF delta > 180 THEN
        delta = delta - 360
    END_IF

    IF delta < -180 THEN
        delta = delta + 360
    END_IF

    encoder_left.delta_angle_deg = delta
    encoder_left.total_angle_deg = encoder_left.total_angle_deg + delta

    encoder_left.angular_velocity_dps = delta / (CONTROL_DT_MS / 1000.0)

    encoder_left.distance_mm =
        (encoder_left.total_angle_deg / 360.0) * WHEEL_CIRCUMFERENCE_MM

    encoder_left.velocity_mm_s =
        (encoder_left.angular_velocity_dps / 360.0) * WHEEL_CIRCUMFERENCE_MM

    encoder_left.prev_raw_angle_deg = encoder_left.raw_angle_deg
END_FUNCTION

FUNCTION update_encoder_right()
    encoder_right.raw_angle_deg = read_as5600_right_angle()

    delta = encoder_right.raw_angle_deg - encoder_right.prev_raw_angle_deg

    IF delta > 180 THEN
        delta = delta - 360
    END_IF

    IF delta < -180 THEN
        delta = delta + 360
    END_IF

    encoder_right.delta_angle_deg = delta
    encoder_right.total_angle_deg = encoder_right.total_angle_deg + delta

    encoder_right.angular_velocity_dps = delta / (CONTROL_DT_MS / 1000.0)

    encoder_right.distance_mm =
        (encoder_right.total_angle_deg / 360.0) * WHEEL_CIRCUMFERENCE_MM

    encoder_right.velocity_mm_s =
        (encoder_right.angular_velocity_dps / 360.0) * WHEEL_CIRCUMFERENCE_MM

    encoder_right.prev_raw_angle_deg = encoder_right.raw_angle_deg
END_FUNCTION
```

# 10. Odometría
```psc
FUNCTION update_odometry()
    left_now  = encoder_left.distance_mm
    right_now = encoder_right.distance_mm

    dL = left_now  - odom.prev_left_distance_mm
    dR = right_now - odom.prev_right_distance_mm

    odom.prev_left_distance_mm  = left_now
    odom.prev_right_distance_mm = right_now

    dC = (dL + dR) / 2.0

    // orientación principal desde gyro
    odom.theta_deg = imu.yaw_deg

    theta_rad = deg_to_rad(odom.theta_deg)

    odom.x_mm = odom.x_mm + dC * cos(theta_rad)
    odom.y_mm = odom.y_mm + dC * sin(theta_rad)

    odom.linear_distance_total_mm = odom.linear_distance_total_mm + abs(dC)
    odom.cell_progress_mm = odom.cell_progress_mm + dC
END_FUNCTION
```

# 11. Control de motores

* separaría el control en dos niveles:
generas una consigna de velocidad lineal y angular
la conviertes a velocidad izquierda/derecha
cada rueda tiene su propio control

## 11.1. mezcla diferencial
```psc
FUNCTION compute_wheel_speed_targets(target_linear_mm_s, target_angular_dps)
    omega_rad_s = deg_to_rad(target_angular_dps)

    v_left  = target_linear_mm_s - omega_rad_s * (WHEEL_BASE_MM / 2.0)
    v_right = target_linear_mm_s + omega_rad_s * (WHEEL_BASE_MM / 2.0)

    RETURN v_left, v_right
END_FUNCTION
```

## 11.2. update_motion_controller
```psc
FUNCTION update_motion_controller()
    SWITCH robot.motion_state

        CASE STOPPED:
            robot.target_linear_speed_mm_s = 0
            robot.target_angular_speed_dps = 0

        CASE MOVING_STRAIGHT:
            straight_motion_controller()

        CASE TURNING_LEFT_90:
            turn_controller(robot.target_heading_deg)

        CASE TURNING_RIGHT_90:
            turn_controller(robot.target_heading_deg)

        CASE TURNING_180:
            turn_controller(robot.target_heading_deg)

        CASE ALIGNING_FRONT:
            front_align_controller()

        CASE BRAKING:
            braking_controller()
    END_SWITCH

    vL_target, vR_target =
        compute_wheel_speed_targets(robot.target_linear_speed_mm_s,
                                    robot.target_angular_speed_dps)

    pwmL = wheel_speed_pid_left(vL_target, encoder_left.velocity_mm_s)
    pwmR = wheel_speed_pid_right(vR_target, encoder_right.velocity_mm_s)

    robot.left_motor_pwm  = saturate_pwm(pwmL)
    robot.right_motor_pwm = saturate_pwm(pwmR)
END_FUNCTION
```