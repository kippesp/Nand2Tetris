#include <list>
#include <string>

struct CliArgs {
  CliArgs(int argc, const char* argv[]);

  // returns list of input files found (or specified)
  const std::list<const std::string>& inputlist() const;

  static void show_usage();
  static void show_help();

  bool halt_after_tokenizer{false};
  bool halt_after_tokenizer_s_expression{false};

  bool halt_after_parse_tree_s_expression{false};

private:
  std::list<const std::string> filelist;
};
