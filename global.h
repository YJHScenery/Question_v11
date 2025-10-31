// Copyright (C) 2025 玛卡巴卡YJH
// SPDX-License-Identifier: GPL-3.0-only OR LGPL-3.0-only
#pragma once

#include <QtGlobal>

#define QUESTION_VERSION_MAJOR 11

#define QUESTION_VERSION_MINOR 1

#define QUESTION_VERSION_PATCH 4

#define KERNEL_VERSION_MAJOR 7

#define KERNEL_VERSION_MINOR 3

#define KERNEL_VERSION_PATCH 2


// 宏 QUESTION_11_XLSX_ENABLED 标识了是否启用 Xlsx 表格文件修改功能。
// 如果启用，请确保正确地导入了 QXlsx 库。
// 如果删除此宏的声明，相关的函数声明和定义将被忽略。（详情请见 "ClassGroup.h", "ClassGroup.cpp"）
// 此宏非常重要！！！
#define QUESTION_11_XLSX_ENABLED

#include <QtTypes>
// 此处定义了一些需要在全局范围内使用的常量。

constexpr char QUESTION_VERSION_STR[]{ "11.1.4" };
constexpr char KERNEL_VERSION_STR[]{ "7.3.2" };

constexpr char DEFAULT_DATA_DIRECTORY[]{ "./data/" };
constexpr char TRAVERSAL_DIRECTORY[]{ "./traversal/" };
constexpr char CONFIG_DIRECTORY[]{ "./config/" };
constexpr char DEFAULT_HISTORY_DIRECTORY[]{ "./history/" };
constexpr char DATA_SUFFIX[]{ ".qcg" };
constexpr char TRAVERSAL_SUFFIX[]{ ".qct" };
constexpr char WINDOW_CONFIG[]{ "windowconfig.xml" };
constexpr char WHOLE_WINDOW_CONFIG_PATH[]{ "./config/windowconfig.xml" };
constexpr char HISTORY_FILE_PREFIX[]{ "HISTORY_" };
constexpr char HISTORY_FILE_SUFFIX[]{ ".history" };

constexpr quint32 MAX_CLASS_GROUP_SIZE{ 9999 };
constexpr quint32 MAX_QUESTION_HISTORY_SIZE{ 1000 };

constexpr quint32 CLASS_GROUP_DATA_MAGIC_NUMBER{ 0x43473230 };
constexpr quint8 CLASS_GROUP_DATA_VERSION{ 0x01 };

constexpr quint32 QUESTION_TRAVERSE_MAGIC_NUMBER{ 0x74726176 };
constexpr quint8 QUESTION_TRAVERSE_DATA_VERSION{ 0x01 };

constexpr quint32 MY_ARDUINO_REMOTE_VID_01 {0x1A86};
constexpr quint32 MY_ARDUINO_REMOTE_PID_01 {0x7523};

#define PLACEHOLDER_CLASS_DEFILE QK::ClassGroup("PLACEHOLDER CLASS", "", "")

// 警报声的宏定义
#ifdef _WIN32
#include <Windows.h>
#define MAINWINDOW_CALL_WARNING_SOUND MessageBeep(MB_ICONERROR);
#elif defined(__APPLE__)
#include <Cocoa/Cocoa.h>
#define MAINWINDOW_CALL_WARNING_SOUND NSBeep();
#elif defined(__linux__)
#include <canberra.h>
#define MAINWINDOW_CALL_WARNING_SOUND \
ca_context *ctx;\
if (ca_context_create(&ctx) == CA_SUCCESS) {\
ca_context_play(ctx, 0, CA_PROP_EVENT_ID, "dialog-error", NULL);\
ca_context_destroy(ctx);\
}
#endif

