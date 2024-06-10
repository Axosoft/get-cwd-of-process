#include <string>

#include <napi.h>

#include <windows.h>
#include <winternl.h>

#define PTR_ADD_OFFSET(Pointer, Offset) ((PVOID)((ULONG_PTR)(Pointer) + (ULONG_PTR)(Offset)))

struct CURDIR {
  UNICODE_STRING DosPath;
  HANDLE Handle;
};

struct UNICODE_STRING32 {
  USHORT Length;
  USHORT MaximumLength;
  ULONG Buffer;
};

struct CURDIR32 {
  UNICODE_STRING32 DosPath;
  ULONG Handle;
};

// https://www.geoffchappell.com/studies/windows/km/ntoskrnl/inc/api/pebteb/rtl_user_process_parameters.htm
struct LOCAL_RTL_USER_PROCESS_PARAMETERS {
  ULONG _1[4];
  HANDLE _2;
  ULONG _3;
  HANDLE _4[3];
  CURDIR CurrentDirectory;
};

struct LOCAL_RTL_USER_PROCESS_PARAMETERS32 {
  ULONG _1[4];
  ULONG /* HANDLE32 */ _2;
  ULONG _3;
  ULONG /* HANDLE32 */ _4[3];
  CURDIR32 CurrentDirectory;
};

struct PEB32 {
  BOOLEAN _1[4];
  ULONG _2[3];
  ULONG ProcessParameters;
};

typedef NTSTATUS(__stdcall *nt_close)(HANDLE);
typedef NTSTATUS(__stdcall *nt_openProcess)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, CLIENT_ID *);
typedef NTSTATUS(__stdcall *nt_queryInformationProcess)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, ULONG_PTR);
typedef NTSTATUS(__stdcall *nt_readVirtualMemory)(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);

struct NtCall {
    HMODULE p_ntdll;
    nt_close close;
    nt_openProcess openProcess;
    nt_queryInformationProcess queryInformationProcess;
    nt_readVirtualMemory readVirtualMemory;
};

class GetCwdOfProcess : public Napi::Addon<GetCwdOfProcess> {
  NtCall nt;

  void initializeNtCall() {
    this->nt.p_ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!this->nt.p_ntdll) {
      throw std::runtime_error{"unable to get handle to ntdll.dll"};
    }

    this->nt.close = (nt_close)GetProcAddress(this->nt.p_ntdll, "NtClose");
    if (!this->nt.close) {
      throw std::runtime_error{"unable to find NtClose"};
    }

    this->nt.openProcess = (nt_openProcess)GetProcAddress(this->nt.p_ntdll, "NtOpenProcess");
    if (!this->nt.openProcess) {
      throw std::runtime_error{"unable to find NtOpenProcess"};
    }

    this->nt.queryInformationProcess = (nt_queryInformationProcess)GetProcAddress(this->nt.p_ntdll, "NtQueryInformationProcess");
    if (!this->nt.queryInformationProcess) {
      throw std::runtime_error{"unable to find NtQueryInformationProcess"};
    }

