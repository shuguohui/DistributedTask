// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�:
#include <windows.h>


#ifdef _WIN32
#ifdef EXP_GVI_TASK
#    define GVI_API_TASK __declspec(dllexport)
#else
#    define GVI_API_TASK __declspec(dllimport)
#endif
#else
#define GVI_API_TASK
#endif


// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
