#ifndef FILEHANDLER_HPP
#define FILEHANDLER_HPP

#include <filesystem>

namespace FileHandler
{
  class [[
    /* nullAttr */
  ]] FileWriter {
    private:
      int FileDesc{-1};

    public:
      explicit FileWriter(const char* FilePath) noexcept;

      ~FileWriter() noexcept;
      [[nodiscard]] bool IsOpen() const noexcept;

      bool Write(std::string_view data) noexcept;
  };

  class [[
    /* nullAttr */
  ]] FileOpener {
    private:
      std::string_view sourceView {};
      [[nodiscard]] constexpr auto load
      (
        std::filesystem::path const& FilePath_
      ) noexcept(true) -> bool;

      constexpr auto release(void) noexcept(true) -> void;
    public:
      explicit FileOpener(FileOpener const&) noexcept(true) = delete;

      auto operator = (FileOpener const&) -> FileOpener& = delete;

      explicit FileOpener(FileOpener&& other) noexcept(true) : sourceView {
        std::move(other.sourceView)
      } { other.sourceView = {}; }

      auto operator = (FileOpener&& other) noexcept(true) -> FileOpener&;

      FileOpener(void) noexcept(true) = default;

      FileOpener(std::filesystem::path const& FilePath_) noexcept(true);
      ~FileOpener(void) noexcept(true);

      [[nodiscard]]constexpr auto data(void)
      const noexcept(true) -> char const*
      {
        return this->sourceView.data();
      }

      [[nodiscard]]constexpr auto size(void)
      const noexcept(true) -> std::size_t
      {
        return this->sourceView.size();
      }

      [[nodiscard]]constexpr auto empty(void)
      const noexcept(true) -> bool
      {
        return this->sourceView.empty();
      }

      [[nodiscard]]constexpr auto view(void)
      const noexcept(true) -> std::string_view
      {
        return this->sourceView;
      }
  };
}
#endif
