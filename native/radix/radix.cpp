#include <napi.h>
#include <unordered_map>
#include <string>
#include <utility>

class RadixNode : public Napi::ObjectWrap<RadixNode>
{
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    RadixNode(const Napi::CallbackInfo &info);

    Napi::Value GetPrefix(const Napi::CallbackInfo &info);
    void SetPrefix(const Napi::CallbackInfo &info, const Napi::Value &value);
    Napi::Value GetHandler(const Napi::CallbackInfo &info);
    void SetHandler(const Napi::CallbackInfo &info, const Napi::Value &value);
    Napi::Value GetChildren(const Napi::CallbackInfo &info);
    void SetChildren(const Napi::CallbackInfo &info, const Napi::Value &value);

    static Napi::FunctionReference constructor;

private:
    std::string prefix_;
    Napi::FunctionReference handler_;
    Napi::ObjectReference children_;
};

Napi::FunctionReference RadixNode::constructor;

class RadixRouter : public Napi::ObjectWrap<RadixRouter>
{
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    RadixRouter(const Napi::CallbackInfo &info);

    Napi::Value AddRoute(const Napi::CallbackInfo &info);
    Napi::Value FindHandler(const Napi::CallbackInfo &info);
    Napi::Value GetRoot(const Napi::CallbackInfo &info);

    static Napi::FunctionReference constructor;

private:
    Napi::ObjectReference root_;

    void Insert(const Napi::Object &node, const std::string &method, const std::string &path, const Napi::Function &handler);
    size_t CommonPrefixLength(const std::string &a, const std::string &b);
    Napi::Value Search(const Napi::Object &node, const std::string &method, const std::string &path);
};

Napi::FunctionReference RadixRouter::constructor;

Napi::Object RadixNode::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "RadixNode", {
                                                            InstanceAccessor("prefix", &RadixNode::GetPrefix, &RadixNode::SetPrefix),
                                                            InstanceAccessor("handler", &RadixNode::GetHandler, &RadixNode::SetHandler),
                                                            InstanceAccessor("children", &RadixNode::GetChildren, &RadixNode::SetChildren),
                                                        });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("RadixNode", func);
    return exports;
}

RadixNode::RadixNode(const Napi::CallbackInfo &info) : Napi::ObjectWrap<RadixNode>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() > 0 && info[0].IsString())
    {
        prefix_ = info[0].As<Napi::String>().Utf8Value();
    }

    Napi::Object children = Napi::Object::New(env);
    children_ = Napi::Persistent(children);
}

Napi::Value RadixNode::GetPrefix(const Napi::CallbackInfo &info)
{
    return Napi::String::New(info.Env(), prefix_);
}

void RadixNode::SetPrefix(const Napi::CallbackInfo &info, const Napi::Value &value)
{
    if (value.IsString())
    {
        prefix_ = value.As<Napi::String>().Utf8Value();
    }
}

Napi::Value RadixNode::GetHandler(const Napi::CallbackInfo &info)
{
    return handler_.IsEmpty() ? info.Env().Null() : handler_.Value();
}

void RadixNode::SetHandler(const Napi::CallbackInfo &info, const Napi::Value &value)
{
    Napi::Env env = info.Env();
    if (value.IsFunction())
    {
        handler_ = Napi::Persistent(value.As<Napi::Function>());
    }
    else if (value.IsNull() || value.IsUndefined())
    {
        handler_.Reset();
    }
    else
    {
        Napi::TypeError::New(env, "Handler must be a function").ThrowAsJavaScriptException();
    }
}

Napi::Value RadixNode::GetChildren(const Napi::CallbackInfo &info)
{
    return children_.Value();
}

void RadixNode::SetChildren(const Napi::CallbackInfo &info, const Napi::Value &value)
{
    if (value.IsObject())
    {
        children_ = Napi::Persistent(value.As<Napi::Object>());
    }
}

Napi::Object RadixRouter::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "RadixRouter", {
                                                              InstanceMethod("addRoute", &RadixRouter::AddRoute),
                                                              InstanceMethod("findHandler", &RadixRouter::FindHandler),
                                                              InstanceAccessor("root", &RadixRouter::GetRoot, nullptr),
                                                          });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("RadixRouter", func);
    return exports;
}

RadixRouter::RadixRouter(const Napi::CallbackInfo &info) : Napi::ObjectWrap<RadixRouter>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    Napi::Object root = RadixNode::constructor.New({});
    root_ = Napi::Persistent(root);
}

Napi::Value RadixRouter::GetRoot(const Napi::CallbackInfo &info)
{
    return root_.Value();
}

