



add_custom_target(ui COMMAND
        vite build --outDir ${CMAKE_CURRENT_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Building UI"
        VERBATIM
)
