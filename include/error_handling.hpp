#ifndef __ERROR_HANDLING_H
#define __ERROR_HANDLING_H

#include <exception>
#include <string>

namespace uil {
    struct source_location {
        std::string file_name;

        unsigned int line;
        unsigned int col;
        unsigned int length;
    };

    class SyntaxEvent: public std::exception {
        public:
        enum class EventType {
            ERROR,
            WARNING,
            NOTE
        };

        private:
        EventType event_type;
        source_location location;
        std::string message;

        unsigned int err_no;

        public:
        SyntaxEvent(
            EventType event_type, 
            const std::string& message, 
            unsigned int err_no = 0, 
            source_location location = {"", 0, 0, 0}
        );

        const char* what() const noexcept override {
            return this->message.c_str();
        }

        const source_location& where() const;
        const EventType& get_event_type() const;

        std::string fmt(bool en_color = true, const std::string& line = "") const;
    };

    void throw_syntax_error(const std::string& message, unsigned int err_no, source_location location, const std::string& line);
    void throw_syntax_warning(const std::string& message, unsigned int err_no, source_location location, const std::string& line);
    void throw_syntax_note(const std::string& message, unsigned int err_no, source_location location, const std::string& line);
};

#endif
