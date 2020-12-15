#include <list>
#include <string>

struct CliArgs {
  CliArgs(int argc, const char* argv[]);

  // returns list of input files found (or specified)
  const std::list<const std::string>& inputlist();

  static void show_usage();
  static void show_help();

  bool halt_after_tokenizer{false};

private:
  std::list<const std::string> filelist;
};
