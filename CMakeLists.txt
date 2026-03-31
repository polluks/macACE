cmake_minimum_required(VERSION 3.10)
project(macACE LANGUAGES ASM C)
set(ACE_DEBUG ON) #turn this off when making real disk!!!!
set(ACE_DEBUG_UAE ON)
#set(CMAKE_C_STANDARD 17)
# Add your project settings here
file(GLOB_RECURSE SOURCES src/*.c main.c)

if(ELF2HUNK)
	set(macACE_EXECUTABLE macACE.elf)
	set(macACE_OUTPUT_EXECUTABLE macACE.exe)
	add_executable(${macACE_EXECUTABLE} ${SOURCES})
	
	target_link_libraries(${macACE_EXECUTABLE} -Wl,-Map=macACE.map)

	add_custom_command(
		TARGET ${macACE_EXECUTABLE} POST_BUILD
		COMMAND ${ELF2HUNK} ${macACE_EXECUTABLE} ${macACE_OUTPUT_EXECUTABLE}
	)
	add_custom_command(
		TARGET ${macACE_EXECUTABLE} POST_BUILD
		COMMAND ${OBJDUMP} --disassemble -S ${macACE_EXECUTABLE} > macACE.s
	)
else()
	SET(macACE_EXECUTABLE macACE)
	SET(macACE_OUTPUT_EXECUTABLE macACE)
	
	add_executable(${macACE_EXECUTABLE} ${SOURCES})
endif()

add_subdirectory(deps/ace ace) 

# Version stuff
string(TIMESTAMP YEAR "%y")
string(TIMESTAMP DAY "%d")
string(TIMESTAMP MONTH "%m")
MATH(EXPR VER_MAJOR "0 + ${YEAR}")
MATH(EXPR VER_MINOR "0 + ${MONTH}")
MATH(EXPR VER_FIX "0 + ${DAY}")
set(VERSION "${VER_MAJOR}.${VER_MINOR}.${VER_FIX}")


target_compile_definitions(${macACE_EXECUTABLE} PRIVATE macACE_VERSION="${VERSION}")
include_directories(${SDL2_INCLUDE_DIRS} include/)
target_link_libraries(${macACE_EXECUTABLE} ace )  
target_compile_options(${macACE_EXECUTABLE} PUBLIC -Wall)


if(GAME_DEBUG)
	target_compile_definitions(${macACE_EXECUTABLE} PRIVATE GAME_DEBUG)
	target_compile_definitions(ace PUBLIC ACE_DEBUG_ALL ACE_DEBUG_UAE)
endif()

set(RES_DIR ${CMAKE_CURRENT_LIST_DIR}/res)
set(DATA_DIR ${CMAKE_CURRENT_BINARY_DIR}/data)
set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)


# Generating ZIP
set(macACE_ZIP_NAME "${CMAKE_PROJECT_NAME} ${VER_MAJOR}_${VER_MINOR}_${VER_FIX}")
add_custom_target(generateZip COMMAND
	${CMAKE_COMMAND} -E tar "cf" "${macACE_ZIP_NAME}.zip" --format=zip
	"${CMAKE_CURRENT_BINARY_DIR}/${macACE_OUTPUT_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/myacefont.fnt"
	COMMENT "Generating ${macACE_ZIP_NAME}.zip"
)

# Generating ADF
set(ADF_DIR "${CMAKE_CURRENT_BINARY_DIR}/adf")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/startup-sequence" "${macACE_OUTPUT_EXECUTABLE}\n")

add_custom_target(generateAdf
	COMMAND ${CMAKE_COMMAND} -E make_directory "${ADF_DIR}/s"
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${macACE_OUTPUT_EXECUTABLE}" "${ADF_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/startup-sequence" "${ADF_DIR}/s/startup-sequence"
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${DATA_DIR}" "${ADF_DIR}/data"
	COMMAND exe2adf -l ${CMAKE_PROJECT_NAME} -a "${CMAKE_PROJECT_NAME}.adf" -d ${ADF_DIR}
	COMMAND ${CMAKE_COMMAND} -E rm -rf "${ADF_DIR}"
	COMMENT "Generating ADF file"
)