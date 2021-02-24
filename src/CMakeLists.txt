#
# Program: STEPToMesh
#
# Description:
#
# The program STEPToMesh converts solids contained in STEP files into triangle meshes.
#
# Copyright(C) 2020 Alexander Leutgeb
#
# This library is free software; you can redistribute it and / or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301  USA
#

cmake_minimum_required (VERSION 2.8)

macro(update_target_library LIBRARIES SEARCH REPLACE)
	foreach (LIBRARY ${LIBRARIES})
		get_target_property(TMP ${LIBRARY} INTERFACE_LINK_LIBRARIES)
		list(LENGTH TMP LEN)
		if(${LEN} GREATER 0)
			list(TRANSFORM TMP REPLACE "${SEARCH}" "${REPLACE}")
			set_target_properties(${LIBRARY} PROPERTIES INTERFACE_LINK_LIBRARIES "${TMP}")
		endif()
	endforeach()
endmacro()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project (STEPToMesh)

#
# begin of configuration section
# change values according to path structure in OpenCASCADE installation
#
set(OPENCASCADE_PREFIX "C:/OpenCASCADE-7.4.0-vc14-64" CACHE PATH "OpenCASCADE prefix")
if(WIN32)
	set(CMAKE_INSTALL_BINDIR "." CACHE PATH "User executables (bin)")
endif()
#
# end of configuration section
#

set(CMAKE_PREFIX_PATH "${OPENCASCADE_PREFIX};${CMAKE_PREFIX_PATH}")

find_package(OpenCASCADE REQUIRED)

if(OpenCASCADE_WITH_TBB)
	set(TBB_LIBRARY_PREFIX "${OPENCASCADE_PREFIX}/tbb_2017.0.100/lib/intel64/vc14" CACHE PATH "TBB library directory")
	set(TBB_BIN_PREFIX "${OPENCASCADE_PREFIX}/tbb_2017.0.100/bin/intel64/vc14" CACHE PATH "TBB binary directory")
endif()
if(OpenCASCADE_WITH_FREEIMAGE)
	set(FREEIMAGE_LIBRARY_PREFIX "${OPENCASCADE_PREFIX}/freeimage-3.17.0-vc14-64/lib" CACHE PATH "FreeImage library directory")
	set(FREEIMAGE_BIN_PREFIX "${OPENCASCADE_PREFIX}/freeimage-3.17.0-vc14-64/bin" CACHE PATH "FreeImage binary directory")
endif()
if(OpenCASCADE_WITH_FFMPEG)
	set(FFMPEG_LIBRARY_PREFIX "${OPENCASCADE_PREFIX}/ffmpeg-3.3.4-64/lib" CACHE PATH "FFmpeg library directory")
	set(FFMPEG_BIN_PREFIX "${OPENCASCADE_PREFIX}/ffmpeg-3.3.4-64/bin" CACHE PATH "FFmpeg binary directory")
endif()
if(OpenCASCADE_WITH_FREETYPE)
	set(FREETYPE_LIBRARY_PREFIX "${OPENCASCADE_PREFIX}/freetype-2.5.5-vc14-64/lib" CACHE PATH "FreeType library directory")
	set(FREETYPE_BIN_PREFIX "${OPENCASCADE_PREFIX}/freetype-2.5.5-vc14-64/bin" CACHE PATH "FreeType binary directory")
endif()

include_directories(${OpenCASCADE_INCLUDE_DIR})

set(LIBS TKernel TKMath TKGeomBase TKTopAlgo TKMesh TKSTEP TKCAF TKXCAF TKSTEPAttr TKSTEP209 TKSTEPBase TKXSBase TKLCAF TKVCAF TKCDF TKBO TKShHealing TKPrim TKGeomAlgo TKBRep TKG2d TKG3d TKService TKV3d TKXDESTEP TKSTL)

if(WIN32)
	#
	# update libraries for imported OpenCASCADE targets (modules),
	# because they have predefined paths from OpenCASCADE build
	#
	if(OpenCASCADE_WITH_TBB)
		update_target_library("${LIBS}" "^.+/tbbmalloc\\\\.lib$" "${TBB_LIBRARY_PREFIX}/tbbmalloc.lib")
		update_target_library("${LIBS}" "^.+/tbb\\\\.lib$" "${TBB_LIBRARY_PREFIX}/tbb.lib")
	endif()
	if(OpenCASCADE_WITH_FREEIMAGE)
		update_target_library("${LIBS}" "^.+/FreeImage\\\\.lib$" "${FREEIMAGE_LIBRARY_PREFIX}/FreeImage.lib")
	endif()
	if(OpenCASCADE_WITH_FFMPEG)
		update_target_library("${LIBS}" "^.+/avcodec\\\\.lib$" "${FFMPEG_LIBRARY_PREFIX}/avcodec.lib")
		update_target_library("${LIBS}" "^.+/avformat\\\\.lib$" "${FFMPEG_LIBRARY_PREFIX}/avformat.lib")
		update_target_library("${LIBS}" "^.+/swscale\\\\.lib$" "${FFMPEG_LIBRARY_PREFIX}/swscale.lib")
		update_target_library("${LIBS}" "^.+/avutil\\\\.lib$" "${FFMPEG_LIBRARY_PREFIX}/avutil.lib")
	endif()
	if(OpenCASCADE_WITH_FREETYPE)
		update_target_library("${LIBS}" "^.+/freetype\\\\.lib$" "${FREETYPE_LIBRARY_PREFIX}/freetype.lib")
	endif()
endif()

add_executable(STEPToMesh STEPToMesh.cpp)

target_link_libraries(STEPToMesh ${LIBS})

include(GNUInstallDirs)

install(TARGETS STEPToMesh)

if(WIN32)
	option(INSTALL_DEPENDENCIES "whether to install dependent libraries or not" ON)
else()
	option(INSTALL_DEPENDENCIES "whether to install dependent libraries or not" OFF)
endif()

if(INSTALL_DEPENDENCIES)
	if(WIN32)
		foreach(LIB ${LIBS} TKHLR)
			# installing imported targets as TARGETS is not possible (see https://gitlab.kitware.com/cmake/cmake/-/issues/14311)
			# so we must use FILES mode
			install(FILES $<TARGET_FILE:${LIB}> TYPE BIN)
		endforeach()
		if(OpenCASCADE_WITH_TBB)
			install(FILES 
				"${TBB_BIN_PREFIX}/tbb.dll"
				"${TBB_BIN_PREFIX}/tbbmalloc.dll"
				TYPE BIN)
		endif()
		if(OpenCASCADE_WITH_FREEIMAGE)
			install(FILES 
				"${FREEIMAGE_BIN_PREFIX}/FreeImage.dll"
				TYPE BIN)
		endif()
		if(OpenCASCADE_WITH_FREETYPE)
			install(FILES 
				"${FREETYPE_BIN_PREFIX}/freetype.dll"
				TYPE BIN)
		endif()
		if(OpenCASCADE_WITH_FFMPEG)
			install(FILES 
				"${FFMPEG_BIN_PREFIX}/avcodec-57.dll"
				"${FFMPEG_BIN_PREFIX}/avformat-57.dll"
				"${FFMPEG_BIN_PREFIX}/swscale-4.dll"
				"${FFMPEG_BIN_PREFIX}/avutil-55.dll"
				TYPE BIN)
		endif()
	endif()
endif()
