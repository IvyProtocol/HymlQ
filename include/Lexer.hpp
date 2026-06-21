#ifndef LEXER_HPP
#define LEXER_HPP

#include "Token.hpp"
#include <algorithm>
#include <array>
#include <ctre.hpp>
#include <span>
#include <functional>

namespace Lexer
{

using TokenType = Token::TokenType;
using TokenHandler = std::function<void(const Token::TokenData&)>;

class [[
  /* nullAttr */
]] KeywordPair {
public:
  std::string_view text;
  TokenType type;
  std::byte _pad[4]{};
};

constexpr std::array KeyWords
{
  KeywordPair{"#", TokenType::Hash},
  KeywordPair{"$", TokenType::Dollar},
  KeywordPair{"(", TokenType::LeftParen},
  KeywordPair{")", TokenType::RightParen},
  KeywordPair{",", TokenType::Comma},
  KeywordPair{".", TokenType::Dot},
  KeywordPair{"-", TokenType::Hyphen},
  KeywordPair{"=", TokenType::Assign},
  KeywordPair{"[", TokenType::LeftBracket},
  KeywordPair{"]", TokenType::RightBracket},
  KeywordPair{"{", TokenType::LeftBrace},
  KeywordPair{"|", TokenType::Pipe},
  KeywordPair{"}", TokenType::RightBrace},
};

inline constexpr TokenType findKeywordOrIdentifier(std::string_view Toks)
{
  auto it = std::lower_bound
  (
    KeyWords.begin(),
    KeyWords.end(),
    Toks,
    [](const KeywordPair &pair, std::string_view value)
    {
      return pair.text < value;
    }
  );

  if (it != KeyWords.end() && it->text == Toks)
    return it->type;

  return TokenType::Identifier;
}

void Lex(std::span<const char> k_Cbuffer, const TokenHandler& handleToken);

} // namespace Lexer

#endif
