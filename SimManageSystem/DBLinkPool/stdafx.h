// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: 在此处引用程序需要的其他头文件
#include <WinSock2.h>
#include <process.h>

#include <string>
#include <iostream>
#include <queue>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

#include <icrsint.h>

#import "C:\Program Files (x86)\Common Files\system\ado\msado15.dll" no_namespace rename("EOF","adoEOF") rename("BOF", "adoBOF")
