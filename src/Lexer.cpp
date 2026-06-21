#include "Lexer.hpp"
#include "Token.hpp"
#include <ctre.hpp>
#include <cctype>

namespace Lexer
{

static constexpr auto TokenPattern = ctll::fixed_string
{
  R"((?<comment>/\*([^*]|\*+[^*/])*\*+/|//[^\n]*|#[^\n]*))"
  R"(|(?<float_literal>\d+\.\d+)|(?<int_literal>\d+))"

  R"(|(?<string_literal>"(?:[^"\\]|\\.)*")|(?<raw_string_literal>'(?:[^'\\]|\\.)*'))"

  R"(|(?<ident>[a-zA-Z_][a-zA-Z0-9_]*))"
  R"(|(?<op>[=&#.:;,\(\)\{\}\[\]\|\$|\-]))"
};

void Lex
(
  std::span<const char> kC_buffer,
  const TokenHandler& handleToken
) {
  std::size_t _st_fileOffset_ {};
  std::size_t _st_fileLine_ = 1;
  std::size_t _st_fileColumn_ = 1;

  while
  (
    _st_fileOffset_ < kC_buffer.size()
  ) {
    auto _sv_fileSlice_ = std::string_view(
      kC_buffer.subspan(_st_fileOffset_).data(),
       kC_buffer.size() - _st_fileOffset_
    );

    char _sv_CfileSlice_zI = _sv_fileSlice_[0];

    // Handle whitespace
    if
    (
      std::isspace
      (
        static_cast<unsigned char>(_sv_CfileSlice_zI)
      )
    ) {

      if
      (
        _sv_CfileSlice_zI == '\n'
      ) {

        _st_fileLine_++;
        _st_fileColumn_ = 1;
      }

      else if
      (
        _sv_CfileSlice_zI == '\t'
      ) _st_fileColumn_ += 4;

      else [[
        /* nullAttr */
      ]] _st_fileColumn_++;

      _st_fileOffset_++;
      continue;

    }

    if
    (
      auto match = ctre::starts_with<TokenPattern>(_sv_fileSlice_)
    ) {
      std::string_view _sv_Mstr_ = match.to_view();

      Token::TokenType type = Token::TokenType::Unknown;
      if
      (
        match.get<"comment">()
      ) type = Token::TokenType::Hash;

      else if
      (
        match.get<"float_literal">()
      ) type = Token::TokenType::FloatLiteral;

      else if
      (
        match.get<"int_literal">()
      ) type = Token::TokenType::IntLiteral;

      else if
      (
        match.get<"string_literal">()
      ) type = Token::TokenType::StringLiteral;

      else if
      (
        match.get<"raw_string_literal">()
      ) type = Token::TokenType::RawStringLiteral;

      else if
      (
        match.get<"ident">() ||
        match.get<"op">()
      ) type = findKeywordOrIdentifier(_sv_Mstr_);

      if
      (
        !match.get<"comment">()
      ) {

        handleToken
        (
          Token::TokenData
          {
            .Type = type,
            .Sv_Val_ = _sv_Mstr_,
            .Lexer_Size_t_Line_ = _st_fileLine_,
            .Lexer_Size_t_Column_ = _st_fileColumn_,
            .Lexer_Size_t_Offset_ = _st_fileOffset_
          }
        );

      }

      for
      (
        char ch : _sv_Mstr_
      ) if
        (
          ch == '\n'
        ) {
          _st_fileLine_++;
          _st_fileColumn_ = 1;
        }

        else if
        (
          ch == '\t'
        ) _st_fileColumn_ += 4;
        else [[
          /* nullAttr */
        ]] _st_fileColumn_++;

      _st_fileOffset_ += _sv_Mstr_.length();
    }

    else [[
      /* nullAttr */
    ]] {

      handleToken
      (
        Token::TokenData
        {
          .Type = Token::TokenType::Unknown,
          .Sv_Val_ = _sv_fileSlice_.substr(0, 1),
          .Lexer_Size_t_Line_ = _st_fileLine_,
          .Lexer_Size_t_Column_ = _st_fileColumn_,
          .Lexer_Size_t_Offset_ = _st_fileOffset_
        }
      );

      _st_fileOffset_++;
      _st_fileColumn_++;

    }
  }

  // 4. EOF: safely appended Once reaches the end of file.
  handleToken
  (
    Token::TokenData
    {
      .Type = Token::TokenType::EndOfFile,
      .Sv_Val_ = "",
      .Lexer_Size_t_Line_ = _st_fileLine_,
      .Lexer_Size_t_Column_ = _st_fileColumn_,
      .Lexer_Size_t_Offset_ = _st_fileOffset_
    }
  );

}
}
