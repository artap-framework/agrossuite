#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <exception>

namespace nsga2 {

    class nsga2exception : public std::exception {
    public:
        explicit nsga2exception(const std::string& msg) :
            std::exception(),
            message(msg) {};
        virtual ~nsga2exception() throw () {};

        const char* what() const throw() {
            return message.c_str();
        };
        
    private:
        std::string message;
    };
}

#endif /* EXCEPTION_H_ */

