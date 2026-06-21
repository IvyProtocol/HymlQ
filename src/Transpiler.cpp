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
    for
    (
      char& C : Formatted
    ) if
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
    while
    (
      CurrentIdx != TOML::NodeIdx::None
    ) {

      const auto& Node = Arena.GetNode(CurrentIdx);

      std::visit
      (
        [&](auto&& Payload)
        {
          using T = std::decay_t<decltype(Payload)>;

          if constexpr
          (
            std::is_same_v<T,
            TOML::TableNode>
          ) {

            std::string NewPrefix = Payload.name.empty() ?
            "" : this->FormatIdentifier(Payload.name) + "_";

            for
            (
              char& c : NewPrefix
            ) c = static_cast<char>(
              std::toupper(static_cast<unsigned char>(c))
            );

            NewPrefix = CurrentPrefix + NewPrefix;
            this->Traverse(Arena, Payload.FirstChildIndx, NewPrefix);

          }

          else if constexpr
          (
            std::is_same_v<T,
            TOML::KeyValueNode>
          ) {

            bool IsArray = false;
            bool IsInlineTable = false;

            if
            (
              Node.NextSiblingIndx != TOML::NodeIdx::None
            ) {

              const auto& NextNode = Arena.GetNode(Node.NextSiblingIndx);

              if
              (
                std::holds_alternative<TOML::ArrayNode>(NextNode.Payload)
              ) IsArray = true;

              else if
              (
                std::holds_alternative<TOML::InlineTableNode>(NextNode.Payload)
              ) IsInlineTable = true;

            }

          std::size_t RequiredLength = 7 + CurrentPrefix.length() + Payload.Key.length() + 1 + Payload.Value.length() + 1;
            if
            (
              IsArray
            ) {

              if
              (
                !Payload.Key.empty()
              ) {

                std::string ArrDecl;

                ArrDecl.reserve(CurrentPrefix.length() + Payload.Key.length());

                ArrDecl.append(CurrentPrefix);

                for
                (
                  char c : Payload.Key
                ) ArrDecl.push_back(static_cast<char>(
                  std::toupper(static_cast<unsigned char>(c)))
                );

                const auto& NextNode = Arena.GetNode(Node.NextSiblingIndx);
                const auto& Array = std::get<TOML::ArrayNode>(NextNode.Payload);

                /*
                  Pre-Scan: Does this array contain complex objects like Inline Tables?
                */

                bool ContainsComplex = false;
                auto ScanIdx = Array.FirstChildIndx;

                while
                (
                  ScanIdx != TOML::NodeIdx::None
                ) {

                  const auto& ScanNode = Arena.GetNode(ScanIdx);

                  if
                  (
                    ScanNode.NextSiblingIndx != TOML::NodeIdx::None
                  ) {

                    const auto& NextScan = Arena.GetNode(ScanNode.NextSiblingIndx);

                    if
                    (
                      std::holds_alternative<TOML::InlineTableNode>(NextScan.Payload)
                    ) {
                      ContainsComplex = true;
                      break;

                    }
                  }

                  ScanIdx = ScanNode.NextSiblingIndx;
                }

                if
                (
                  ContainsComplex
                ) this->Writer_.Write("declare -A " + ArrDecl + "\n");

                else [[
                  /* nullAttr */
                ]] this->Writer_.Write("export " + ArrDecl + "=( ");


                auto ProcessArrayElements = [&](
                  auto& Self,
                  TOML::NodeIdx CurrentElemIdx,
                  int& IndexCounter
                ) -> void
                {
                  while
                  (
                    CurrentElemIdx != TOML::NodeIdx::None
                  ) {

                    const auto& ElemNode = Arena.GetNode(CurrentElemIdx);
                    const auto& ElemPayload = std::get<TOML::KeyValueNode>(ElemNode.Payload);

                    bool ElemIsArray = false;
                    bool ElemIsInlineTable = false;

                    if
                    (
                      ElemNode.NextSiblingIndx != TOML::NodeIdx::None
                    ) {

                      const auto& NextElemNode = Arena.GetNode(ElemNode.NextSiblingIndx);

                      if
                      (
                        std::holds_alternative<TOML::ArrayNode>(NextElemNode.Payload)
                      ) ElemIsArray = true;

                      else if
                      (
                        std::holds_alternative<TOML::InlineTableNode>(NextElemNode.Payload)
                      ) ElemIsInlineTable = true;
                    }

                    if
                    (
                      ElemIsArray
                    ) {

                      const auto& NextElemNode = Arena.GetNode(ElemNode.NextSiblingIndx);
                      const auto& NestedArray = std::get<TOML::ArrayNode>(NextElemNode.Payload);

                      Self(Self, NestedArray.FirstChildIndx, IndexCounter);

                      CurrentElemIdx = ElemNode.NextSiblingIndx;

                    }

                    else if
                    (
                      ElemIsInlineTable
                    ) {

                      const auto& NextElemNode = Arena.GetNode(ElemNode.NextSiblingIndx);
                      const auto& NestedTable = std::get<TOML::InlineTableNode>(NextElemNode.Payload);

                      // Mini Lambda to extract the properties of the inline table.
                      auto ProcessInlineInArray = [&](
                        [[maybe_unused]] auto& InnerSelf,
                        TOML::NodeIdx PropIndx,
                        std::string KeyPrefix
                      ) -> void {

                        while
                        (
                          PropIndx != TOML::NodeIdx::None
                        ) {

                          const auto& PropNode = Arena.GetNode(PropIndx);
                          const auto& PropPayload = std::get<TOML::KeyValueNode>(PropNode.Payload);

                          bool PropIsArr = false;
                          bool PropIsTabl = false;

                          if
                          (
                            PropNode.NextSiblingIndx != TOML::NodeIdx::None
                          ) {

                            const auto& Next = Arena.GetNode(PropNode.NextSiblingIndx);

                            if
                            (
                              std::holds_alternative<TOML::ArrayNode>(Next.Payload)
                            ) PropIsArr = true;

                            else if
                            (
                              std::holds_alternative<TOML::InlineTableNode>(Next.Payload)
                            ) PropIsTabl = true;

                          }

                          std::string FullKey = KeyPrefix + PropPayload.Key;

                          if
                          (
                            PropIsArr
                          ) {

                            const auto& NextPropNode = Arena.GetNode(PropNode.NextSiblingIndx);
                            const auto& NestedArray = std::get<TOML::ArrayNode>(NextPropNode.Payload);

                            auto ElemIdx = NestedArray.FirstChildIndx;
                            std::string ElemStr = "(";

                            while
                            (
                              ElemIdx != TOML::NodeIdx::None
                            ) {

                              const auto& EleNode = Arena.GetNode(ElemIdx);
                              const auto& ElePayload = std::get<TOML::KeyValueNode>(EleNode.Payload);

                              ElemStr.append(ElePayload.Value).append(" ");
                              ElemIdx = EleNode.NextSiblingIndx;
                            }
                            ElemStr.append(")");

                            std::string Assign = ArrDecl + "[\"" + FullKey + "\"]=" + ElemStr + "\n";
                            this->Writer_.Write(Assign);

                            PropIndx = PropNode.NextSiblingIndx;
                          }

                          else if
                          (
                            PropIsTabl
                          ) {

                            const auto& NextPropNode = Arena.GetNode(PropNode.NextSiblingIndx);
                            const auto& NestTable = std::get<TOML::InlineTableNode>(NextPropNode.Payload);

                            InnerSelf(InnerSelf, NestTable.FirstChildIndx, FullKey + "_");
                            PropIndx = PropNode.NextSiblingIndx;
                          }

                          else [[
                            /* nullAttr */
                          ]] {

                            std::string Assign = ArrDecl + "[\"" + FullKey + "\"]=" + PropPayload.Value + "\n";
                            this->Writer_.Write(Assign);
                          }

                          PropIndx = Arena.GetNode(PropIndx).NextSiblingIndx;
                        }
                      };

                      std::string BasePrefix = std::to_string(IndexCounter) + "_";

                      ProcessInlineInArray(
                        ProcessInlineInArray,
                        NestedTable.FirstChildIndx,
                        BasePrefix
                      );

                      CurrentElemIdx = ElemNode.NextSiblingIndx;
                      IndexCounter++;
                    }

                    else [[
                      /* nullAttr */
                    ]] {

                      if
                      (
                        ContainsComplex
                      ) {

                        std::string Assign = ArrDecl + "[\"" + std::to_string(IndexCounter) + "\"]=" + ElemPayload.Value + "\n";

                        this->Writer_.Write(Assign);
                        IndexCounter++;
                      }

                      else [[
                        /* nullAttr */
                      ]] {

                        std::string ElemStr;
                        ElemStr.reserve(ElemPayload.Value.length() + 1);

                        ElemStr.append(ElemPayload.Value).append(" ");
                        this->Writer_.Write(ElemStr);
                      }
                    }

                    CurrentElemIdx = Arena.GetNode(CurrentElemIdx).NextSiblingIndx;
                  }
                };

                int StartIndex = 0;
                ProcessArrayElements(ProcessArrayElements, Array.FirstChildIndx, StartIndex);

                if
                (
                  !ContainsComplex
                ) this->Writer_.Write(")\n");

              }
              CurrentIdx = Node.NextSiblingIndx;

            }

            else if
            (
              IsInlineTable
            ) {

              if
              (
                !Payload.Key.empty()
              ) {
                std::string MapName = CurrentPrefix;


                for
                (
                  char C : Payload.Key
                ) MapName.push_back(static_cast<char>(
                  std::toupper(static_cast<unsigned char>(C)))
                );


                this->Writer_.Write("declare -A " + MapName + "\n");


                const auto& NextNode = Arena.GetNode(Node.NextSiblingIndx);
                const auto& InlineTable = std::get<TOML::InlineTableNode>(NextNode.Payload);

                auto ProcessProperties = [&](
                  auto& Self,
                  TOML::NodeIdx CurrentPropIndx,
                  std::string KeyPrefix
                ) -> void
                {
                  while
                  (
                    CurrentPropIndx != TOML::NodeIdx::None
                  ) {

                    const auto& PropNode = Arena.GetNode(CurrentPropIndx);
                    const auto& PropPayload = std::get<TOML::KeyValueNode>(PropNode.Payload);

                    bool PropIsArray = false;
                    bool PropIsInlineTable = false;

                    if
                    (
                      PropNode.NextSiblingIndx != TOML::NodeIdx::None
                    ) {
                      const auto& NextPropNode = Arena.GetNode(PropNode.NextSiblingIndx);

                      if
                      (
                        std::holds_alternative<TOML::ArrayNode>(NextPropNode.Payload)
                      ) PropIsArray = true;

                      else if
                      (
                        std::holds_alternative<TOML::InlineTableNode>(NextPropNode.Payload)
                      ) PropIsInlineTable = true;
                    }

                   std::string FullKey = KeyPrefix + PropPayload.Key;
                    if
                    (
                      PropIsArray
                    ) {
                      const auto& NextPropNode = Arena.GetNode(PropNode.NextSiblingIndx);
                      const auto& Array = std::get<TOML::ArrayNode>(NextPropNode.Payload);

                      auto ElemIdx = Array.FirstChildIndx;

                      std::string ElemStr = "( ";
                      while
                      (
                        ElemIdx != TOML::NodeIdx::None
                      ) {
                        const auto& ElemNode = Arena.GetNode(ElemIdx);
                        const auto& ElemPayload = std::get<TOML::KeyValueNode>( ElemNode.Payload);

                        ElemStr.append(ElemPayload.Value).append(" ");
                        ElemIdx = ElemNode.NextSiblingIndx;
                      }

                      ElemStr.append(")");

                      std::string Assign = MapName + "[\"" + FullKey + "\"]=" + ElemStr + "\n";
                      this->Writer_.Write(Assign);

                      CurrentPropIndx = PropNode.NextSiblingIndx;
                    }

                    else if
                    (
                      PropIsInlineTable
                    ) {
                      const auto& NextPropNode = Arena.GetNode(PropNode.NextSiblingIndx);
                      const auto& Nestedtable = std::get<TOML::InlineTableNode>(NextPropNode.Payload);

                    Self(Self, Nestedtable.FirstChildIndx, FullKey + "_");

                    CurrentPropIndx = PropNode.NextSiblingIndx;
                    }

                    else [[
                      /* nullAttr */
                    ]] {

                      std::string Assign;
                      Assign.reserve(
                      MapName.length() + FullKey.length() + PropPayload.Value.length() + 6
                      );
                      Assign.append(MapName).append("[\"").append(FullKey).append("\"]=").append(PropPayload.Value).append("\n");

                      this->Writer_.Write(Assign);
                    }

                    CurrentPropIndx = Arena.GetNode(CurrentPropIndx).NextSiblingIndx;
                  }

                };

                ProcessProperties(ProcessProperties, InlineTable.FirstChildIndx, "");

              }
              CurrentIdx = Node.NextSiblingIndx;

            }

            else [[
              /* nullAttr */
            ]] {

              if
              (
                !Payload.Key.empty()
              ) {
                std::string Line;
                Line.reserve(RequiredLength);

                Line.append("export ").append(CurrentPrefix);

                for
                (
                  char c : Payload.Key
                ) Line.push_back(static_cast<char>(
                  std::toupper(static_cast<unsigned char>(c)))
                );

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
