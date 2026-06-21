#include "Parser.hpp"
#include "AST.hpp"
#include "Logger.hpp"
#include <vector>
#include <print>
#include <cstdlib>

namespace TOML
{
  Parser::Parser
  (
    std::string_view source_view,
    std::string_view filename
  ) noexcept : Sv_SourceView_(source_view), Sv_FileName_(filename)
  {
    TOML::TableNode RootTable
    {
      .name = "",
      .FirstChildIndx = NodeIdx::None,
      .IsArrayOfTable = false,
      ._pad = {}
    };

    this->RootTableIdx = this->Arena_.EmplaceNode(std::move(RootTable));
    this->LastTableIdx = this->RootTableIdx;
  }

  auto Parser::Peek() const noexcept -> Token::TokenType
  {

    size_t TempCursor = this->Cursor_;

    while
    (
      TempCursor < this->Toks_.size() &&
      this->Toks_[TempCursor].Type == Token::TokenType::Hash
    ) TempCursor++;

    if
    (
      this->Cursor_ >= this->Toks_.size()
    ) return Token::TokenType::EndOfFile;

    return this->Toks_[this->Cursor_].Type;
  }

  auto Parser::Consume() noexcept
  -> const Token::TokenData&
  {

    while
    (
      this->Cursor_ < this->Toks_.size() &&
      this->Toks_[this->Cursor_].Type == Token::TokenType::Hash
    ) this->Cursor_++;

    return this->Toks_[this->Cursor_++];
  }

  auto Parser::StartOfStatement() const noexcept -> bool
  {
    if
    (
      this->Cursor_ >= this->Toks_.size()
    ) return true;

    const auto Type = this->Toks_[this->Cursor_].Type;

    if
    (
      Type == Token::TokenType::LeftBracket ||
      Type == Token::TokenType::EndOfFile
    ) return true;

    if
    (
      Type == Token::TokenType::Identifier
    ) if
      (
        this->Cursor_ + 1 < this->Toks_.size()
      ) return this->Toks_[this->Cursor_ + 1].Type == Token::TokenType::Assign;

    return false;
  }

  auto Parser::ReportError
  (
    const Token::TokenData &Tokens,
    std::string_view Msg
  ) const noexcept -> void
  {

    std::println
    (
      stderr,
      "\033[1m{}:{}:{}:\033[0m {}{}: {}{}\033[0m",
      this->Sv_FileName_,
      Tokens.Lexer_Size_t_Line_,
      Tokens.Lexer_Size_t_Column_,
      ConsoleColor::BOLD_RED,
      "error",
      ConsoleColor::RESET,
      Msg
    );

    if
    (
      Tokens.Lexer_Size_t_Line_ == 0 ||
      this->Sv_SourceView_.empty()
    ) std::exit(1);

    size_t st_LineStart_ = Tokens.Lexer_Size_t_Offset_;

    while
    (
      st_LineStart_ > 0 &&
      this->Sv_SourceView_[st_LineStart_ - 1] != '\n'
    ) st_LineStart_--;

    size_t st_LineEnd_ = Tokens.Lexer_Size_t_Offset_;

    while
    (
      st_LineEnd_ < this->Sv_SourceView_.size() &&
      this->Sv_SourceView_[st_LineEnd_] != '\n'
    ) st_LineEnd_++;

    std::string_view Sv_SourceLine_ = this->Sv_SourceView_.substr
    (st_LineStart_,
      st_LineEnd_ - st_LineStart_
    );

    std::println(stderr, "{:<5} | {}", Tokens.Lexer_Size_t_Line_, Sv_SourceLine_);
    const size_t kSt_PadLen_ = (Tokens.Lexer_Size_t_Column_ > 0) ? (Tokens.Lexer_Size_t_Column_ - 1 ) : 0;

    std::string Pad(kSt_PadLen_, ' ');

    std::string Underline = "^";

    if
    (
      Tokens.Sv_Val_.length() > 1
    ) Underline.append(Tokens.Sv_Val_.length() - 1, '~');

    std::println
    (
      stderr,
      "      | {}{}{}{}",
      Pad,
      ConsoleColor::BOLD_RED,
      Underline,
      ConsoleColor::RESET
    );
    std::exit(1);
  }

