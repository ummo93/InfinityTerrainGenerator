cmake_minimum_required (VERSION 3.8)

# Hot reload for msvc
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("InfinityTerrainGenerator")

include_directories("${CMAKE_SOURCE_DIR}/libs/raylib-5.5/include")
link_directories("${CMAKE_SOURCE_DIR}/libs/raylib-5.5/lib")
include_directories("include")

file(GLOB PROJECT_SOURCES "src/*.cpp")

if(WIN32)
    if (CMAKE_BUILD_TYPE STREQUAL Release) 
        add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES})
    else()
        add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
    endif()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    target_link_libraries(${PROJECT_NAME}
        "${CMAKE_SOURCE_DIR}/libs/raylib-5.5/lib/raylib.lib"
        winmm 
        gdi32 
        opengl32 
        kernel32 
        user32 
        shell32
    )
elseif(UNIX AND NOT APPLE)
    #TODO: If Linux do not forget to download raylib for linux and put it into /libs/raylib-5.5 folder
    add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
    if (CMAKE_BUILD_TYPE STREQUAL Release) 
        set(SUBSYSTEM_LINKER_OPTIONS "-mwindows")   
    endif()
    target_link_options(${PROJECT_NAME} PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})
    target_link_libraries(${PROJECT_NAME} 
        "${CMAKE_SOURCE_DIR}/libs/raylib-5.5/lib/raylib.lib"
        GL      # OpenGL
        m       # Math library
        pthread # Threads
        dl      # Dynamic linking
        rt      # Real-time clock
        X11     # X11 window system
    )
endif()

#Copy all resources to the build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()