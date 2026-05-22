#include <stdexcept>
#include <string>
#include <napi.h>

#include <libproc.h>
#include <sys/errno.h>
#include <sys/proc_info.h>

class GetCwdOfProcess : public Napi::Addon<GetCwdOfProcess> {
  void readCwd(std::string &outputString, unsigned long pid) {
    constexpr size_t max_len = 1024;

    struct proc_vnodepathinfo node_path_info = {};

    int status = proc_pidinfo((int)pid, PROC_PIDVNODEPATHINFO, 0, &node_path_info, PROC_PIDVNODEPATHINFO_SIZE);

    if (status != sizeof(node_path_info)) {
      if (status == ESRCH) {
        throw std::runtime_error {"process does not exist"};
      }

      throw std::runtime_error {std::string("proc_pidinfo returned error code ") + std::to_string(status)};
    }

    // the cdir is the working directory, while the rdir is the 
    size_t len = strnlen(node_path_info.pvi_cdir.vip_path, max_len);

    if (len == max_len) {
      throw std::runtime_error {"length of returned path exceeds 1024"};
    }

    outputString = std::string {node_path_info.pvi_cdir.vip_path, len};
  }

public:
  GetCwdOfProcess(Napi::Env env, Napi::Object exports) {
    DefineAddon(exports, {
      InstanceMethod("readCwd", &GetCwdOfProcess::ReadCwd),
    });
  }

  Napi::Value ReadCwd(const Napi::CallbackInfo& info) {
    // unfortunately 0 is the default value when trying to coerce a value into a number
    // but pid 0 is the init process which we aren't allowed to query anyways
    uint32_t pid = 0;
    if (info.Length() < 1 || !(pid = (uint32_t)info[0].ToNumber())) {
      throw Napi::Error::New(info.Env(), "Expected a pid");
    }

    try {
      std::string path;
      readCwd(path, pid);
      return Napi::String::New(info.Env(), path);
    } catch (std::runtime_error &e) {
      throw Napi::Error::New(info.Env(), e.what());
    } catch (...) {
      throw Napi::Error::New(info.Env(), "an unknown error occurred");
    }
  }
};

NODE_API_ADDON(GetCwdOfProcess)