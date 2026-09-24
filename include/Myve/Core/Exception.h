#pragma once

#include <exception>
#include <string>
#include <source_location>
#include <vulkan/vulkan.h>

namespace myve {

const char* string_VkResult(VkResult result);

class MyveException : public std::exception {
public:
    MyveException(
        std::string message,
        std::source_location location = std::source_location::current()
    );

    MyveException(
        VkResult result,
        std::source_location location = std::source_location::current()
    );

    MyveException(
        VkResult result,
        std::string message,
        std::source_location location = std::source_location::current()
    );

    MyveException(
        int line,
        const char* file,
        VkResult result = VK_SUCCESS,
        std::string message = ""
    );

    const char* what() const noexcept override;

    int getLine() const noexcept { return m_line; }
    const std::string& getFile() const noexcept { return m_file; }
    const std::string& getFunction() const noexcept { return m_function; }
    VkResult getResult() const noexcept { return m_result; }
    const std::string& getMessage() const noexcept { return m_rawMessage; }

private:
    void formatFullMessage();

    int m_line;
    std::string m_file;
    std::string m_function;
    VkResult m_result{VK_SUCCESS};
    std::string m_rawMessage;
    std::string m_formattedMessage;
};

} // namespace myve

// Macros no estilo familiar para o desenvolvedor
#define _throw throw ::myve::MyveException("", std::source_location::current())
#define _throwMsg(msg) throw ::myve::MyveException((msg), std::source_location::current())
#define _throwVk(res) \
    do { \
        VkResult _res = (res); \
        if (_res != VK_SUCCESS) { \
            throw ::myve::MyveException(_res, std::source_location::current()); \
        } \
    } while (0)

#define MYVE_CHECK_VK(res) _throwVk(res)
#define MYVE_CHECK_VK_MSG(res, msg) \
    do { \
        VkResult _res = (res); \
        if (_res != VK_SUCCESS) { \
            throw ::myve::MyveException(_res, (msg), std::source_location::current()); \
        } \
    } while (0)
#define MYVE_THROW_MSG(msg) _throwMsg(msg)
