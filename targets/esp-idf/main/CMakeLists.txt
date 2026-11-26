idf_component_register(
    SRCS "app_main.cpp" "failsafe.cpp"
    INCLUDE_DIRS "."
    REQUIRES dalhal esp_http_server
)
