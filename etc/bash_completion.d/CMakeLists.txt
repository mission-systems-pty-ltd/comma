IF (UNIX)
    OPTION( INSTALL_BASH_COMPLETION "Install the BASH completion scripts." ON )
    SET( INSTALL_BASH_COMPLETION_DIR /etc/bash_completion.d
         CACHE PATH "Location in which to install the bash completion scripts.")
    IF( INSTALL_BASH_COMPLETION )
        INSTALL(FILES comma DESTINATION ${INSTALL_BASH_COMPLETION_DIR} )
    ENDIF()
ENDIF()

