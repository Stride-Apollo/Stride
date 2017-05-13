/*
 *  This is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Interface for install directory queries.
 */

#include "InstallDirs.h"

#include "util/StringUtils.h"

#if defined(WIN32)
#  include <stdlib.h>
#  include <windows.h>
#elif defined(__linux__)

#elif defined(__APPLE__)
#  include <mach-o/dyld.h>
#  include <limits.h>
#endif

namespace stride {
namespace util {

using namespace std;
using namespace boost::filesystem;

path     InstallDirs::g_bin_dir;
path     InstallDirs::g_current_dir;
path     InstallDirs::g_data_dir;
path     InstallDirs::g_exec_path;
path     InstallDirs::g_root_dir;
path	 InstallDirs::g_output_dir;


inline void InstallDirs::check() {
	static bool initialized = false;
	if (!initialized) {
		initialize();
		initialized = true;
	}
}

void InstallDirs::initialize() {
	//------- Retrieving path of executable
	{
		// Returns the full path to the currently running executable, or an empty string in case of failure.
		// http://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe/33249023#33249023

		#if defined(WIN32)
		char exePath[MAX_PATH];
		HMODULE hModule = GetModuleHandle(NULL);
		if (GetModuleFileName(NULL, exePath, sizeof(exePath)) !=0); {
				g_exec_path = canonical(system_complete(exePath));
		}
		#elif defined(__linux__)
		char exePath[PATH_MAX+1];
		size_t size = ::readlink("/proc/self/exe", exePath, sizeof(exePath));
		exePath[size] = '\0';
		if (size > 0 && size != sizeof(exePath)) {
			exePath[size] = '\0';
			g_exec_path = canonical(system_complete(exePath));
		}
		#elif defined(__APPLE__)
		char exePath[PATH_MAX];
		uint32_t size = sizeof(exePath);
			if (_NSGetExecutablePath(exePath, &size) == 0) {
							g_exec_path = canonical(system_complete(exePath));
			}
		#endif
	}

	//------- Retrieving root and bin directory (the subdirectory of the install root)
	{
		path exec_dir = g_exec_path.parent_path();
		if (!g_exec_path.empty()) {
			#if (__APPLE__)
			if (exec_dir.filename().string() == "MacOS") {
					// app
					//      -Contents               <-Root Path
					//              -MacOS
					//                   -executables
					g_bin_dir = exec_dir;
					g_root_dir = exec_dir.parent_path();
			} else
			#endif
			if (StringUtils::toLower(exec_dir.filename().string()) == "debug"
				|| StringUtils::toLower(exec_dir.filename().string()) == "release") {
				//x/exec                <-Root Path
				//      -bin
				//              -release/debug
				//                      -executables
				g_bin_dir = exec_dir.parent_path();
				g_root_dir = exec_dir.parent_path().parent_path();
			} else
				#if (WIN32)
				if (exec_dir.filename().string() != "bin") {
						// Executables in root folder
						g_bin_dir  = exec_dir;
						g_root_dir = exec_dir;
				} else
				#endif
			{
				//x/exec                <-Root Path
				//      -bin
				//              -executables
				g_bin_dir = exec_dir;
				g_root_dir = exec_dir.parent_path();
			}
		}
	}

	//------- Data Dir
	{
		g_data_dir = g_root_dir / "data";
		g_data_dir = is_directory(g_data_dir) ? g_data_dir : path();
	}
	//------- Current Dir
	{
		g_current_dir = system_complete(current_path());
	}
	//------- Output Dir
	{
		g_output_dir = g_root_dir / "output";
		g_output_dir = is_directory(g_output_dir) ? g_output_dir : path();
	}
}

path InstallDirs::getBinDir() {
	check();
	return g_bin_dir;
}

path InstallDirs::getCurrentDir() {
	check();
	return g_current_dir;
}

path InstallDirs::getDataDir() {
	check();
	return g_data_dir;
}

path InstallDirs::getExecPath() {
	check();
	return g_exec_path;
}

path InstallDirs::getRootDir() {
	check();
	return g_root_dir;
}

path InstallDirs::getOutputDir() {
	check();
	return g_output_dir;
}

}
}
