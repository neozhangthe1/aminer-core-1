#pragma once
#include <glog/logging.h>

#define CHECK_PTR_LOG_FATAL(ptr)																			\
	if(ptr == NULL){LOG(FATAL) << "fatal error occured" ;}	\
	if(ptr == NULL)

#define CHECK_PTR(ptr)															\
	if(ptr == NULL){LOG(FATAL) << "*** null pointer ***";}	\
	if(ptr == NULL)
