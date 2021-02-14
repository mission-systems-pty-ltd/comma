if( BUILD_SHARED_LIBS )
    install( CODE "execute_process( COMMAND /sbin/ldconfig )" )
endif( BUILD_SHARED_LIBS )

