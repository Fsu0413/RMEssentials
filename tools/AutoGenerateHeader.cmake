
# This file has different logic of original .sh or .vbs files.
# It can produce output file list as well as final output files.

# When included in project mode, it registers functions for generating output file list.
# When being run, it generates files.

set(RMESSENTIALS_HEADER_GENERATION_REGEX "(struct|class) [A-Z]*_EXPORT ([A-Za-z_][A-Za-z0-9_]*)")
set(RMESSENTIALS_AUTO_GENERATE_HEADER_CMAKE_FILE "${CMAKE_CURRENT_LIST_FILE}")

function(rmessentials_auto_generate_header_file_list original_header_file_path output_var)
    file(STRINGS "${original_header_file_path}" ORIGINAL_HEADER_STRINGS REGEX "${RMESSENTIALS_HEADER_GENERATION_REGEX}")
    set(output)
    foreach (ORIGINAL_HEADER_STRING IN LISTS ORIGINAL_HEADER_STRINGS)
        if (ORIGINAL_HEADER_STRING MATCHES "${RMESSENTIALS_HEADER_GENERATION_REGEX}")
            list(APPEND output "${CMAKE_MATCH_2}")
        else()
            message(FATAL_ERROR "Error when reading file ${original_header_file_path}")
        endif()
    endforeach()
    set("${output_var}" "${output}" PARENT_SCOPE)
endfunction()

function(write_header_file original_header_file_path target_header_file_path)
    get_filename_component(original_header_file_name "${original_header_file_path}" NAME)
    get_filename_component(target_header_file_name "${target_header_file_path}" NAME)
    get_filename_component(target_header_directory "${target_header_file_path}" DIRECTORY)
    if (NOT IS_DIRECTORY "${target_header_directory}")
        file(MAKE_DIRECTORY "${target_header_directory}")
    endif()
    if (original_header_file_name STREQUAL target_header_file_name)
        file(COPY "${original_header_file_path}" DESTINATION "${target_header_directory}")
    else()
        set(target_content "#include \"${original_header_file_name}\"")
        file(WRITE "${target_header_file_path}" "${target_content}")
    endif()
endfunction()

# PROJECT_NAME must be defined when included in project mode so let's judge this
# At least project() must be called from top directory CMakeLists.txt

if (DEFINED PROJECT_NAME)
    # project mode
    # register functions for generate header
    function (rmessentials_auto_generate_header_file_for_target target header_files)
        foreach (header_file IN LISTS header_files)
            get_filename_component(header_path_absolute "${header_file}" ABSOLUTE)
            rmessentials_auto_generate_header_file_list("${header_path_absolute}" header_generated_file_names)
            get_filename_component(header_file_name "${header_file}" NAME)
            list(PREPEND header_generated_file_names "${header_file_name}")
            set(header_generated_paths)
            foreach (header_generated_file_name IN LISTS header_generated_file_names)
                list(APPEND header_generated_paths "${CMAKE_BINARY_DIR}/dist/include/${target}/${header_generated_file_name}")
            endforeach()
            add_custom_command(OUTPUT ${header_generated_paths}
                               COMMAND "${CMAKE_COMMAND}" -P "${RMESSENTIALS_AUTO_GENERATE_HEADER_CMAKE_FILE}" "${header_path_absolute}" ${header_generated_paths}
                               MAIN_DEPENDENCY "${header_file}"
            )
            target_sources("${target}" PRIVATE ${header_generated_paths})
        endforeach()
    endfunction()
else()
    # generate mode
    set(START_ARGN)
    foreach (I RANGE ${CMAKE_ARGC})
        if (CMAKE_ARGV${I} STREQUAL "-P")
            math(EXPR START_ARGN "${I} + 2")
            break()
        endif()
    endforeach()
    math(EXPR START_ARG2 "${START_ARGN} + 1")
    math(EXPR END_ARG "${CMAKE_ARGC} - 1")
    foreach (I RANGE ${START_ARG2} ${END_ARG})
        write_header_file("${CMAKE_ARGV${START_ARGN}}" "${CMAKE_ARGV${I}}")
    endforeach()
endif()
