IF (UNIX)
    OPTION( INSTALL_BASH_COMPLETION "Install the BASH completion scripts." OFF )
    IF( INSTALL_BASH_COMPLETION )
        INSTALL(FILES comma DESTINATION /etc/bash_completion.d )
    ENDIF()
ENDIF()

