#include "Transpiler.hpp"
#include "AST.hpp"
#include <variant>

namespace Transpiler
{
  Transpile::Transpile(FileHandler::FileWriter& Writer) noexcept
      : Writer_(Writer) {}

  auto Transpile::FormatIdentifier(std::string_view Name)
  const -> std::string
  {
    std::string Formatted(Name);
    for (char& C : Formatted)
      if
      (
        C == '.' ||
        C == '-'
      ) C = '_';

    return Formatted;
  }

  auto Transpile::Generate
  (
    const TOML::ASTArena& Arena,
    TOML::NodeIdx RootIdx
  ) noexcept -> void
  {
    this->Traverse(Arena, RootIdx, "");
  }

  auto Transpile::Traverse
  (
    const TOML::ASTArena& Arena,
    TOML::NodeIdx CurrentIdx,
    std::string CurrentPrefix
  ) noexcept -> void
  {
    while (CurrentIdx != TOML::NodeIdx::None)
    {
      const auto& Node = Arena.GetNode(CurrentIdx);

      std::visit
      (
        [&](auto&& Payload)
        {
          using T = std::decay_t<decltype(Payload)>;

          if constexpr (std::is_same_v<T, TOML::TableNode>)
          {
            std::string NewPrefix = Payload.name.empty() ? "" : this->FormatIdentifier(Payload.name) + "_";

            for (char& c : NewPrefix)
              c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

            NewPrefix = CurrentPrefix + NewPrefix;
            this->Traverse(Arena, Payload.FirstChildIndx, NewPrefix);
          }
          else if constexpr (std::is_same_v<T, TOML::KeyValueNode>)
          {
            bool IsArray = false;
            if (Node.NextSiblingIndx != TOML::NodeIdx::None)
            {
              const auto& NextNode = Arena.GetNode(Node.NextSiblingIndx);

              if (std::holds_alternative<TOML::ArrayNode>(NextNode.Payload))
                IsArray = true;
            }

          std::size_t RequiredLength = 7 + CurrentPrefix.length() + Payload.Key.length() + 1 + Payload.Value.length() + 1;
            if (IsArray)
            {
              if (!Payload.Key.empty())
              {
                std::string ArrDecl;


                ArrDecl.reserve(RequiredLength);

                ArrDecl.append("export ").append(CurrentPrefix);

                for ( char c : Payload.Key)
                  ArrDecl.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));

                ArrDecl.append("=( ");
                this->Writer_.Write(ArrDecl);

                const auto& NextNode = Arena.GetNode(Node.NextSiblingIndx);
                const auto& Array = std::get<TOML::ArrayNode>(NextNode.Payload);
                auto ElemIdx = Array.FirstChildIndx;

                while (ElemIdx != TOML::NodeIdx::None)
                {
                  const auto& ElemNode = Arena.GetNode(ElemIdx);
                  const auto& ElemPayload = std::get<TOML::KeyValueNode>(ElemNode.Payload);

                  std::string ElemStr;
                  ElemStr.reserve(ElemPayload.Value.length() + 1);

                  ElemStr.append(ElemPayload.Value).append(" ");
                  this->Writer_.Write(ElemStr);

                  ElemIdx = ElemNode.NextSiblingIndx;
                }
                this->Writer_.Write(")\n");
              }
              CurrentIdx = Node.NextSiblingIndx;
            }
            else
            {
              if (!Payload.Key.empty())
              {
                std::string Line;
                Line.reserve(RequiredLength);

                Line.append("export ").append(CurrentPrefix);

                for (char c : Payload.Key)
                  Line.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));

                Line.append("=").append(Payload.Value).append("\n");
                this->Writer_.Write(Line);
              }
            }
          }
        }, Node.Payload
      );
      CurrentIdx = Arena.GetNode(CurrentIdx).NextSiblingIndx;
    }
  }
}
