#include "token_reader.h"

TokenReader::TokenReader(std::istream& is) : inputStream(is) {}

void TokenReader::skipCommentLine() const {
    char c;
    while (inputStream.get(c)) {
        if (c == '\n') {
            break;
        }
    }
}

void TokenReader::skipWhitespace() const {
    char c;
    while (inputStream.get(c)) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            inputStream.unget();
            break;
        }
    }
}

void TokenReader::skipWhitespaceAndComments() const {
    while (true) {
        int peekChar = inputStream.peek();
        if (peekChar == EOF) {
            return;
        }
        char c = static_cast<char> (peekChar);
        if (std::isspace(static_cast<unsigned char>(c))) {
            skipWhitespace();
            continue;
        }
        if (c == '#') {
            inputStream.get();
            skipCommentLine();
            continue;
        }

        return;

    }
}

    std::string TokenReader::nextToken() {
    skipWhitespaceAndComments();
    char c;
    std::string token;
    while (inputStream.get(c)) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            break;
        }
        if (c == '#') {
            if (token.empty()) {
                skipCommentLine();
                skipWhitespaceAndComments();
                continue;
            } else {
                inputStream.unget();
                break;
            }
        }
        token.push_back(c);

    }
    if (token.empty()) {
        throw std::runtime_error("No more tokens available");
    }
    return  token;


}

int TokenReader::nextIntToken() {
    std::string token = nextToken();
    try {
        return std::stoi(token);
    } catch (const std::invalid_argument& e) {
        throw std::runtime_error("Invalid integer token: " + token);
    } catch (const std::out_of_range& e) {
        throw std::runtime_error("Integer token out of range: " + token);
    }
}



