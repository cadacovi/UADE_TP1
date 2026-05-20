# ALGORITMO
INICIO
    FUNCION setup():
        inicializar_pines()
        inicializar_driver_motores()
        inicializar_ultrasonicos()
        inicializar_encoders()
        inicializar_imu()
        inicializar_servo()
        inicializar_comunicacion()

        robot.posicion_actual = Celda(0, 0)
        robot.direccion_actual = NORTE
        robot.estado_actual = IDLE
        robot.carga_entregada = FALSO
        robot.comando_descarga_manual = FALSO

        inicializar_mapa(robot.mapa)
        marcar_celda(robot.posicion_actual, VISITADA)
    FIN FUNCION

    FUNCION loop():
        leer_comandos()

        SI robot.comando_descarga_manual == VERDADERO ENTONCES
            detener_motores()
            descargar_carga()
            robot.comando_descarga_manual = FALSO
        FIN_SI

        SEGUN robot.estado_actual HACER

            CASO IDLE:
                SI hay_mision_nueva() ENTONCES
                    robot.destino = obtener_destino_como_celda()
                    robot.estado_actual = PLANIFICANDO_RUTA
                FIN_SI
            FIN_CASO

            CASO PLANIFICANDO_RUTA:
                actualizar_mapa_local()
                robot.ruta_actual = planificar_ruta_BFS(
                    robot.posicion_actual,
                    robot.destino,
                    robot.mapa
                )

                SI ruta_vacia(robot.ruta_actual) ENTONCES
                    robot.estado_actual = ERROR
                    reportar("Destino inalcanzable")
                SINO
                    robot.estado_actual = NAVEGANDO
                FIN_SI
            FIN_CASO

            CASO NAVEGANDO:
                navegar_hacia_destino()
            FIN_CASO

            CASO EVITANDO_OBSTACULO:
                manejar_obstaculo()
            FIN_CASO

            CASO DESCARGANDO:
                descargar_carga()
                robot.estado_actual = MISION_COMPLETADA
            FIN_CASO

            CASO MISION_COMPLETADA:
                detener_motores()
                reportar("Mision completada")
                robot.estado_actual = IDLE
            FIN_CASO

            CASO ERROR:
                detener_motores()
                reportar("Robot en estado de error")
                esperar_reinicio_o_nueva_mision()
            FIN_CASO

        FIN_SEGUN
    FIN FUNCION
FIN