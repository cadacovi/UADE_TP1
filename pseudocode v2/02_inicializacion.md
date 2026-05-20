# 6. Inicialización

## 6.1. main
```psc
FUNCTION main()
    init_hardware()
    init_variables()
    init_maze()

    robot.mode = BOOT

    LOOP FOREVER
        scheduler_update()

        IF task_control_due THEN
            control_loop_1khz()
        END_IF

        IF task_sensors_due THEN
            sensor_loop_200hz()
        END_IF

        IF task_nav_due THEN
            navigation_loop_50hz()
        END_IF
    END_LOOP
END_FUNCTION
```

## 6.2. init_hardware
```psc
FUNCTION init_hardware()
    init_gpio()
    init_pwm_motors()
    init_adc()
    init_i2c_as5600_left()
    init_i2c_as5600_right()
    init_i2c_tof_and_imu() //EL ESP32 S3 SOLO TIENE SOPORTE PARA 2 BUSES I2C, NO 3

    init_mpu6050()
    init_vl53l0x()
    init_as5600_left()
    init_as5600_right()

    stop_motors()
END_FUNCTION
```

## 6.3. init_variables
``` psc
FUNCTION init_variables()
    zero_all_sensor_structs()
    zero_all_odom()
    zero_all_pid()

    robot.mode = CALIBRATION
    robot.motion_state = STOPPED
    robot.run_requested = FALSE
    robot.calibrated = FALSE
    robot.error_flag = FALSE

    configure_pid_values()
END_FUNCTION
```

# 7. Calibración

## 7.1. calibración general
```psc
FUNCTION perform_calibration()
    stop_motors()

    calibrate_gyro_bias()
    calibrate_ir_ambient_levels()
    calibrate_encoder_zero_reference()
    validate_tof()

    robot.calibrated = TRUE
    robot.mode = IDLE
END_FUNCTION
```

## 7.2. bias del gyro
```psc
FUNCTION calibrate_gyro_bias()
    sum = 0

    FOR i FROM 1 TO GYRO_BIAS_SAMPLES
        read_mpu6050_raw()
        sum = sum + imu.gyro_z_dps
        wait_ms(2)
    END_FOR

    imu.gyro_bias_z = sum / GYRO_BIAS_SAMPLES
END_FUNCTION
```

## 7.3. calibración IR
```psc
FUNCTION calibrate_ir_ambient_levels()
    sum_left = 0
    sum_right = 0

    FOR i FROM 1 TO IR_CALIB_SAMPLES
        ir_leds_off()
        wait_us(200)

        sum_left  = sum_left  + read_adc_left()
        sum_right = sum_right + read_adc_right()

        wait_ms(2)
    END_FOR

    ir.ambient_left  = sum_left / IR_CALIB_SAMPLES
    ir.ambient_right = sum_right / IR_CALIB_SAMPLES
END_FUNCTION
```

## 7.4. referencia inicial de encoder
```psc
FUNCTION calibrate_encoder_zero_reference()
    encoder_left.raw_angle_deg  = read_as5600_left_angle()
    encoder_right.raw_angle_deg = read_as5600_right_angle()

    encoder_left.prev_raw_angle_deg  = encoder_left.raw_angle_deg
    encoder_right.prev_raw_angle_deg = encoder_right.raw_angle_deg

    encoder_left.total_angle_deg  = 0
    encoder_right.total_angle_deg = 0

    encoder_left.distance_mm  = 0
    encoder_right.distance_mm = 0
END_FUNCTION
```