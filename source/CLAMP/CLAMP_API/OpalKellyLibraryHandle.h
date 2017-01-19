#pragma once

#include "okFrontPanelDLL.h"

extern bool logOpalKelly;

/** \brief Helper class to load/unload the Opal Kelly library only as needed
 *
 *  To use, write code something like this:
 *  \code{.cpp}
    OpalKellyLibraryHandle* myHandle = OpalKellyLibraryHandle::create("okFrontPanel.dll");
 *  \endcode
 *  This code will load the library, create a handle object, and keep an instance to it.
 
 *  When you're done with the library, simply use:
 *  \code{.cpp}
    delete myHandle;
 *  \endcode
 *  This code deletes the object and unloads the library.
 *
 *  *Of course, it would be better to use a smart pointer like std::unique_ptr.*
 *
 *  Internally, the class is smart enough that it keeps a reference count, so if you're doing
 *  something with more than one board (say), you can safely create multiple OpalKellyLibraryHandle
 *  instances, and the library will only be loaded once (when the first one is created) and unloaded
 *  once (when the last one is deleted).
 */
class OpalKellyLibraryHandle {
public:
    ~OpalKellyLibraryHandle();

    static OpalKellyLibraryHandle* create(okFP_dll_pchar dllPath);

private:
    OpalKellyLibraryHandle();

    static void addRef(okFP_dll_pchar dllPath);
    static void decRef();
    static int refCount;
};

