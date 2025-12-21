#include "mmed_pipeline_manager.h"

static GstFlowReturn on_new_sample(GstAppSink* sink, gpointer user_data)
{
//	std::cout <<"onnewsample" << std::endl;
    // user_data: 你的共享内存对象
    auto* shm = static_cast<IpcSharedMemory*>(user_data);

    // 1. 拉一帧
    GstSample* sample = gst_app_sink_pull_sample(sink);
    if (!sample)
        return GST_FLOW_ERROR;

    // 2. 取 buffer
    GstBuffer* buffer = gst_sample_get_buffer(sample);
    GstMapInfo map;

    if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        // map.data  → 像素指针
        // map.size  → 字节数

		std::cout << "map.size = " << map.size << std::endl;
        shm->write(map.data, map.size);

        gst_buffer_unmap(buffer, &map);
    }

    gst_sample_unref(sample);
    return GST_FLOW_OK;
}

MmedPipelineManager::MmedPipelineManager()
{
	m_shared_memory = new IpcSharedMemory("/VideoFrameMemory", 1024*600*2, true);
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
 
//	std::string pipeline_str =
//    "imxv4l2src device=" + device + " ! "
//    "video/x-raw, format=YUYV, width=" + std::to_string(width) +
  //  ", height=" + std::to_string(height) +
   // ", framerate=" + std::to_string(framerate) + "/1 ! "
    //"videoconvert ! imxv4l2sink";
    //"videoconvert ! appsink";
//	"videoconvert ! "
 //  	 "appsink name=appsink";

	std::string pipeline_str =
	"imxv4l2src device=/dev/video1 ! "
	"video/x-raw, format=RGB16, width=1024, height=600 ! "
	"appsink name=appsink";


        std::cout << "Pipeline: " << pipeline_str << std::endl;
        
        // 解析管道
        GError *error = nullptr;
		GstElement *pipeline_;
        pipeline_ = gst_parse_launch(pipeline_str.c_str(), &error);
	GstElement* appsink =
    	gst_bin_get_by_name(GST_BIN(pipeline_), "appsink");
	GstAppSink* app_sink = GST_APP_SINK(appsink);
	g_object_set(G_OBJECT(app_sink),
             "emit-signals", TRUE,   // 必须
             "sync", FALSE,          // 不跟时钟
             "max-buffers", 1,       // 防止堆积
             "drop", TRUE,           // 只要最新帧
             NULL);
	GstAppSinkCallbacks callbacks = {};
	callbacks.new_sample = on_new_sample;

	gst_app_sink_set_callbacks(app_sink,
                           &callbacks,
                           m_shared_memory,
                           nullptr);
        
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
		
		GMainLoop* loop = g_main_loop_new(nullptr, FALSE);
		g_main_loop_run(loop);
	return true;
}
