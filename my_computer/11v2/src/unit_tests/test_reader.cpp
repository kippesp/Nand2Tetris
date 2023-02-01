#include "common.h"

#include <cstdio>

using namespace jfcl;

SCENARIO("Simple Text Handling")
{
  SECTION("One Line w/no EOL")
  {
    TextReader r("one line");

    REQUIRE(r.get_current_line_number() == 1);
    REQUIRE(r.eof() == false);
    REQUIRE(r.read() == 'o');
    REQUIRE(r.read() == 'n');
    REQUIRE(r.read() == 'e');
    REQUIRE(r.read() == ' ');
    REQUIRE(r.read() == 'l');
    REQUIRE(r.read() == 'i');
    REQUIRE(r.read() == 'n');
    REQUIRE(r.eof() == false);
    REQUIRE(r.get_current_line_number() == 1);
    REQUIRE(r.read() == 'e');
    REQUIRE(r.eof() == true);
    REQUIRE(r.read() == '\0');
    REQUIRE(r.eof() == true);
    REQUIRE(r.read() == '\0');
    REQUIRE(r.peek() == '\0');
    REQUIRE(r.eof() == true);

    REQUIRE(r.get_line(0) == "one line");
    REQUIRE_THROWS(r.get_line(1));

    REQUIRE(r.num_lines() == 1);
  }

  SECTION("Two line")
  {
    TextReader r(
        "1st\n"
        "2nd\n");

    REQUIRE(r.read() == '1');
    REQUIRE(r.read() == 's');
    REQUIRE(r.read() == 't');
    REQUIRE(r.get_current_line_number() == 1);
    REQUIRE(r.read() == '\n');
    REQUIRE(r.get_current_line_number() == 2);
    REQUIRE(r.read() == '2');
    REQUIRE(r.read() == 'n');
    REQUIRE(r.read() == 'd');
    REQUIRE(r.peek() == '\n');
    REQUIRE(r.eof() == false);
    REQUIRE(r.get_current_line_number() == 2);
    REQUIRE(r.read() == '\n');
    REQUIRE(r.get_current_line_number() == 3);
    REQUIRE(r.eof() == true);
    REQUIRE(r.read() == '\0');
    REQUIRE(r.eof() == true);

    REQUIRE(r.get_line(0) == "1st");
    REQUIRE(r.get_line(1) == "2nd");

    REQUIRE(r.num_lines() == 2);
  }

  SECTION("One Line file")
  {
    FILE* fd = fopen("tmpfile", "w");
    REQUIRE(fd);
    fputs("line1", fd);
    fclose(fd);

    TextFileReader r("tmpfile");

    REQUIRE(r.get_line(0) == "line1");
    REQUIRE(r.read() == 'l');
    REQUIRE(r.read() == 'i');
    REQUIRE(r.read() == 'n');
    REQUIRE(r.read() == 'e');
    REQUIRE(r.eof() == false);
    REQUIRE(r.read() == '1');
    REQUIRE(r.eof() == true);
    REQUIRE(r.read() == '\0');
    REQUIRE(r.eof() == true);

    remove("tmpfile");
  }

  SECTION("Two Line file")
  {
    FILE* fd = fopen("tmpfile", "w");
    REQUIRE(fd);
    fputs("l1\n", fd);
    fputs("l2\n", fd);
    fclose(fd);

    TextFileReader r("tmpfile");

    REQUIRE(r.get_line(0) == "l1");
    REQUIRE(r.read() == 'l');
    REQUIRE(r.read() == '1');
    REQUIRE(r.read() == '\n');
    REQUIRE(r.read() == 'l');
    REQUIRE(r.read() == '2');
    REQUIRE(r.eof() == false);
    REQUIRE(r.read() == '\n');
    REQUIRE(r.eof() == true);
    REQUIRE(r.read() == '\0');
    REQUIRE(r.eof() == true);

    remove("tmpfile");
  }
}
