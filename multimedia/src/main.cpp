#include<iostream>
#include <unistd.h>
#include "mmed_pipeline_manager.h"
#include <stdio.h>
#include <gst/gst.h> 
 
int main(){

const gchar *nano_str; 
  guint major, minor, micro, nano; 
 
	gst_init(NULL,NULL);
 
  gst_version (&major, &minor, &micro, &nano); 
 
  if (nano == 1) 
    nano_str = "(CVS)"; 
  else if (nano == 2) 
    nano_str = "(Prerelease)"; 
  else 
    nano_str = ""; 
 
  printf ("This program is linked against GStreamer %d.%d.%d %s\n", 
          major, minor, micro, nano_str);

	MmedPipelineManager pipeline;

	while(1){
		usleep(1000);
	}
    return 0;
}