  auto Parser::ParseTable() noexcept -> void
  {
    this->Consume();
    bool IsArrayOfTable = false;


    if
    (
      this->Peek() == Token::TokenType::LeftBracket
    ) {
      this->Consume();
      IsArrayOfTable = true;
    }

    const auto& FirstTok = this->Toks_[this->Cursor_];
    size_t StartOffset = FirstTok.Lexer_Size_t_Offset_;
    size_t EndOffset = StartOffset;

    while
    (
      this->Peek() != Token::TokenType::RightBracket &&
      this->Peek() != Token::TokenType::EndOfFile
    ) {
      const auto& Tok = this->Consume();
      EndOffset = Tok.Lexer_Size_t_Offset_ + Tok.Sv_Val_.length();
    }

    std::string_view ka_NameTok = this->Sv_SourceView_.substr(StartOffset, EndOffset - StartOffset);

    if
    (
      this->Peek() != Token::TokenType::RightBracket
    ) this->ReportError(this->Toks_[this->Cursor_], "Expected closing ']' for group");

    this->Consume();

    if
    (
      IsArrayOfTable
    ) {

      if
      (
        this->Peek() != Token::TokenType::RightBracket
      ) this->ReportError(this->Toks_[this->Cursor_], "Expected second closing ']' for array of tables.");

      this->Consume();
    }

    TableNode TablePayLoad
    {
      .name = ka_NameTok,
      .FirstChildIndx = NodeIdx::None,
      .IsArrayOfTable = IsArrayOfTable,
      ._pad = {}
    };

    const auto ka_NewTableIdx_ = this->Arena_.EmplaceNode(std::move(TablePayLoad));

    if
    (
      this->RootTableIdx == NodeIdx::None
    ) this->RootTableIdx = ka_NewTableIdx_;

    else [[
      /* nullAttr*/
    ]] {
      NodeIdx TailIdx = this->RootTableIdx;

      while
      (
        this->Arena_.GetNode(TailIdx).NextSiblingIndx != NodeIdx::None
      ) TailIdx = this->Arena_.GetNode(TailIdx).NextSiblingIndx;


      this->Arena_.GetNode(TailIdx).NextSiblingIndx = ka_NewTableIdx_;
    }

    this->LastTableIdx = ka_NewTableIdx_;
  }


  auto Parser::ParseArray(std::string_view KeyToken) noexcept -> NodeIdx
  {

    this->Consume();

    KeyValueNode KeyPayload
    {
      .Key = KeyToken,
      .Value = "",
      .TypeDisc = 4,
      ._pad = {}
    };
    const auto KeyValueIdx = this->Arena_.EmplaceNode(std::move(KeyPayload));

    ArrayNode ArrayPayload { .FirstChildIndx = NodeIdx::None };
    const auto ArrayIndx = this->Arena_.EmplaceNode(std::move(ArrayPayload));

    this->Arena_.GetNode(KeyValueIdx).NextSiblingIndx = ArrayIndx;

    while
    (
      this->Peek() != Token::TokenType::RightBracket &&
      this->Peek() != Token::TokenType::EndOfFile
    ) {

      const auto ElementIdx = this->ParseValue("");


    /*
      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
      After taking some breaks... Maybe one or two days.. I was having hard
      time understanding my own code.. It is unusual but number one cause may
      be the terrible naming conventions. But anyway.

      For the future me who may forget what this is... Let me write this:

      Let us say the Parent Table which we imagine as a teacher, leading a group
      of Kindergateners holdings hands in a line.

      * The Teacher only holds the hand of the first Kid (FirstChildIndx)
      * The Teacher does NOT know who is at the end of the line.
      * Each kid only holds the hand of the next kid in line (NextSiblingIndx)

      If a new kid shows up to join the group, the teacher can't just put them
      at the end of the line directly. The teacher has to look at the first kid,
      trace down the line of hands one by one until the teacher finds the kid
      who isn't holding anyone's hand and tell that kid to grab the new kid's
      hand.
      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    */

      auto& A_ArrayNode = this->Arena_.GetNode(ArrayIndx);
      auto& ArrayData = std::get<ArrayNode>(A_ArrayNode.Payload);

    /*
      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
      First, we look for the teacher. We look up the last [Group] we parsed and
      pull out its data so we can look at it.
      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    */

      if
      (
        ArrayData.FirstChildIndx == NodeIdx::None
      ) ArrayData.FirstChildIndx = ElementIdx;

    /*
      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
      Does the teacher have any kids yet? (Is FirstChildIndx empty!?).
      If no, then the teacher grabs the new kid's hand directly. The new key
      (KeyValueIdx) becomes the very first child. We are done here.
      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    */

      else [[
        /*nullAttr*/
      ]] {
        NodeIdx Current = ArrayData.FirstChildIndx;

      /*
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
        If the teacher already has kids in line, we have to find the end of the
        line. To do that, we start by looking at the very first kid. We assign
        them to our tracking variable called 'Current'.
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
      */

        while
        (
          this->Arena_.GetNode(Current).NextSiblingIndx != NodeIdx::None
        ) Current = this->Arena_.GetNode(Current).NextSiblingIndx;

      /*
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
        it is a while loop that translates to:
          "
            While the 'Current' kid is holding someone else's hand
            (NextSiblingIndx is NOT None), move our focus to that next kid.
          "

        It iterates over and over, hopping from kid to kid, until it finally
        lands on a kid whose 'NextSiblingIndx' is None. That means we've
        successfully found the last kid in line.
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
      */

        this->Arena_.GetNode(Current).NextSiblingIndx = ElementIdx;

      /*
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
        Now that 'Current' is officially the last kid in line, we give them an
        instruction:
          "
            Hey, your 'NextSiblingIndx' is now this new key (KeyValueIdx).
          "

        The new node is now successfully attached to the end of the chain.
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
      */

      }

      if
      (
        this->Peek() == Token::TokenType::Comma
      ) {
        this->Consume();

        if
        (
          this->Peek() == Token::TokenType::RightBracket
        ) this->ReportError(this->Toks_[this->Cursor_], "Expected another array element but got ']'");

      }

      else if
      (
        this->Peek() == Token::TokenType::RightBracket
      ) break;

      else [[
        /* nullAttr */
      ]] this->ReportError(this->Toks_[this->Cursor_], "Expected ',' or ']' after array elements.");
    }

    if
    (
      this->Peek() != Token::TokenType::RightBracket
    ) this->ReportError(this->Toks_[this->Cursor_], "Unterminated array block, missing closing ']'");

    this->Consume();

    return KeyValueIdx;
  }

