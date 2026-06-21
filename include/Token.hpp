#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <cstdint>
#include <string_view>

namespace Token
{
  enum class [[
    /* nullAttr */
  ]] TokenType : std::int32_t {

    /*
    * Assignment Operator
    */

    Assign,       // AssignOperator

    /*
    * BitWise Operators
    */

    Pipe,       // PipeOperator

    /*
    * Separator/Temrinator
    */

    Comma,       // CommaSeparator
    Dot,         // DotSeparator
    Hash,        // HashSeparator
    Hyphen,

    /*
    * Brackets
    */

    LeftParen,  // LeftParen

    RightParen, // RightParen

    LeftBrace,  // Leftbrace
    RightBrace, // RightBrace

    LeftBracket,  // LeftBracket
    RightBracket, // RightBracket


    /*
    * Operators
    */
    Plus,
    Minus,
    Div,
    Mul,

    /*
    * Literals
    */
    FloatLiteral,     // FloatLiteral
    IntLiteral,       // IntLiteral
    StringLiteral,    // StringLiteral
    RawStringLiteral, // RawStringLiteral

    /*
    * Default
    */

    Dollar,     // DollarIdentifier
    Identifier, // IdentifierMarker
    Unknown,    // UnknownMarker
    EndOfFile,  // EndOfFileMarker

  };

  class [[
    /* nullAttr*/
  ]] TokenData {
    public:
      TokenType Type;
      long : (8 * 4);
      std::string_view Sv_Val_;
      std::size_t Lexer_Size_t_Line_;
      std::size_t Lexer_Size_t_Column_;
      std::size_t Lexer_Size_t_Offset_;
  };
} // namespace Token

#endif
