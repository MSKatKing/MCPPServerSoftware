#ifndef TEXT_COMPONENTS_H
#define TEXT_COMPONENTS_H

#include "../../io/json.h"
#include <string.h>

class TextComponent {
private:
    JSON json;

public:
    TextComponent(const std::string& text) {
        json.writeString("text", text);
    }

    const std::string asString() const;
};



#endif //TEXT_COMPONENTS_H
