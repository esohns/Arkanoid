# configure C/C++ exceptions
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../Common/scripts")
include (SetCppExceptions)

include (Threads)

include (ACE)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../yarp/scripts")
include (sdl)

if (UNIX)
 include (FindPkgConfig)
 if (SDL2_USE)
  pkg_check_modules (PKG_CONFIG_SDL2_IMAGE SDL2_image REQUIRED)
  if (PKG_CONFIG_SDL2_IMAGE_FOUND)
   set (SDL2_IMAGE_INCLUDE_DIRS ${PKG_CONFIG_SDL2_IMAGE_INCLUDE_DIRS})
   set (SDL2_IMAGE_LIBRARIES ${PKG_CONFIG_SDL2_IMAGE_LIBRARIES})
  endif (PKG_CONFIG_SDL2_IMAGE_FOUND)
 endif (SDL2_USE)

 add_link_options ("LINKER:--as-needed")
endif (UNIX)

add_executable (${PROJECT_NAME})

target_include_directories (${PROJECT_NAME} PUBLIC
                            $<BUILD_INTERFACE:${ACE_INCLUDE_DIR}>)
if (SDL_SUPPORT)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             "$<BUILD_INTERFACE:${SDL_INCLUDE_DIRS}>")
 if (SDL1_USE)
  if (WIN32)
   target_include_directories (${PROJECT_NAME} PUBLIC
                               $<BUILD_INTERFACE:${LIB_ROOT}/SDL_image/include>)
  endif (WIN32)
 elseif (SDL2_USE)
  target_include_directories (${PROJECT_NAME} PUBLIC
                              $<BUILD_INTERFACE:${SDL2_IMAGE_INCLUDE_DIRS}>)
 endif ()
endif (SDL_SUPPORT)

target_include_directories (${PROJECT_NAME} PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>)
if (WIN32)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
endif (WIN32)

############## next target ################
set (source_files
     Animation.h
     Animation.cpp
     Background.h
     Background.cpp
     Ball.h
     Ball.cpp
     Block.h
     Block.cpp
     ConfigFile.h
     ConfigFile.cpp
     defines.h
     Effect.h
     Effect.cpp
     FpsCounter.h
     FpsCounter.cpp
     Game.h
     Game.cpp
     GameObject.h
     GameObject.cpp
     Gui.h
     Gui.cpp
     main.cpp
     MapLoader.cpp
     MapLoader.h
     MenuState.cpp
     MenuState.h
     Music.cpp
     Music.h
     Platform.cpp
     Platform.h
     PlayingState.cpp
     PlayingState.h
     Projectile.cpp
     Projectile.h
     scaler.cpp
     scaler.h
     Singleton.cpp
     Singleton.h
     State.cpp
     State.h)
set (source_files
     ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_config.h
     ${source_files})
if (WIN32)
 set (source_files
      ../include/targetver.h
      ${source_files})
endif (WIN32)

# Include dir for installed targets
#set (INCLUDE_DIR include/${PROJECT_NAME}-${upstream_version})

foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
       PRIVATE ${abs_source_file}
       )
#       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
target_sources (${PROJECT_NAME}
#                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                PRIVATE ${absolute_source_files})
target_precompile_headers (${PROJECT_NAME}
                           PRIVATE stdafx.h)

#add_dependencies (${PROJECT_NAME} ACE)

target_link_libraries (${PROJECT_NAME} PUBLIC ${ACE_LIBRARY})
if (SDL_SUPPORT)
 if (SDL1_USE)
  if (WIN32)
   target_link_libraries (${PROJECT_NAME} PUBLIC legacy_stdio_definitions.lib)
  endif (WIN32)
  target_link_libraries (${PROJECT_NAME} PUBLIC ${SDL1_MAIN_LIBRARY})
 endif (SDL1_USE)
 target_link_libraries (${PROJECT_NAME} PUBLIC ${SDL_LIBRARIES})
 if (SDL1_USE)
  if (WIN32)
   target_link_libraries (${PROJECT_NAME} PUBLIC ${LIB_ROOT}/SDL_image/lib/${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}/SDL_image.lib)
  endif (WIN32)
 elseif (SDL2_USE)
  if (UNIX)
   target_link_libraries (${PROJECT_NAME} PUBLIC ${SDL2_IMAGE_LIBRARIES})
  endif (UNIX)
 endif ()
endif (SDL_SUPPORT)

##########################################

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}
        FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ############
install (FILES
         Animation.h
         Background.h
         Ball.h
         Block.h
         ConfigFile.h
         defines.h
         Effect.h
         FpsCounter.h
         Game.h
         GameObject.h
         Gui.h
         MapLoader.h
         MenuState.h
         Music.h
         Platform.h
         PlayingState.h
         Projectile.h
         scaler.h
         Singleton.h
         State.h
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT include)
install (FILES
         ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_config.h
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT include)
install (TARGETS ${PROJECT_NAME}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT bin)

################ launchers ###############

set (ARGS "1024 786")
if (UNIX)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}" COMMAND)
 create_target_launcher (${PROJECT_NAME}
                         COMMAND ${COMMAND}
                         ARGS ${ARGS}
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${ACE_LIB_PATH}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                         ENVIRONMENT ${ENVIRONMENT})
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_LIB_PATH)
 file (TO_CMAKE_PATH "${SDL_LIB_DIR}" SDL_LIB_PATH)
 if (SDL1_USE)
  file (TO_CMAKE_PATH "${LIB_ROOT}\\SDL_image\\lib\\${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}" SDL_IMAGE_LIB_PATH)
 endif (SDL1_USE)
 create_target_launcher (${PROJECT_NAME}
                         COMMAND ${COMMAND}
                         ARGS ${ARGS}
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${ACE_LIB_PATH}
                                              ${SDL_LIB_PATH} ${SDL_IMAGE_LIB_PATH}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                         ENVIRONMENT ${ENVIRONMENT})
endif ()

################ tests ###################
add_test (NAME ${PROJECT_NAME}_test
          COMMAND ${PROJECT_BINARY_DIR}/${PROJECT_NAME}
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_tests_properties (${PROJECT_NAME}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\libCommon\cmake\src\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\ui\Debug;$(ProjectDir)..\..\src\Debug;$(ProjectDir)..\..\src\modules\dev\Debug;$(ProjectDir)..\..\src\modules\vis\Debug