  auto Parser::ParseInlineTable(std::string_view KeyToken) noexcept -> NodeIdx
  {
    this->Consume();

      KeyValueNode KeyPayload
      {
        .Key = KeyToken,
        .Value = "",
        .TypeDisc = 5, // Discriminator for InlineTable
        ._pad = {}
      };

      const NodeIdx KeyValueIndx = this->Arena_.EmplaceNode(
        std::move(KeyPayload)
      );

      InlineTableNode InlinePayload { .FirstChildIndx = NodeIdx::None };
      const auto InlineIndx = this->Arena_.EmplaceNode(
        std::move(InlinePayload)
      );

      this->Arena_.GetNode(KeyValueIndx).NextSiblingIndx = InlineIndx;

      while
      (
        this->Peek() != Token::TokenType::RightBrace &&
        this->Peek() != Token::TokenType::EndOfFile
      ) {

        if
        (
          this->Peek() != Token::TokenType::Identifier
        ) this->ReportError(this->Toks_[this->Cursor_], "Expected an identifier");

        const auto PropKeyToken = this->Consume();

        if
        (
          this->Peek() != Token::TokenType::Assign
        ) this->ReportError(this->Toks_[this->Cursor_], "Expected '=' inside inline table.");

        this->Consume();

        NodeIdx PropValueToken = this->ParseValue(PropKeyToken.Sv_Val_);

        auto& A_InlineNode = this->Arena_.GetNode(InlineIndx);
        auto& InlineData = std::get<InlineTableNode>(A_InlineNode.Payload);

        if
        (
          InlineData.FirstChildIndx == NodeIdx::None
        ) InlineData.FirstChildIndx = PropValueToken;

        else [[
          /* nullAttr */
        ]] {
          NodeIdx Current = InlineData.FirstChildIndx;

          while
          (
            this->Arena_.GetNode(Current).NextSiblingIndx != NodeIdx::None
          ) Current = this->Arena_.GetNode(Current).NextSiblingIndx;

          this->Arena_.GetNode(Current).NextSiblingIndx = PropValueToken;
        }

        if
        (
          this->Peek() == Token::TokenType::Comma
        ) {

          this->Consume();

          if
          (
            this->Peek() == Token::TokenType::RightBrace
          ) this->ReportError(this->Toks_[this->Cursor_], "Expected another key-value pair");

        }

        else if
        (
          this->Peek() == Token::TokenType::RightBrace
        ) break;

        else [[
          /* nullAttr */
        ]] this->ReportError(this->Toks_[this->Cursor_], "Expected ',' or '}' after inline table.");

      }

      if
      (
        this->Peek() != Token::TokenType::RightBrace
      ) this->ReportError(this->Toks_[this->Cursor_], "Unterminated inline table block, missing '}'");

      this->Consume();

      return KeyValueIndx;
  }

