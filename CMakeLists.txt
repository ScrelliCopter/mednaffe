cmake_minimum_required(VERSION "3.20" FATAL_ERROR)
project(mednaffe LANGUAGES C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)

add_executable(mednaffe
	src/about.c src/about.h
	src/bios.c src/bios.h
	src/logbook.c src/logbook.h
	src/mainwindow.c src/mainwindow.h
	src/manager.c src/manager.h
	src/mednaffe.c
	src/medprocess.c src/medprocess.h
	src/panedimage.c src/panedimage.h
	src/panedlist.c src/panedlist.h
	src/pathcombobox.c src/pathcombobox.h
	src/preferences.c src/preferences.h
	src/resources.c
	src/setup.c src/setup.h
	src/signals.c
	src/widgets/dialogs.c src/widgets/dialogs.h
	src/widgets/marshallers.c src/widgets/marshallers.h
	src/widgets/bios_helper.c src/widgets/bios_helper.h
	src/widgets/medbiosentry.c src/widgets/medbiosentry.h
	src/widgets/medcheckbutton.c src/widgets/medcheckbutton.h
	src/widgets/medcolorentry.c src/widgets/medcolorentry.h
	src/widgets/medcombobox.c src/widgets/medcombobox.h
	src/widgets/meddialogentry.c src/widgets/meddialogentry.h
	src/widgets/medentry.c src/widgets/medentry.h
	src/widgets/medinput.c src/widgets/medinput.h
	src/widgets/medirange.c src/widgets/medirange.h
	src/widgets/medlistjoy.c src/widgets/medlistjoy.h
	src/widgets/medrange.c src/widgets/medrange.h
	src/widgets/medscale.c src/widgets/medscale.h
	src/widgets/medspin.c src/widgets/medspin.h
	src/widgets/medwidget.c src/widgets/medwidget.h
	src/widgets/menuinput.c src/widgets/menuinput.h
	src/widgets/joystick.h)
set_property(TARGET mednaffe PROPERTY C_STANDARD 11)
target_link_libraries(mednaffe PkgConfig::GTK3)
target_include_directories(mednaffe PRIVATE src)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	target_compile_options(mednaffe PRIVATE -Wall -Wl,-export-dynamic)
	target_sources(mednaffe PRIVATE src/widgets/joystick_linux.c)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	target_link_libraries(mednaffe share/win/mednaffe.res dxguid dinput)
	target_compile_options(mednaffe PRIVATE -mwindows)
	target_sources(mednaffe PRIVATE src/win32util.c src/win32util.h src/widgets/joystick_windows.c)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")  # Apple
	target_link_options(mednaffe PRIVATE "-Wl,-rpath,/Library/Frameworks")
	find_package(SDL2 REQUIRED)
	target_link_libraries(mednaffe SDL2::SDL2)
	target_sources(mednaffe PRIVATE src/widgets/joystick_sdl.c)
else()
	target_compile_options(mednaffe PRIVATE -Wall -Wl,-export-dynamic)
	target_sources(mednaffe PRIVATE src/widgets/joystick_dummy.c)
endif()
