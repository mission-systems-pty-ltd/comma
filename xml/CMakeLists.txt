SET( PROJECT "xml" )
SET( TARGET_NAME comma_${PROJECT} )

if( comma_BUILD_APPLICATIONS )
    add_subdirectory( applications )
endif( comma_BUILD_APPLICATIONS )
