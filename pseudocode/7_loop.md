# INICIALIZACION
FUNCTION setup()

    setup_motion_hardware()
    sensor_init()

    maze_init()
    set_outer_walls()

    robot_pose.x = start_x
    robot_pose.y = start_y
    robot_pose.dir = start_dir

    goal_reached = FALSE

END FUNCTION
# MAIN
FUNCTION loop()

    WHILE goal_reached == FALSE

        // 1. leer sensores + actualizar flags
        update_sensors()

        // 2. actualizar mapa
        update_maze_from_sensors()

        // 3. verificar meta
        SI is_goal_cell(robot_pose.x, robot_pose.y)
            goal_reached = TRUE
            stop_motors()
            BREAK
        FIN SI

        // 4. recalcular flood fill
        flood_fill()

        // 5. decidir a dónde ir
        target_dir = choose_next_direction(robot_pose.x, robot_pose.y)

        // 6. moverse
        execute_move(target_dir)

    END WHILE

    stop_motors()

END FUNCTION