#ifndef TRANSPILER_HPP
#define TRANSPILER_HPP

#include "AST.hpp"
#include "FileHandler.hpp"

namespace Transpiler
{
  class [[
    /* nullAttr */
  ]] Transpile
  {
    private:
      FileHandler::FileWriter& Writer_;

      auto Traverse
      (
        const TOML::ASTArena& Arena,
        TOML::NodeIdx CurrentIdx,
        std::string CurrentPrefix
      ) noexcept -> void;
      [[nodiscard]] auto FormatIdentifier(std::string_view Name)
      const -> std::string;

    public:
      explicit Transpile(FileHandler::FileWriter& Writer) noexcept;
      auto Generate
      (
        const TOML::ASTArena& Arena,
        TOML::NodeIdx RootIdx
      ) noexcept -> void;
  };
}

#endif
