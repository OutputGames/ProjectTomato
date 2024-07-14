#include "debug.h"

#include <cassert>
#include <cstddef>
#include <iostream>

#include "renderdoc_app.h"
#include <windows.h>

RENDERDOC_API_1_1_2* rdoc_api = NULL;

void tmDebug::rdoc_init()
{
    // At init, on windows

    HMODULE mod = LoadLibraryA(RENDERDOC_HOME "/renderdoc.dll");

    if (mod)
    {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI =
            (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
        int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&rdoc_api);
        assert(ret == 1);
    } else
    {
	    std::cerr << "RenderDOC api not found." << std::endl;
    }
}

void tmDebug::rdoc_beginframe()
{
    if (rdoc_api) rdoc_api->StartFrameCapture(NULL, NULL);
}

void tmDebug::rdoc_endframe()
{
    if (rdoc_api)
    {
	    rdoc_api->EndFrameCapture(NULL, NULL);
        rdoc_api->LaunchReplayUI(1, NULL);
    }
}
