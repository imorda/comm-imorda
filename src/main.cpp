#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>

namespace {
std::optional<std::array<bool, 3>> parse_opts(int argc, char ** argv)
{
    std::array<bool, 3> ans = {true, true, true};
    if (argc < 3) {
        std::cerr << "Not enough args passed!" << std::endl;
        return std::nullopt;
    }

    for (int i = 1; i < argc - 2; ++i) {
        std::string_view arg(argv[i]);
        if (arg.length() >= 2) {
            if (arg[0] == '-') {
                std::size_t j;
                for (j = 1; j < arg.length(); ++j) {
                    int value = arg[j] - '0';
                    if (1 <= value && value <= 3) {
                        ans[value - 1] = false;
                    }
                    else {
                        break; // Exit abnormally
                    }
                }
                if (j == arg.length()) { // Loop successfully ended
                    continue;            // Go through without falling to an error block
                }
            }
        }
        // If any condition fails, we fall here to show an error
        std::cerr << "Unable to parse option '" << arg << "'" << std::endl;
        return std::nullopt;
    }
    return ans;
}

void print_line(std::ostream & strm, const std::array<bool, 3> & settings, std::string_view column, std::size_t index)
{
    for (std::size_t i = 0; i < index; ++i) {
        if (settings[i]) {
            strm << '\t';
        }
    }
    strm << column;
    strm << '\n';
}

std::shared_ptr<std::istream> get_file(char * filename)
{
    std::shared_ptr<std::istream> file;
    if (filename[0] == '-') {
        file.reset(&std::cin, [](...) {}); // Use empty deleter not to accidentally delete cin
    }
    else {
        file.reset(new std::ifstream(filename));
    }
    return file;
}

} // anonymous namespace

int main(int argc, char ** argv)
{
    auto settings = parse_opts(argc, argv);
    if (settings == std::nullopt) {
        std::cerr << "Syntax: \ncomm [OPTION] FILE1 FILE2" << std::endl;
        return 1;
    }

    auto file1 = get_file(argv[argc - 2]);
    auto file2 = get_file(argv[argc - 1]);

    if (!(*file1) || !(*file2)) {
        std::cerr << "Unable to open specified file" << std::endl;
        return 1;
    }

    int exitcode = 0;

    bool outdated1 = true;
    bool outdated2 = true;
    std::string cur_line1;
    std::string cur_line2;
    while (true) {
        if (outdated1) {
            if (!std::getline(*file1, cur_line1)) {
                break;
            }
            outdated1 = false;
        }
        if (outdated2) {
            if (!std::getline(*file2, cur_line2)) {
                break;
            }
            outdated2 = false;
        }

        int cmp = cur_line1.compare(cur_line2);
        std::size_t index;
        if (cmp < 0) {
            outdated1 = true;
            index = 0;
        }
        else if (cmp > 0) {
            outdated2 = true;
            index = 1;
        }
        else {
            outdated1 = true;
            outdated2 = true;
            index = 2;
        }
        if ((*settings)[index]) {
            print_line(std::cout, *settings, index == 1 ? cur_line2 : cur_line1, index);
        }
    }

    do {
        if ((*settings)[0] && !outdated1) {
            print_line(std::cout, *settings, cur_line1, 0);
        }
        outdated1 = false;
    } while (std::getline(*file1, cur_line1));
    do {
        if ((*settings)[1] && !outdated2) {
            print_line(std::cout, *settings, cur_line2, 1);
        }
        outdated2 = false;
    } while (std::getline(*file2, cur_line2));

    if (!file1->eof() || !file2->eof()) {
        std::cerr << "IO error occurred while reading files" << std::endl;
        exitcode = 1;
    }

    return exitcode;
}
