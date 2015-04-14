
# Install web shell
OPTION( comma_INSTALL_WEB_SHELL "Install the Web Shell to the system bin" OFF )
IF( comma_INSTALL_WEB_SHELL )

    install( PROGRAMS web-shell DESTINATION ${comma_INSTALL_BIN_DIR} )
        
ENDIF( comma_INSTALL_WEB_SHELL )
