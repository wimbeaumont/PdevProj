cmake_minimum_required(VERSION 2.8)



project(cmaketry_project  )


###############################################################################
## file globbing ##############################################################
###############################################################################
# from https://stackoverflow.com/questions/21163188/most-simple-but-complete-cmake-HWorld
# these instructions search the directory tree when cmake is
# invoked and put all files that match the pattern in the variables 
# `sources` and `data`




# message( ${simplelibs})

# check if there is a i2c device  
# to compile for dummy cmake . -DDUMMY=true 
# but once used , make sure you remove it from cache or cmake . -DDUMMY=FALSE 
if ( DUMMY )
message ("compile for dummy") 
set( TARGET_FLAG __DUMMY__) 
else()
	if (UNIX) 
		if ( EXISTS "/dev/i2c-1")
			set(TARGET_FLAG  __LINUX__)
			message("compile for LINUX i2cdev")
		else() 
       			set(TARGET_FLAG  __DUMMY__ )
			message ("compile for dummy") 

		endif()
	else()
		message("build for MBED not supported ") 
		exit()
	endif() 
endif()

# generate the i2c dev  lib 

file(GLOB_RECURSE simplelibs  ../../PeripheralDevices/*.cpp  ../../PeripheralDevices/*.h )
# for the moment don't include  MCP4728setaddr.cpp 
list(REMOVE_ITEM simplelibs "${CMAKE_CURRENT_SOURCE_DIR}/../../PeripheralDevices/MCP4728/MCP4728setaddr.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/../../PeripheralDevices/MCP4728/MCP4728setaddr.h")

add_library(tstlib STATIC  ${simplelibs})
target_include_directories(tstlib PUBLIC  ../../PeripheralDevices/DevInterfaces/)
#message ( "${CMAKE_CURRENT_SOURCE_DIR}/../../PeripheralDevices/MCP4728//MCP4728setaddr.cpp" )


#generate the scpi lib 
file(GLOB_RECURSE scpiliblist  ../../PdevUtils/scpi-parser/*.c  ../../PdevUtils/scpi-parser/*.h )
add_library(scpiparserlib STATIC  ${scpiliblist})
target_include_directories(scpiparserlib  PUBLIC  ../../PdevUtils/scpi-parser/inc )

#set_target_properties(tstlib PROPERTIES LINKER_LANGUAGE CXX)
#set_target_properties(tstlib PROPERTIES COMPILER_LANGUAGE CXX)


# collect all main programs 
file(GLOB_RECURSE ALLPROGS  ../*/src/*.cpp)


## build the include directories , 

MACRO(HEADER_DIRECTORIES return_list)
    FILE(GLOB_RECURSE new_list ../../PeripheralDevices/*/*h)
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

# macro source https://stackoverflow.com/questions/53013529/how-to-tell-cmake-to-recursively-search-include-directory-when-specificying-incl

#get the list 

HEADER_DIRECTORIES ( dev_incl_dirs)



#message( ${dev_incl_dirs})

##############################################################################
## target definitions #########################################################
###############################################################################


foreach ( testprog ${ALLPROGS}) 
	#string( REPLACE ".cpp" "" testname ${testprog} )
	get_filename_component(testname  ${testprog} NAME_WE )
	add_executable( ${testname}  ${testprog}  )
	# just add some compiler flags
	#target_compile_options(${testname}  PUBLIC -std=c++1y -Wall -D${TARGET_FLAG})
	target_compile_options(${testname}  PUBLIC -g -Wall -D${TARGET_FLAG})
	# this lets me include files relative to the root src dir with a <> pair
	#target_include_directories(${testname} PUBLIC  ../../PeripheralDevices/*/)
	target_include_directories(${testname} PUBLIC  ${dev_incl_dirs}  )
	target_link_libraries( ${testname} tstlib )
 	#install(TARGETS  ${testname}  DESTINATION ../BUILD/${testname} )
endforeach ( testprog ${allprogs}) 
###############################################################################
## testing ####################################################################
###############################################################################



