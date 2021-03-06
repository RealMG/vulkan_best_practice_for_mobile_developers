#[[
 Copyright (c) 2019, Arm Limited and Contributors

 SPDX-License-Identifier: MIT

 Permission is hereby granted, free of charge,
 to any person obtaining a copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 ]]

set(SCRIPT_DIR ${CMAKE_CURRENT_LIST_DIR})

function(generate_sample_entrypoint)
    set(options)
    set(oneValueArgs ID NAME OUTPUT_DIR)
    set(multiValueArgs)

    cmake_parse_arguments(TARGET "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(SAMPLE_NAME ${TARGET_NAME})

    set(CONFIG_FILE ${SCRIPT_DIR}/template/sample_main.cpp.in)

    if(EXISTS ${CONFIG_FILE})
        configure_file(${CONFIG_FILE} ${TARGET_OUTPUT_DIR}/main.cpp)
    else()
        message(FATAL_ERROR "No template file exists at `${CONFIG_FILE}`")
    endif()
endfunction()

function(generate_samples_header)
    set(options)
    set(oneValueArgs OUTPUT_DIR)
    set(multiValueArgs SAMPLE_ID_LIST)

    cmake_parse_arguments(TARGET "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(SAMPLE_INCLUDE_FILES)
    set(SAMPLE_NAME_FUNC_PAIRS)
    set(SAMPLE_INFO_LIST)

    foreach(SAMPLE_ID ${TARGET_SAMPLE_ID_LIST})
        if (${VKB_${SAMPLE_ID}} AND TARGET ${SAMPLE_ID})
            get_target_property(SAMPLE_NAME ${SAMPLE_ID} SAMPLE_NAME)
            get_target_property(SAMPLE_DESCRIPTION ${SAMPLE_ID} SAMPLE_DESCRIPTION)

            list(APPEND SAMPLE_INFO_LIST "\tSampleInfo{\"${SAMPLE_ID}\", \"${SAMPLE_NAME}\"\, \"${SAMPLE_DESCRIPTION}\"},")
            list(APPEND SAMPLE_INCLUDE_FILES "#include \"${SAMPLE_ID}/${SAMPLE_ID}.h\"")
            list(APPEND SAMPLE_NAME_FUNC_PAIRS "\t{ \"${SAMPLE_ID}\", create_${SAMPLE_ID} },")
        endif()
    endforeach()

    string_join(
        GLUE "\n"
        INPUT ${SAMPLE_INFO_LIST}
        OUTPUT SAMPLE_INFO_LIST)

    string_join(
        GLUE "\n"
        INPUT ${SAMPLE_INCLUDE_FILES}
        OUTPUT SAMPLE_INCLUDE_FILES)

    string_join(
        GLUE "\n"
        INPUT ${SAMPLE_NAME_FUNC_PAIRS}
        OUTPUT SAMPLE_NAME_FUNC_PAIRS)

    set(CONFIG_FILE ${SCRIPT_DIR}/template/samples.h.in)

    if(EXISTS ${CONFIG_FILE})
        configure_file(${CONFIG_FILE} ${TARGET_OUTPUT_DIR}/samples.h)
    else()
        message(FATAL_ERROR "No template file exists at `${CONFIG_FILE}`")
    endif()
endfunction()

function(add_sample_project)
    set(options)  
    set(oneValueArgs ID NAME DESCRIPTION)
    set(multiValueArgs FILES)

    cmake_parse_arguments(TARGET "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set("VKB_${TARGET_ID}" ON CACHE BOOL "Build sample ${TARGET_ID}")

    if(NOT ${VKB_${TARGET_ID}})
        message(STATUS "Sample `${TARGET_ID}` - DISABLED")
        return()
    endif()

    message(STATUS "Sample `${SAMPLE_ID}` - BUILD")

    # create sample project (object target - reused by app target)
    project(${TARGET_ID} LANGUAGES C CXX)

    source_group("\\" FILES ${TARGET_FILES})

    add_library(${PROJECT_NAME} OBJECT ${TARGET_FILES})

    # inherit include directories from framework target
    target_include_directories(${PROJECT_NAME} PUBLIC 
        $<TARGET_PROPERTY:framework,INCLUDE_DIRECTORIES>
        ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

    # inherit compile definitions from framework target
    target_compile_definitions(${PROJECT_NAME} PUBLIC 
        $<TARGET_PROPERTY:framework,COMPILE_DEFINITIONS>)

    # add sample project to a folder
    set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Samples")

    # set sample properties
    set_target_properties(${PROJECT_NAME}
        PROPERTIES 
            SAMPLE_NAME ${TARGET_NAME}
            SAMPLE_DESCRIPTION ${TARGET_DESCRIPTION})

    if(NOT ANDROID AND ${VKB_SAMPLE_ENTRYPOINT})
        # create sample app project
        project(${TARGET_ID}_app LANGUAGES C CXX)

        # create entrypoint file for sample
        generate_sample_entrypoint(
            ID ${TARGET_ID}
            NAME ${TARGET_NAME}
            OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

        # create list of project files (objects + entrypoint file)
        set(PROJECT_FILES 
            ${CMAKE_CURRENT_BINARY_DIR}/main.cpp
            $<TARGET_OBJECTS:${TARGET_ID}>)

        source_group("\\" FILES ${PROJECT_FILES})

        add_executable(${PROJECT_NAME} WIN32 ${PROJECT_FILES})

        # inherit sample include directories from sample object target
        target_include_directories(${PROJECT_NAME} PUBLIC 
            $<TARGET_PROPERTY:${TARGET_ID},INCLUDE_DIRECTORIES>)

        target_link_libraries(${PROJECT_NAME} PUBLIC framework)

        # add sample app project to a folder
        set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Entrypoints")

        if(${VKB_ASSETS_SYMLINK})
            create_symlink(
                NAME ${PROJECT_NAME}
                DIR ${CMAKE_SOURCE_DIR}/assets 
                LINK ${CMAKE_CURRENT_BINARY_DIR}/assets)
        endif()
    endif()
endfunction()

function(order_sample_list)
    set(options)  
    set(oneValueArgs NAME)
    set(multiValueArgs INPUT ORDER OUTPUT)

    set(OUTPUT_LIST)

    cmake_parse_arguments(TARGET "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Add samples based on the given order
    foreach(SAMPLE_ID ${TARGET_ORDER})
        list(FIND TARGET_INPUT ${SAMPLE_ID} FOUND_SAMPLE)
        if(NOT ${FOUND_SAMPLE} LESS 0)
            list(APPEND OUTPUT_LIST ${SAMPLE_ID})
        endif()
    endforeach()

    # Move the remaining samples
    foreach(SAMPLE_ID ${TARGET_INPUT})
        list(FIND OUTPUT_LIST ${SAMPLE_ID} FOUND_SAMPLE)
        if(${FOUND_SAMPLE} LESS 0)
            list(APPEND OUTPUT_LIST ${SAMPLE_ID})
        endif()
    endforeach()

    set(${TARGET_OUTPUT} ${OUTPUT_LIST} PARENT_SCOPE)
endfunction()
