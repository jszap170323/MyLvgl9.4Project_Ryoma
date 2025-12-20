#include "mmed_pipeline_manager.h"


MmedPipelineManager::MmedPipelineManager()
{
	preview();
}

MmedPipelineManager::~MmedPipelineManager()
{
}


bool MmedPipelineManager::preview()
{


	std::string device = "/dev/video1";
	int width = 1024;   // 支持的离散分辨率
	int height = 600;
	int framerate = 30;
 
	std::string pipeline_str =
    "imxv4l2src device=" + device + " ! "
//    "video/x-raw, format=YUYV, width=" + std::to_string(width) +
  //  ", height=" + std::to_string(height) +
   // ", framerate=" + std::to_string(framerate) + "/1 ! "
    "videoconvert ! imxv4l2sink";
	

        std::cout << "Pipeline: " << pipeline_str << std::endl;
        
        // 解析管道
        GError *error = nullptr;
		GstElement *pipeline_;
        pipeline_ = gst_parse_launch(pipeline_str.c_str(), &error);
        
        if (error) {
            std::cerr << "Failed to create pipeline: " << error->message << std::endl;
            g_error_free(error);
            return false;
        }
        
        if (!pipeline_) {
            std::cerr << "Failed to create pipeline" << std::endl;
            return false;
        }
        
        // 设置管道状态为播放
        GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_FAILURE) {
            std::cerr << "Failed to start pipeline" << std::endl;
            gst_object_unref(pipeline_);
            pipeline_ = nullptr;
            return false;
        }
	return true;
}
