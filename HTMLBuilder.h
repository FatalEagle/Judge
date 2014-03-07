#ifndef HTMLBUILDER_H_INCLUDED
#define HTMLBUILDER_H_INCLUDED

#include "basicresource.h"

class HTMLBuilder
{
private:
    std::ostream& out;
    std::stack<std::string> tags;
    bool enabled;
public:
    HTMLBuilder(std::ostream& out_arg, bool enabled_arg):
        out(out_arg),
        enabled(enabled_arg)
    {
        if(enabled)
            out<<"<!DOCTYPE html>";
        push("html");
        push("head");
    }
    ~HTMLBuilder()
    {
        close();
    }
    static void HTMLencode(std::string& data)
    {
        std::string buffer;
        buffer.reserve(data.size()+10);
        for(size_t pos=0; pos!=data.size(); ++pos)
        {
            switch(data[pos])
            {
            case '&':
                buffer.append("&amp;");
                break;
            case '\"':
                buffer.append("&quot;");
                break;
            case '\'':
                buffer.append("&apos;");
                break;
            case '<':
                buffer.append("&lt;");
                break;
            case '>':
                buffer.append("&gt;");
                break;
            case '\n':
                buffer.append("<br>");
                break;
            default:
                buffer.append(&data[pos], 1);
                break;
            }
        }
        data.swap(buffer);
    }
    void open()
    {
        while(!empty() && pop()!="head");
        push("body");
    }
    void close()
    {
        if(enabled)
        {
            while(!pop().empty());
            enabled=false;
        }
    }
    bool empty()
    {
        return tags.empty();
    }
    void push(std::string tag)
    {
        if(enabled)
        {
            out<<"<"<<tag<<">";
            tags.push(tag.substr(0, tag.find(' ')));
        }
    }
    std::string pop()
    {
        if(enabled && !empty())
        {
            std::string tag=tags.top();
            tags.pop();
            out<<"</"<<tag<<">";
            return tag;
        }
        return std::string();
    }
    bool setstate(bool newstate)
    {
        bool oldstate=enabled;
        enabled=newstate;
        return oldstate;
    }
    template<class Arg>
    HTMLBuilder& operator<< (Arg arg)
    {
        if(enabled)
            out<<arg;
        return *this;
    }
};

#endif // HTMLBUILDER_H_INCLUDED
