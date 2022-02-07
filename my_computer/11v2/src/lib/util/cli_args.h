#include <list>
#include <string>

struct CliArgs {
  using filelist_t = std::list<std::string>;

  CliArgs(int argc, const char* argv[]);

  // returns list of input files found (or specified)
  const filelist_t& inputlist() const;

  static void show_usage();
  static void show_help();

  bool halt_after_tokenizer {false};

  bool halt_after_parse_tree_s_expression {false};
  bool halt_after_vmwriter {false};

  bool disable_precedence_parsing {false};

private:
  filelist_t filelist;
};
