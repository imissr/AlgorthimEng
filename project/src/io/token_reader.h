//
// Created by khale on 11.01.2026.
//

#ifndef ALGENG_TOKEN_READER_H
#define ALGENG_TOKEN_READER_H
#include <iostream>
#include <string>
#include <cctype>
#include <stdexcept>

class TokenReader {
public:
    TokenReader(std::istream& is);

    std::string nextToken();
    int nextIntToken();

private:
    std::istream& inputStream;
    void skipCommentLine() const;
    void skipWhitespace() const;
    void skipWhitespaceAndComments() const;



};



#endif //ALGENG_TOKEN_READER_H