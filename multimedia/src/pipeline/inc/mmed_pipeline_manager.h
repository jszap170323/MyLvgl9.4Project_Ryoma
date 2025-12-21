#ifndef _MMED_PIPELINE_MANAGER_H

#define _MMED_PIPELINE_MANAGER_H

#include <string>
#include <gst/gst.h>
#include <string>
#include <iostream>
#include <gst/app/gstappsink.h>


#include "ipc_shared_memory.h"

class MmedPipelineManager{

public:
	MmedPipelineManager();
	~MmedPipelineManager();

	bool preview();
	

private:

	IpcSharedMemory* m_shared_memory;

};







#endif /*  _MMED_PIPELINE_MANAGER_H*/
