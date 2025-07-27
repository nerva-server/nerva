#include <napi.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <cstring>
#include <cerrno>

class Http : public Napi::ObjectWrap<Http> {
  std::vector<iovec> iovecs_;
  std::vector<std::string> stringStorage_;
  int fd_ = -1;

public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  Http(const Napi::CallbackInfo& info);

  Napi::Value AddString(const Napi::CallbackInfo& info);
  Napi::Value AddBuffer(const Napi::CallbackInfo& info);
  Napi::Value WriteToFd(const Napi::CallbackInfo& info);

  Napi::Value GetBodyString(const Napi::CallbackInfo& info);
  Napi::Value GetHeaderString(const Napi::CallbackInfo& info);

  Napi::Value FindHeaderEnd(const Napi::CallbackInfo& info);

  Napi::Value Consume(const Napi::CallbackInfo& info);

  Napi::Value GetSlice(const Napi::CallbackInfo& info);

  Napi::Value ParseHeaders(const Napi::CallbackInfo& info);

private:
  std::string concatAllData();
};

Napi::Object Http::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "Http", {
    InstanceMethod("addString", &Http::AddString),
    InstanceMethod("addBuffer", &Http::AddBuffer),
    InstanceMethod("writeToFd", &Http::WriteToFd),
    InstanceMethod("findHeaderEnd", &Http::FindHeaderEnd),
    InstanceMethod("consume", &Http::Consume),
    InstanceMethod("getSlice", &Http::GetSlice),
    InstanceMethod("getBodyString", &Http::GetBodyString),
    InstanceMethod("getHeaderString", &Http::GetHeaderString),
    InstanceMethod("parseHeaders", &Http::ParseHeaders),
  });

  exports.Set("Http", func);
  return exports;
}

Http::Http(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Http>(info) {
  if (info.Length() > 0 && info[0].IsNumber()) {
    fd_ = info[0].As<Napi::Number>().Int32Value();
  }
}

Napi::Value Http::AddString(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "String Excepted").ThrowAsJavaScriptException();
    return env.Null();
  }
  std::string str = info[0].As<Napi::String>().Utf8Value();
  stringStorage_.push_back(std::move(str));
  const std::string& ref = stringStorage_.back();

  iovecs_.push_back(iovec{
    .iov_base = (void*)ref.data(),
    .iov_len = ref.size()
  });

  return Napi::Number::New(env, iovecs_.size());
}

Napi::Value Http::AddBuffer(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1 || !info[0].IsBuffer()) {
    Napi::TypeError::New(env, "Buffer Excepted").ThrowAsJavaScriptException();
    return env.Null();
  }
  Napi::Buffer<char> buf = info[0].As<Napi::Buffer<char>>();
  iovecs_.push_back(iovec{
    .iov_base = buf.Data(),
    .iov_len = buf.Length()
  });

  return Napi::Number::New(env, iovecs_.size());
}

Napi::Value Http::WriteToFd(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (fd_ == -1) {
    if (info.Length() < 1 || !info[0].IsNumber()) {
      Napi::TypeError::New(env, "File descriptor Excepted").ThrowAsJavaScriptException();
      return env.Null();
    }
    fd_ = info[0].As<Napi::Number>().Int32Value();
  }

  ssize_t totalWritten = 0;
  size_t iovcnt = iovecs_.size();

  while (iovcnt > 0) {
    ssize_t written = writev(fd_, iovecs_.data(), iovcnt);
    if (written == -1) {
      if (errno == EINTR) continue;
      Napi::Error::New(env, strerror(errno)).ThrowAsJavaScriptException();
      return env.Null();
    }
    totalWritten += written;

    ssize_t remaining = written;
    size_t i = 0;
    while (i < iovcnt && remaining >= (ssize_t)iovecs_[i].iov_len) {
      remaining -= iovecs_[i].iov_len;
      i++;
    }

    if (i > 0) {
      iovecs_.erase(iovecs_.begin(), iovecs_.begin() + i);
      iovcnt -= i;
    }

    if (remaining > 0 && i < iovcnt) {
      iovecs_[0].iov_base = (char*)iovecs_[0].iov_base + remaining;
      iovecs_[0].iov_len -= remaining;
    }
  }

  return Napi::Number::New(env, totalWritten);
}

std::string Http::concatAllData() {
  size_t totalLen = 0;
  for (const auto& iov : iovecs_) {
    totalLen += iov.iov_len;
  }
  std::string data;
  data.reserve(totalLen);
  for (const auto& iov : iovecs_) {
    data.append((const char*)iov.iov_base, iov.iov_len);
  }
  return data;
}

