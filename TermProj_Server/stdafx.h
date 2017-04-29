#pragma once
#pragma comment(lib,"ws2_32")
#define WIN32_LEAN_AND_MEAN
// 윈도우즈 프로그램을 만드는 방법에는 두 가지가 있는데 하나는 MFC(Microsoft Foundation Class)를 사용하는 방법이며
// 다른 하나는 SDK(Software Development Kit)를 사용하는 방법이 있다.
// 전반적으로 C++과 Class 기반에 두고 있는 MFC가 훨씬 복잡하면 게임 프로그래머가 필요로 하는 것보다 수십배는 더 강력하고 정교하다.
// 반면에 SDK는 일반 C언어를 사용하며 MFC에 비해서는 아주 간단하다.
// 그러므로 게임 프로그램을 개발할 때 MFC를 사용하지 않고 SDK를 사용한다면 
// MFC를 위해 만들어둔 수많은 자료구조와 새로운 데이터 정의, 데이터형은 필요로 하지 않는다.
// 이것들은 MFC 프로그래밍에서는 필요로 하지만 SDK 프로그래밍에 있어서는 심한 오버헤더를 유발한다.
// #define WIN32_LEAN_AND_MEAN은 컴파일러게 외부 MFC 오버헤더를 포함하지 말도록 지시한다.
// 이와 비슷한 문장으로는 #pragma once인데, 이 문장은 MFC에서 헤더를 한 번만 include 하도록 해준다. 그래서 오버헤더를 줄이도록 한다.
// 위의 문장을 다른 헤더파일을 include하기 이전에 선언한다면, 수많은 헤더를 include하면서 중복되는 헤더를 방지하고
// 데이터의 중복정의를 제거해가면서 SDK 프로그래밍에 필요없는 정의들을 건너뛴다.

#include <Windows.h>
#include <WinSock2.h>
#include <iostream>
#include <locale>

#include <vector>
#include <algorithm>

#include <thread>