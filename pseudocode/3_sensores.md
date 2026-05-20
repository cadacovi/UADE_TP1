# INICIALIZACION
FUNCTION sensor_init()
    configurar PIN_SENSOR_LEFT  como entrada analógica
    configurar PIN_SENSOR_FRONT como entrada analógica
    configurar PIN_SENSOR_RIGHT como entrada analógica
END FUNCTION
# LECTURA
FUNCTION read_sensor_left()
    RETURN analogRead(PIN_SENSOR_LEFT)
END FUNCTION
(...)
FUNCTION sensor_read_all()
    sensor_data.left_raw  = read_sensor_left()
    sensor_data.front_raw = read_sensor_front()
    sensor_data.right_raw = read_sensor_right()
END FUNCTION
# INTERPRETACION
FUNCTION detect_wall_left()
    SI sensor_data.left_raw > SENSOR_THRESHOLD_LEFT
        RETURN TRUE
    SI_NO
        RETURN FALSE
END FUNCTION
(...)
# GENERAL
FUNCTION update_sensors()
    sensor_read_all()

    sensor_data.wall_left  = detect_wall_left()
    sensor_data.wall_front = detect_wall_front()
    sensor_data.wall_right = detect_wall_right()
END FUNCTION