  auto Parser::ParseScalar(std::string_view KeyToken) noexcept -> NodeIdx
  {
    const auto& StartValueToken = this->Toks_[this->Cursor_];

    if
    (
      StartValueToken.Type == Token::TokenType::EndOfFile ||
      this->StartOfStatement()
    ) this->ReportError(StartValueToken, "Missing value assignment after '='");

    const size_t StartOffset = StartValueToken.Lexer_Size_t_Offset_;
    size_t EndOffset = StartOffset + StartValueToken.Sv_Val_.length();


    while
    (
      !this->StartOfStatement() &&
      this->Peek() != Token::TokenType::Comma &&
      this->Peek() != Token::TokenType::RightBracket &&
      this->Peek() != Token::TokenType::RightBrace &&
      this->Peek() != Token::TokenType::EndOfFile
    ) {
      const auto& ka_ContentToken_ = this->Consume();
      EndOffset = ka_ContentToken_.Lexer_Size_t_Offset_ + ka_ContentToken_.Sv_Val_.length();
    }

    const auto CombinedLength = EndOffset - StartOffset;

    std::string_view ValueView = this->Sv_SourceView_.substr(StartOffset, CombinedLength);

    KeyValueNode ValuePayload
    {
      .Key = KeyToken,
      .Value = ValueView,
      .TypeDisc = static_cast<std::uint32_t>(StartValueToken.Type),
      ._pad = {}
    };

    return this->Arena_.EmplaceNode(
      std::move(ValuePayload)
    );
  }

  auto Parser::ParseValue(std::string_view KeyToken) noexcept -> NodeIdx
  {
    if
    (
      this->Peek() == Token::TokenType::LeftBrace
    ) return this->ParseInlineTable(KeyToken);

    else if
    (
      this->Peek() == Token::TokenType::LeftBracket
    ) return this->ParseArray(KeyToken);

    else [[
      /* nullAttr */
    ]] return this->ParseScalar(KeyToken);
  }

  auto Parser::ParseKeyValue() noexcept -> void
  {

    if
    (
      this->LastTableIdx == NodeIdx::None
    ) this->ReportError(this->Toks_[this->Cursor_], "Key-value pair declared outside of a group block.");

    const auto& KeyToken = this->Consume();

    if
    (
      this->Peek() != Token::TokenType::Assign
    ) this->ReportError(this->Toks_[this->Cursor_], "Expected '=' operator after key identifier");

    this->Consume();

    NodeIdx ValueChainHead = this->ParseValue(KeyToken.Sv_Val_);

    auto& A_TableNode = this->Arena_.GetNode(this->LastTableIdx);
    auto& TableData = std::get<TableNode>(A_TableNode.Payload);

    if
    (
      TableData.FirstChildIndx == NodeIdx::None
    ) TableData.FirstChildIndx = ValueChainHead;

    else [[
      /* nullAttr */
    ]] {
      NodeIdx Current = TableData.FirstChildIndx;

      while
      (
        this->Arena_.GetNode(Current).NextSiblingIndx != NodeIdx::None
      ) Current = this->Arena_.GetNode(Current).NextSiblingIndx;

      this->Arena_.GetNode(Current).NextSiblingIndx = ValueChainHead;
    }
  }

  auto Parser::ReceiveToken(const Token::TokenData& Token) noexcept -> void
  {

    if
    (
      Token.Type != Token::TokenType::Unknown
    ) this->Toks_.emplace_back(Token);

    else [[
      /* nullAttr */
    ]] this->ReportError(Token, "Encountered unrecognized Lexer Symbol Context");

  }

  auto Parser::Parse() noexcept -> NodeIdx
  {

    std::size_t _st_LastParsedLine_ {};
    while
    (
      this->Cursor_ < this->Toks_.size() &&
      this->Toks_[this->Cursor_].Type != Token::TokenType::EndOfFile
    ) {

      const auto& Currentoken = this->Toks_[this->Cursor_];

      if
      (
        Currentoken.Type == Token::TokenType::Hash
      ) {

        _st_LastParsedLine_ = Currentoken.Lexer_Size_t_Line_;
        this->Cursor_++;
        continue;
      }

      if
      (

        _st_LastParsedLine_ != 0 &&
        (
          Currentoken.Lexer_Size_t_Line_ - _st_LastParsedLine_ > 1
        )
      ) this->LastTableIdx = this->RootTableIdx;


      if
      (
        this->Peek() == Token::TokenType::LeftBracket
      ) this->ParseTable();

      else if
      (
        this->Peek() == Token::TokenType::Identifier
      ) this->ParseKeyValue();

      else [[
        /* nullAttr */
      ]] this->ReportError(this->Toks_[this->Cursor_], "Unexpected structural token sequence");

      if
      (
        this->Cursor_ > 0
      ) _st_LastParsedLine_ = this->Toks_[this->Cursor_ - 1].Lexer_Size_t_Line_;
    }

    return this->RootTableIdx;
  }

  auto Parser::GetArena() const noexcept -> const ASTArena&
  {
    return this->Arena_;
  }
}
