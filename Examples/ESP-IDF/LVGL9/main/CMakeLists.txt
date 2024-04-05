idf_component_register(SRCS demo.c lcd.c touch.c
                    INCLUDE_DIRS "."
                    REQUIRES esp_lcd driver 
                    )