    this->nt.readVirtualMemory = (nt_readVirtualMemory)GetProcAddress(this->nt.p_ntdll, "NtReadVirtualMemory");
    if (!this->nt.readVirtualMemory) {
      throw std::runtime_error{"unable to find NtReadVirtualMemory"};
    }
  }

  NTSTATUS readCwd(wchar_t **outputString, unsigned long pid) {
    HANDLE processHandle = NULL;
    PWCHAR cwdString;
    CLIENT_ID clientId{
      UlongToHandle(pid),
      NULL
    };
    OBJECT_ATTRIBUTES objectAttributes{
      sizeof(OBJECT_ATTRIBUTES),
      NULL,
      0,
      NULL,
      NULL,
      NULL
    };
    NTSTATUS status;

    if (!NT_SUCCESS(status = this->nt.openProcess(
      &processHandle,
      PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
      &objectAttributes,
      &clientId
    ))) {
      return status;
    }

    ULONG_PTR wow64;
    if (!NT_SUCCESS(status = this->nt.queryInformationProcess(
      processHandle,
      ProcessWow64Information,
      &wow64,
      sizeof(ULONG_PTR),
      NULL
    ))) {
      return status;
    }

    if (!wow64) {
      ULONG offset = FIELD_OFFSET(LOCAL_RTL_USER_PROCESS_PARAMETERS, CurrentDirectory);
      PROCESS_BASIC_INFORMATION basicInfo;
      PVOID processParameters;
      if (!NT_SUCCESS(status = this->nt.queryInformationProcess(
        processHandle,
        ProcessBasicInformation,
        &basicInfo,
        sizeof(PROCESS_BASIC_INFORMATION),
        NULL
      ))) {
        return status;
      }

      if (!NT_SUCCESS(status = this->nt.readVirtualMemory(
        processHandle,
        PTR_ADD_OFFSET(basicInfo.PebBaseAddress, FIELD_OFFSET(PEB, ProcessParameters)),
        &processParameters,
        sizeof(PVOID),
        NULL
      ))) {
        return status;
      }

      UNICODE_STRING initReadString;
      if (!NT_SUCCESS(status = this->nt.readVirtualMemory(
        processHandle,
        PTR_ADD_OFFSET(processParameters, offset),
        &initReadString,
        sizeof(UNICODE_STRING),
        NULL
      ))) {
        return status;
      }

      if (initReadString.Length == 0) {
        this->nt.close(processHandle);
        *outputString = new WCHAR[1];
        *outputString[0] = '\0';
        return status;
      }

      size_t strLength = initReadString.Length + 1;
      cwdString = new WCHAR[strLength];
      cwdString[strLength / 2] = 0;
      if (!NT_SUCCESS(status = this->nt.readVirtualMemory(
        processHandle,
        initReadString.Buffer,
        cwdString,
        initReadString.Length,
        NULL
      ))) {
        return status;
      }
    } else {
      ULONG offset = FIELD_OFFSET(LOCAL_RTL_USER_PROCESS_PARAMETERS32, CurrentDirectory);
      PVOID peb32;
      ULONG processParameters32;


      if (!NT_SUCCESS(status = this->nt.queryInformationProcess(
        processHandle,
        ProcessWow64Information,
        &peb32,
        sizeof(ULONG_PTR),
        NULL
      ))) {
        return status;
      }


      if (!NT_SUCCESS(status = this->nt.readVirtualMemory(
        processHandle,
        PTR_ADD_OFFSET(peb32, FIELD_OFFSET(PEB32, ProcessParameters)),
        &processParameters32,
        sizeof(unsigned long),
        NULL
      ))) {
        return status;
      }

      UNICODE_STRING32 initReadString;
      if (!NT_SUCCESS(status = this->nt.readVirtualMemory(
        processHandle,
        PTR_ADD_OFFSET(processParameters32, offset),
        &initReadString,
        sizeof(UNICODE_STRING32),
        NULL
      ))) {
        return status;
      }

      if (initReadString.Length == 0) {
        this->nt.close(processHandle);
        *outputString = new wchar_t[1];
        *outputString[0] = '\0';
        return status;
      }

      size_t strLength = initReadString.Length + 1;
      cwdString = new WCHAR[strLength];
      cwdString[strLength / 2] = 0;
      if (!NT_SUCCESS(status = this->nt.readVirtualMemory(
        processHandle,
        UlongToPtr(initReadString.Buffer),
        cwdString,
        initReadString.Length,
        NULL
      ))) {
        return status;
      }
    }

    *outputString = cwdString;
    this->nt.close(processHandle);
    return status;
  }

public:
  GetCwdOfProcess(Napi::Env env, Napi::Object exports) {
    try {
      initializeNtCall();
    } catch (const std::runtime_error &e) {
      throw Napi::Error::New(env, std::string("Failed to initialize: ") + e.what());
    }
    
    DefineAddon(exports, {
      InstanceMethod("readCwd", &GetCwdOfProcess::ReadCwd),
    });
  }

  Napi::Value ReadCwd(const Napi::CallbackInfo& info) {
    // unfortunately 0 is the default value when trying to coerce a value into a number
    // but pid 0 is the System Idle Process which we aren't allowed to query anyways
    uint32_t pid = 0;
    if (info.Length() < 1 || !(pid = (uint32_t)info[0].ToNumber())) {
      throw Napi::Error::New(info.Env(), "Expected a pid");
    }

    NTSTATUS status;
    PWCHAR str;
    if (!NT_SUCCESS(status = readCwd(&str, pid))) {
      throw Napi::Error::New(info.Env(), std::string("failed with status code: ") + std::to_string(status));
    }

    std::wstring wide_string{ str };
    // this conversion only works on windows luckily
    std::u16string u16le_string{ wide_string.begin(), wide_string.end() };
    return Napi::String::New(info.Env(), u16le_string);
  }
};

NODE_API_ADDON(GetCwdOfProcess)