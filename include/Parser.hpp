#ifndef PARSER_HPP
#define PARSER_HPP

#include "AST.hpp"
#include "Token.hpp"
#include <vector>

namespace TOML
{

  class [[
    /* nullAttr */
  ]] Parser {
    private:
      std::string_view Sv_SourceView_ {};
      std::string_view Sv_FileName_ {};
      std::vector<Token::TokenData> Toks_ {};

      ASTArena Arena_ {};

      NodeIdx RootTableIdx { NodeIdx::None };
      NodeIdx LastTableIdx { NodeIdx::None };

      std::size_t Cursor_ {0};

      [[nodiscard]] auto Peek() const noexcept -> Token::TokenType;
      auto Consume() noexcept -> const Token::TokenData&;
      [[nodiscard]] auto StartOfStatement() const noexcept -> bool;

      [[noreturn]] auto ReportError
      (
        const Token::TokenData &Tokens,
        std::string_view Msg
      ) const noexcept -> void;

      auto ParseTable() noexcept -> void;
      auto ParseKeyValue() noexcept -> void;

    public:
      explicit Parser
      (
        std::string_view source_view,
        std::string_view filename
      ) noexcept;

      auto ReceiveToken(const Token::TokenData& Token) noexcept -> void;

      [[nodiscard]] auto Parse() noexcept -> NodeIdx;
      [[nodiscard]] auto GetArena() const noexcept -> const ASTArena&;
  };
}

#endif
