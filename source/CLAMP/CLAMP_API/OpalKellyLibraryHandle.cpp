#include "OpalKellyLibraryHandle.h"
#include "common.h"
#include <exception>
#include <sstream>

using std::exception;
using std::runtime_error;
using std::string;

bool logOpalKelly = true;

// -------------------------------------------------------------------------------------------------
int OpalKellyLibraryHandle::refCount = 0;

OpalKellyLibraryHandle::OpalKellyLibraryHandle()
{
}

OpalKellyLibraryHandle::~OpalKellyLibraryHandle() {
    decRef();
}

/** \brief Creates an instance of OpalKellyLibraryHandle
 *
 *  See usage in OpalKellyLibraryHandle class documentation.
 *
 * *Note: on Windows, shared libraries are DLLs.  On other OSes, they have other extensions.
 *  Although this code works on various OSes, the parameter names have dll in them.*
 *
 *  \param[in] dllPath  (Optional) path from which to load the dll/shared library.  If NULL or "", use default location.
 *  \returns An instance to the handle object.
 *  \throws std::exception if the library can't be loaded.
 */
OpalKellyLibraryHandle* OpalKellyLibraryHandle::create(okFP_dll_pchar dllPath) {
    addRef(dllPath);
    return new OpalKellyLibraryHandle();
}

// If the DLL's already been loaded, do nothing.  If not, load from default path or (if specified) dllPath
// Return true on success; false on error
void OpalKellyLibraryHandle::addRef(okFP_dll_pchar dllPath) {
    if (refCount == 0) {
        okFP_dll_pchar dllLocation = dllPath;
        // Treat "" the same as NULL, which causes Opal Kelly to load from the default location.
        if (dllLocation && dllLocation[0] == '\0') {
            dllLocation = nullptr;
        }
        if (okFrontPanelDLL_LoadLib(dllLocation) == false) {
            string path;
#ifdef _UNICODE
            path = toString(dllLocation);
#else
            path = dllLocation;
#endif
            string msg = "FrontPanel DLL could not be loaded from path '" + path + "'.";
            throw runtime_error(msg.c_str());
        }

        char dll_date[32], dll_time[32];
        okFrontPanelDLL_GetVersion(dll_date, dll_time);
        LOG(logOpalKelly) << "\n" << "FrontPanel DLL loaded.  Built: " << dll_date << "  " << dll_time << "\n";
    }
    refCount++;
}

void OpalKellyLibraryHandle::decRef() {
    refCount--;
    if (refCount == 0) {
        okFrontPanelDLL_FreeLib();
    }
}
