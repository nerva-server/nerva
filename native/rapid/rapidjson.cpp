#include <napi.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using namespace rapidjson;

Napi::Value Parse(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString())
        Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();

    std::string jsonStr = info[0].As<Napi::String>();

    Document doc;
    if (doc.Parse(jsonStr.c_str()).HasParseError())
    {
        Napi::Error::New(env, "Invalid JSON").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Object result = Napi::Object::New(env);
    for (auto itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr)
    {
        if (itr->value.IsString())
        {
            result.Set(itr->name.GetString(), itr->value.GetString());
        }
        else if (itr->value.IsInt())
        {
            result.Set(itr->name.GetString(), itr->value.GetInt());
        }
        else if (itr->value.IsBool())
        {
            result.Set(itr->name.GetString(), itr->value.GetBool());
        }
        else if (itr->value.IsDouble())
        {
            result.Set(itr->name.GetString(), itr->value.GetDouble());
        }
        else
        {
            result.Set(itr->name.GetString(), env.Null()); // unsupported
        }
    }

    return result;
}

Napi::Value Stringify(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsObject())
        Napi::TypeError::New(env, "Object expected").ThrowAsJavaScriptException();

    Napi::Object obj = info[0].As<Napi::Object>();
    Document doc;
    doc.SetObject();
    Document::AllocatorType &allocator = doc.GetAllocator();

    Napi::Array props = obj.GetPropertyNames();
    for (uint32_t i = 0; i < props.Length(); i++)
    {
        std::string key = props.Get(i).As<Napi::String>();
        Napi::Value val = obj.Get(key);

        if (val.IsString())
        {
            doc.AddMember(Value().SetString(key.c_str(), allocator),
                          Value().SetString(val.As<Napi::String>().Utf8Value().c_str(), allocator), allocator);
        }
        else if (val.IsNumber())
        {
            doc.AddMember(Value().SetString(key.c_str(), allocator),
                          Value().SetDouble(val.As<Napi::Number>().DoubleValue()), allocator);
        }
        else if (val.IsBoolean())
        {
            doc.AddMember(Value().SetString(key.c_str(), allocator),
                          Value().SetBool(val.As<Napi::Boolean>().Value()), allocator);
        }
        else
        {
            doc.AddMember(Value().SetString(key.c_str(), allocator),
                          Value().SetNull(), allocator);
        }
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);

    return Napi::String::New(env, buffer.GetString());
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set("parse", Napi::Function::New(env, Parse));
    exports.Set("stringify", Napi::Function::New(env, Stringify));
    return exports;
}

NODE_API_MODULE(rapidjson_native, Init)