Napi::Value RadixRouter::AddRoute(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() < 3 || !info[0].IsString() || !info[1].IsString() || !info[2].IsFunction())
    {
        Napi::TypeError::New(env, "Expected method (string), path (string), handler (function)").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string method = info[0].As<Napi::String>().Utf8Value();
    std::string path = info[1].As<Napi::String>().Utf8Value();
    Napi::Function handler = info[2].As<Napi::Function>();

    Napi::Object rootNode = root_.Value();
    Napi::Object rootChildren = rootNode.Get("children").As<Napi::Object>();

    Napi::Object methodNode;
    if (rootChildren.Has(method))
    {
        methodNode = rootChildren.Get(method).As<Napi::Object>();
    }
    else
    {
        methodNode = RadixNode::constructor.New({Napi::String::New(env, method)});
        rootChildren.Set(method, methodNode);
    }

    Insert(methodNode, method, path, handler);

    return env.Undefined();
}

void RadixRouter::Insert(const Napi::Object &node, const std::string &method, const std::string &path, const Napi::Function &handler)
{
    Napi::Env env = node.Env();
    Napi::HandleScope scope(env);

    Napi::Object currentNode = node;
    std::string remainingPath = path;

    while (true)
    {
        if (remainingPath.empty())
        {
            currentNode.Set("handler", handler);
            return;
        }

        bool foundChild = false;
        Napi::Object children = currentNode.Get("children").As<Napi::Object>();
        Napi::Array childKeys = children.GetPropertyNames();

        for (uint32_t i = 0; i < childKeys.Length(); i++)
        {
            Napi::Value key = childKeys[i];
            std::string childKey = key.As<Napi::String>().Utf8Value();

            size_t commonLen = CommonPrefixLength(remainingPath, childKey);

            if (commonLen > 0)
            {
                Napi::Object childNode = children.Get(key).As<Napi::Object>();

                if (commonLen == childKey.length())
                {
                    currentNode = childNode;
                    remainingPath = remainingPath.substr(commonLen);
                    foundChild = true;
                    break;
                }
                else
                {
                    std::string commonPrefix = remainingPath.substr(0, commonLen);
                    std::string childSuffix = childKey.substr(commonLen);
                    std::string pathSuffix = remainingPath.substr(commonLen);

                    Napi::Object newParent = RadixNode::constructor.New({Napi::String::New(env, commonPrefix)});

                    childNode.Set("prefix", Napi::String::New(env, childSuffix));

                    Napi::Object newParentChildren = newParent.Get("children").As<Napi::Object>();
                    newParentChildren.Set(childSuffix, childNode);

                    if (!pathSuffix.empty())
                    {
                        Napi::Object newChild = RadixNode::constructor.New({Napi::String::New(env, pathSuffix)});
                        newChild.Set("handler", handler);
                        newParentChildren.Set(pathSuffix, newChild);
                    }
                    else
                    {
                        newParent.Set("handler", handler);
                    }

                    children.Delete(childKey);
                    children.Set(commonPrefix, newParent);

                    return;
                }
            }
        }

        if (!foundChild)
        {
            Napi::Object newNode = RadixNode::constructor.New({Napi::String::New(env, remainingPath)});
            newNode.Set("handler", handler);
            children.Set(remainingPath, newNode);
            return;
        }
    }
}

Napi::Value RadixRouter::FindHandler(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsString())
    {
        Napi::TypeError::New(env, "Expected method (string) and path (string)").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string method = info[0].As<Napi::String>().Utf8Value();
    std::string path = info[1].As<Napi::String>().Utf8Value();

    Napi::Object rootNode = root_.Value();
    Napi::Object rootChildren = rootNode.Get("children").As<Napi::Object>();

    if (!rootChildren.Has(method))
    {
        return env.Null();
    }

    Napi::Object methodNode = rootChildren.Get(method).As<Napi::Object>();

    return Search(methodNode, method, path);
}

Napi::Value RadixRouter::Search(const Napi::Object &node, const std::string &method, const std::string &path)
{
    Napi::Env env = node.Env();
    Napi::HandleScope scope(env);

    std::string remainingPath = path;
    Napi::Object currentNode = node;

    while (true)
    {
        if (remainingPath.empty())
        {
            if (currentNode.Has("handler") && !currentNode.Get("handler").IsNull())
            {
                return currentNode.Get("handler");
            }
            return env.Null();
        }

        bool foundChild = false;
        Napi::Object children = currentNode.Get("children").As<Napi::Object>();
        Napi::Array childKeys = children.GetPropertyNames();

        for (uint32_t i = 0; i < childKeys.Length(); i++)
        {
            Napi::Value key = childKeys[i];
            std::string childKey = key.As<Napi::String>().Utf8Value();

            if (remainingPath.length() >= childKey.length() &&
                remainingPath.substr(0, childKey.length()) == childKey)
            {
                currentNode = children.Get(key).As<Napi::Object>();
                remainingPath = remainingPath.substr(childKey.length());
                foundChild = true;
                break;
            }
        }

        if (!foundChild)
        {
            return env.Null();
        }
    }
}

size_t RadixRouter::CommonPrefixLength(const std::string &a, const std::string &b)
{
    size_t len = std::min(a.length(), b.length());
    for (size_t i = 0; i < len; i++)
    {
        if (a[i] != b[i])
            return i;
    }
    return len;
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    RadixNode::Init(env, exports);
    RadixRouter::Init(env, exports);
    return exports;
}

NODE_API_MODULE(RadixRouter, Init);
