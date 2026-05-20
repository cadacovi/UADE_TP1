# 23. Seguridad y chequeos
```psc
FUNCTION safety_checks()
    IF battery_voltage_too_low() THEN
        robot.mode = ERROR
    END_IF

    IF imu_data_invalid() THEN
        robot.mode = ERROR
    END_IF

    IF encoder_left_not_updating() OR encoder_right_not_updating() THEN
        robot.mode = ERROR
    END_IF

    IF motor_driver_overcurrent_detected() THEN
        robot.mode = ERROR
    END_IF
END_FUNCTION
```
Llamar esto en el lazo de navegación o sensores.

# 24. Funciones auxiliares importantes

## 24.1. filtro pasa bajos
```psc
FUNCTION low_pass(prev, input, alpha)
    RETURN prev + alpha * (input - prev)
END_FUNCTION
```

## 24.2. normalización angular
```psc
FUNCTION normalize_angle_deg(angle)
    WHILE angle > 180
        angle = angle - 360
    END_WHILE

    WHILE angle < -180
        angle = angle + 360
    END_WHILE

    RETURN angle
END_FUNCTION
```

## 24.3. promedio ADC
```psc
FUNCTION average_adc_left(samples)
    sum = 0
    FOR i FROM 1 TO samples
        sum = sum + read_adc_left()
    END_FOR
    RETURN sum / samples
END_FUNCTION
```