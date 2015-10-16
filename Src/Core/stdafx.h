// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件:
#include <windows.h>


#ifdef _WIN32
#ifdef EXP_GVI_TASK_CORE
#    define GVI_API_TASK_CORE __declspec(dllexport)
#else
#    define GVI_API_TASK_CORE __declspec(dllimport)
#endif
#else
#define GVI_API_TASK
#endif

#ifndef NS_TASK
#define NS_TASK namespace DT {
#endif

#ifndef NS_END
#define NS_END  }
#endif
// TODO: 在此处引用程序需要的其他头文件
