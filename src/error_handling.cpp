#include "error_handling.hpp"
#include "print_formatter.hpp"
#include <iomanip>
#include <iostream>
#include <sstream>

namespace uil {
    const source_location& SyntaxEvent::where() const {
        return this->location;
    }

    const SyntaxEvent::EventType& SyntaxEvent::get_event_type() const {
        return this->event_type;
    }

    std::string SyntaxEvent::fmt(bool en_color, const std::string& line) const {
        //          | Error: test.txt
        //      123 | int32 a = undefined_var + 14;
        //          |           ^^^^^^^^^^^^^
        //          | 'undefined_var' is not defined in this scope [E12]
        //          |

        std::stringstream out;
        const char* type;
        uint8_t color_idx;

        switch(this->event_type) {
            case EventType::ERROR:      type = "Error";   color_idx = COLOR_IDX_RED;    break;
            case EventType::WARNING:    type = "Warning"; color_idx = COLOR_IDX_PURPLE; break;
            case EventType::NOTE:       type = "Note";    color_idx = COLOR_IDX_CYAN;   break;
        }

        out << "           | ";

        if(en_color) out << make_ansi_code(color_idx, true);
        out << type;
        if(en_color) out << ansi_reset();
        out << " " << location.file_name << std::endl;

        std::string line_number = std::to_string(this->location.line);

        out << std::setw(10) << this->location.line << " | ";
        if(!line.empty()) {
            out << line << std::endl;
            out << std::string(10, ' ') << " | ";
    
            for(unsigned int i = 1; i < this->location.col; i++)
                out << " ";
    
            if(en_color) out << make_ansi_code(COLOR_IDX_RED, true);
    
            for(unsigned int i = 0; i < this->location.length; i++)
                out << "^";
        }

        if(en_color) out << make_ansi_code();
        out << "\n";
        out << "           | ";
        out << this->message;

        if(this->err_no != 0)
            out << " [E" << this->err_no << "]";

        if(en_color) out << make_ansi_code();
        out << "\n";
        out << "           | " << std::endl;

        return out.str();
    }

    SyntaxEvent::SyntaxEvent(SyntaxEvent::EventType event_type, const std::string& message, unsigned int err_no, source_location location)
    : event_type(event_type), message(message), err_no(err_no), location(location) {

    }

    void throw_syntax_error(const std::string &message, unsigned int err_no, source_location location, const std::string &line) {
        SyntaxEvent* event = new SyntaxEvent(SyntaxEvent::EventType::ERROR, message, err_no, location);
        std::cout << event->fmt(true, line) << std::endl;

        throw event;
    }

    void throw_syntax_warning(const std::string &message, unsigned int err_no, source_location location, const std::string &line) {
        SyntaxEvent* event = new SyntaxEvent(SyntaxEvent::EventType::WARNING, message, err_no, location);
        event->fmt(true, line);
        std::cout << event->fmt(true, line) << std::endl;
    }

    void throw_syntax_note(const std::string &message, unsigned int err_no, source_location location, const std::string &line) {
        SyntaxEvent* event = new SyntaxEvent(SyntaxEvent::EventType::NOTE, message, err_no, location);
        event->fmt(true, line);
        std::cout << event->fmt(true, line) << std::endl;
    }
};
