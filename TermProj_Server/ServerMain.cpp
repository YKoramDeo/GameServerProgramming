#include "stdafx.h"
#include "protocol.h"

HANDLE ghIOCP;

void InitializeServerData(void);
void DisplayDebugText(std::string);

int main()
{
	// 01. Main문은 다음과 같이 동작하도록 한다.
	InitializeServerData();
	CreateAcceptThread();
	CreateWorkerThreads();
	StopServer();
	return 0;
}

void DisplayDebugText(std::string str)
{
	std::cout << str << std::endl;
	return;
}

void InitializeServerData(void)
{
	// 02. 기본적인 server를 동작하기 위한 초기화 작업을 진행한다.
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		DisplayDebugText("ERROR :: InitializeServerData :: WSAStartup Fail !!");
		exit(EXIT_FAILURE);
		// exit()	: 프로그램에서 호스트 환경에 제어를 리턴함. atexit() 함수에 등록된 모든 함수를 역순으로 호출.
		//			: 프로그램을 종료하기 전에 버퍼를 모두 삭제하고 열린 파일을 모두 닫음. 
		//			: EXIT_SUCCESS 또는 0은 정상종료, 그렇지 않으면 다른 상태 값이 리턴.
	}
	
	// WSAStartup()	: 윈속 초기화 함수로, 프로그램에서 사용할 윈속 버전을 요청함으로써 윈속 라이브러리(WS2_32.DLL)를 초기화 하는 역할.
	//				: return - 성공하면 0, 실패하면 오류코드

	// MAKEWORD(2,2)	: 프로그램이 요구하는 최상위 윈속 버전. 하위 8비트에 주 버전, 상위 8비트에 부 버전을 넣어서 전달
	//					: 만일 윈속 3.2 버전을 사용을 요청한다면 0x0203 또는 MAKEWORD(3,2)를 사용
	// wsadata			: WSADATA 구조체를 전달하면 이를 통해 윈도우 운영체제가 제공하는 윈속 구현에 관한 정보를 얻을 수 있음.
	//					: (응용 프로그램이 실제로 사용하게 될 윈속 버전, 시스템이 지원하는 윈속 최상위 버전 등), 하지만 실제로 이런 정보는 사용하지 않음.

	ghIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (NULL == ghIOCP)
	{
		DisplayDebugText("ERROR :: InitializeServerData :: CreateIoCompletionPort Fail !!");
		exit(EXIT_FAILURE);
	}
	
	// CreateIoCompletionPort()	: 1. 입출력 완료 포트를 생성.
	//							: 2. 소켓과 입출력 완료포트를 연결.
	//								소켓과 입출력 완료포트를 연결해두면 이 소켓에 대한 비동기 입출력 결과가 입출력 완료포트에 저장.
	return;
}