Napi::Value Http::FindHeaderEnd(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  const std::string data = concatAllData();
  const std::string needle = "\r\n\r\n";
  size_t pos = data.find(needle);
  if (pos == std::string::npos) return Napi::Number::New(env, -1);
  return Napi::Number::New(env, pos + needle.size());
}

Napi::Value Http::Consume(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1 || !info[0].IsNumber()) {
    Napi::TypeError::New(env, "Offset number gerekli").ThrowAsJavaScriptException();
    return env.Null();
  }
  size_t offset = info[0].As<Napi::Number>().Uint32Value();

  size_t consumed = 0;
  while (!iovecs_.empty() && consumed < offset) {
    if (iovecs_[0].iov_len <= offset - consumed) {
      consumed += iovecs_[0].iov_len;
      iovecs_.erase(iovecs_.begin());
      if (!stringStorage_.empty()) {
        stringStorage_.erase(stringStorage_.begin());
      }
    } else {
      size_t diff = offset - consumed;
      iovecs_[0].iov_base = (char*)iovecs_[0].iov_base + diff;
      iovecs_[0].iov_len -= diff;
      consumed += diff;
      break;
    }
  }
  return Napi::Number::New(env, consumed);
}

Napi::Value Http::GetSlice(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
    Napi::TypeError::New(env, "Offset ve length gerekli").ThrowAsJavaScriptException();
    return env.Null();
  }

  size_t offset = info[0].As<Napi::Number>().Uint32Value();
  size_t length = info[1].As<Napi::Number>().Uint32Value();

  const std::string data = concatAllData();
  if (offset + length > data.size()) {
    Napi::RangeError::New(env, "Slice out of range").ThrowAsJavaScriptException();
    return env.Null();
  }

  return Napi::Buffer<char>::Copy(env, data.data() + offset, length);
}

Napi::Value Http::GetHeaderString(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  const std::string data = concatAllData();
  const std::string needle = "\r\n\r\n";
  size_t pos = data.find(needle);
  if (pos == std::string::npos) return env.Null();
  return Napi::String::New(env, data.substr(0, pos + needle.size()));
}

Napi::Value Http::GetBodyString(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  const std::string data = concatAllData();
  const std::string needle = "\r\n\r\n";
  size_t pos = data.find(needle);
  if (pos == std::string::npos) return env.Null();

  std::string body = data.substr(pos + needle.size());
  return Napi::String::New(env, body);
}

Napi::Value Http::ParseHeaders(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  std::string data = concatAllData();
  size_t headerEnd = data.find("\r\n\r\n");
  
  if (headerEnd == std::string::npos) {
    return env.Null();
  }

  std::string headerString = data.substr(0, headerEnd);
  std::vector<std::string> lines;
  size_t lineStart = 0;
  size_t lineEnd = headerString.find("\r\n");

  while (lineEnd != std::string::npos) {
    lines.push_back(headerString.substr(lineStart, lineEnd - lineStart));
    lineStart = lineEnd + 2;
    lineEnd = headerString.find("\r\n", lineStart);
  }

  if (lines.empty()) {
    return env.Null();
  }

  size_t firstSpace = lines[0].find(' ');
  size_t secondSpace = lines[0].find(' ', firstSpace + 1);
  
  if (firstSpace == std::string::npos || secondSpace == std::string::npos) {
    return env.Null();
  }

  std::string method = lines[0].substr(0, firstSpace);
  std::string url = lines[0].substr(firstSpace + 1, secondSpace - firstSpace - 1);

  Napi::Object headers = Napi::Object::New(env);

  for (size_t i = 1; i < lines.size(); i++) {
    const std::string& line = lines[i];
    if (line.empty()) continue;

    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) continue;

    std::string key = line.substr(0, colonPos);
    size_t keyStart = key.find_first_not_of(" \t");
    if (keyStart == std::string::npos) continue;
    size_t keyEnd = key.find_last_not_of(" \t");
    key = key.substr(keyStart, keyEnd - keyStart + 1);
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    std::string value = line.substr(colonPos + 1);
    size_t valStart = value.find_first_not_of(" \t");
    if (valStart != std::string::npos) {
      size_t valEnd = value.find_last_not_of(" \t");
      value = value.substr(valStart, valEnd - valStart + 1);
    } else {
      value = "";
    }

    headers.Set(Napi::String::New(env, key), Napi::String::New(env, value));
  }

  Napi::Object result = Napi::Object::New(env);
  result.Set("method", Napi::String::New(env, method));
  result.Set("url", Napi::String::New(env, url));
  result.Set("headers", headers);
  result.Set("headerEnd", Napi::Number::New(env, headerEnd + 4));

  return result;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    return Http::Init(env, exports);
}

NODE_API_MODULE(Http, Init)