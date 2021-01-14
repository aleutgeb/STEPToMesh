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
set(OPENCASCADE_PREFIX "C:/OpenCASCADE-7.4.0-vc14-64")
set(OPENCASCADE_BIN_PREFIX "${OPENCASCADE_PREFIX}/opencascade-7.4.0/win64/vc14/bin")
set(TBB_LIBRARY_PREFIX "${OPENCASCADE_PREFIX}/tbb_2017.0.100/lib/intel64/vc14")
set(TBB_BIN_PREFIX "${OPENCASCADE_PREFIX}/tbb_2017.0.100/bin/intel64/vc14")
set(FREEIMAGE_LIBRARY_PREFIX "${OPENCASCADE_PREFIX}/freeimage-3.17.0-vc14-64/lib")
set(FREEIMAGE_BIN_PREFIX "${OPENCASCADE_PREFIX}/freeimage-3.17.0-vc14-64/bin")
set(FFMPEG_LIBRARY_PREFIX "${OPENCASCADE_PREFIX}/ffmpeg-3.3.4-64/lib")
set(FFMPEG_BIN_PREFIX "${OPENCASCADE_PREFIX}/ffmpeg-3.3.4-64/bin")
set(FREETYPE_LIBRARY_PREFIX "${OPENCASCADE_PREFIX}/freetype-2.5.5-vc14-64/lib")
set(FREETYPE_BIN_PREFIX "${OPENCASCADE_PREFIX}/freetype-2.5.5-vc14-64/bin")
#
# end of configuration section
#

set(CMAKE_PREFIX_PATH "${OPENCASCADE_PREFIX};${CMAKE_PREFIX_PATH}")

find_package(OpenCASCADE)

include_directories(${OpenCASCADE_INCLUDE_DIR})

set(LIBS TKernel TKMath TKGeomBase TKTopAlgo TKMesh TKSTEP TKCAF TKXCAF TKSTEPAttr TKSTEP209 TKSTEPBase TKXSBase TKLCAF TKVCAF TKCDF TKBO TKShHealing TKPrim TKGeomAlgo TKBRep TKG2d TKG3d TKService TKV3d TKXDESTEP TKSTL)

#
# update libraries for imported OpenCASCADE targets (modules),
# because they have predefined paths from OpenCASCADE build
#
update_target_library("${LIBS}" "^.+/tbbmalloc\\\\.lib$" "${TBB_LIBRARY_PREFIX}/tbbmalloc.lib")
update_target_library("${LIBS}" "^.+/tbb\\\\.lib$" "${TBB_LIBRARY_PREFIX}/tbb.lib")
update_target_library("${LIBS}" "^.+/FreeImage\\\\.lib$" "${FREEIMAGE_LIBRARY_PREFIX}/FreeImage.lib")
update_target_library("${LIBS}" "^.+/avcodec\\\\.lib$" "${FFMPEG_LIBRARY_PREFIX}/avcodec.lib")
update_target_library("${LIBS}" "^.+/avformat\\\\.lib$" "${FFMPEG_LIBRARY_PREFIX}/avformat.lib")
update_target_library("${LIBS}" "^.+/swscale\\\\.lib$" "${FFMPEG_LIBRARY_PREFIX}/swscale.lib")
update_target_library("${LIBS}" "^.+/avutil\\\\.lib$" "${FFMPEG_LIBRARY_PREFIX}/avutil.lib")
update_target_library("${LIBS}" "^.+/freetype\\\\.lib$" "${FREETYPE_LIBRARY_PREFIX}/freetype.lib")

add_executable(STEPToMesh STEPToMesh.cpp)

target_link_libraries(STEPToMesh ${LIBS})

add_custom_command(TARGET STEPToMesh POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${OPENCASCADE_BIN_PREFIX}/TKBRep.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKernel.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKG2d.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKG3d.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKGeomAlgo.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKGeomBase.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKMath.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKMesh.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKShHealing.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKSTEP.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKSTEP209.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKSTEPAttr.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKSTEPBase.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKTopAlgo.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKXSBase.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKXDESTEP.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKXCAF.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKLCAF.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKVCAF.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKV3d.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKCAF.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKCDF.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKBO.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKService.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKHLR.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKPrim.dll"
	"${OPENCASCADE_BIN_PREFIX}/TKSTL.dll"
	"${TBB_BIN_PREFIX}/tbb.dll"
	"${TBB_BIN_PREFIX}/tbbmalloc.dll"
	"${FREEIMAGE_BIN_PREFIX}/FreeImage.dll"
	"${FREETYPE_BIN_PREFIX}/freetype.dll"
	"${FFMPEG_BIN_PREFIX}/avcodec-57.dll"
	"${FFMPEG_BIN_PREFIX}/avformat-57.dll"
	"${FFMPEG_BIN_PREFIX}/swscale-4.dll"
	"${FFMPEG_BIN_PREFIX}/avutil-55.dll"
	$<TARGET_FILE_DIR:STEPToMesh>)