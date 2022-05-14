/// \file logging.h
/// \brief logger for stack trace, assertions 
/// and error msgs.


#ifndef LOGGING_H
#define LOGGING_H
#ifndef STACK_TRACE_SIZE
#define STACK_TRACE_SIZE 5
#endif

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>


#if NEED_LOG_STACK_TRACE & __GNUC__
// stack trace feature requires Linux execinfo syscalls
// and -rdynamic compile flag to fetch symbols
#include <cxxabi.h>
#include <execinfo.h>



std::string BackTrace(
    size_t start_frame = 1,
    const size_t stack_size = STACK_TRACE_SIZE)
{
    std::ostringstream stack_trace_os;
    std::vector<void*> frame_info(stack_size);
    int nframes = backtrace(frame_info.data(), stack_size);
    if (start_frame < nframes) {
        stack_trace_os << "Backtrace:\n";
    }
    char **msgs = backtrace_symbols(frame_info.data(), nframes);
    if (msgs != nullptr) {
        for (int frame = start_frame; frame < nframes; ++frame) {
            stack_trace_os << '\t#' << frame - start_frame << ' ';
            std::string msg(msgs[frame]);
            size_t symbol_start;
            size_t symbol_end;
            /// \brief demangle symbol in stackinfo like
            /// 'a.out(_Zxxxx_xxxx+0x12) [0xabcdxxxx]'
            if (((symbol_start = msg.find("_Z")) != std::string::npos)
                && ((symbol_end = msg.find('+', symbol_start)) != std::string::npos))
            {
                std::string symbol(msg, symbol_start, symbol_end - symbol_start);
                // You can use ::malloc() to allocate a space for demangled symbol 
                // and free manually later. Or just use ref count smart pointer to
                // manage space by RAII.
                // Note that _output buffer is allocate by ::malloc
                int status = 0;
                char* demangled_symbol = abi::__cxa_demangle(symbol.c_str(), NULL, NULL, &status);
                if (status == 0) {
                    stack_trace_os << msg.substr(0, symbol_start) 
                                   << "\033[33m" << std::string(demangled_symbol) << "\033[0m"
                                   << msg.substr(symbol_end, std::string::npos) << "\n";
                    free(demangled_symbol);
                } 
            } else {
                    stack_trace_os << msgs[frame] << "\n";
            }
        }
    }
    free(msgs);
    return stack_trace_os.str();
}

#else 

// dummy backtrace
std::string BackTrace(
    size_t start_frame = 1,
    const size_t stack_size = STACK_TRACE_SIZE)
{
    return std::string("\tdisabled backtrace\n");
}

#endif


class LogFatal {
public:
    LogFatal() = delete;
    LogFatal(const char *file, size_t line) 
    {
        _M_stream << "In " << file << ":" << line << ":\n\t";
    }
    ~LogFatal() { 
        std::cerr << _M_stream.str();
        abort();
     }
    std::ostringstream &stream() { return _M_stream; }

private:
    std::ostringstream _M_stream;
};


#define CHECK_FUNC(name, op)                        \
    template <typename X, typename Y>               \
    bool LogCheck##name(const X &x, const Y &y) {   \
        return (x op y);                            \
    } 

CHECK_FUNC(_LT, <)
CHECK_FUNC(_GT, >)
CHECK_FUNC(_LE, <=)
CHECK_FUNC(_GE, >=)
CHECK_FUNC(_EQ, ==)
CHECK_FUNC(_NE, !=)

#define CHECK_BINARY_OP(name, op, x, y)                                             \
    if (!LogCheck##name(x, y))                                                      \
        LogFatal(__FILE__, __LINE__).stream()                                       \
            <<  "Check failed : "   << #x " " #op " " #y << "\n"                    \
            << BackTrace() << ":"                                                   \

#define CHECK(x)                                                                    \
    if (!(x))                                                                      \
        LogFatal(__FILE__, __LINE__).stream()                                       \
            << "Check failed : " << #x << "\n"                                      \
            << BackTrace() << ":"                                                   

#define CHECK_LT(x, y) CHECK_BINARY_OP(_LT, <, x, y)
#define CHECK_GT(x, y) CHECK_BINARY_OP(_GT, >, x, y)
#define CHECK_LE(x, y) CHECK_BINARY_OP(_LE, <=, x, y)
#define CHECK_GE(x, y) CHECK_BINARY_OP(_GE, >=, x, y)
#define CHECK_EQ(x, y) CHECK_BINARY_OP(_EQ, ==, x, y)
#define CHECK_NE(x, y) CHECK_BINARY_OP(_NE, !=, x, y)



#endif // LOGGING_H