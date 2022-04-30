#include <array>
#include <fstream>
#include <iostream>

namespace {
std::pair<std::array<bool, 3>, bool> parse_opts(int argc, char ** argv)
{
    std::array<bool, 3> ans = {true, true, true};
    if (argc < 3) {
        std::cerr << "Not enough args passed!" << std::endl;
        return {ans, false};
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
                if (j == arg.length()) // Loop successfully ended
                {
                    continue; // Go through without falling to an error block
                }
            }
        }
        // If any condition fails, we fall here to show an error
        std::cerr << "Unable to parse option '" << arg << "'" << std::endl;
        return {ans, false};
    }
    return {ans, true};
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

void print_if_needed(std::ostream & strm, const std::array<bool, 3> & settings, bool outdated, std::string_view column, std::size_t index)
{
    if (!outdated && settings[index]) {
        print_line(strm, settings, column, index);
    }
}

} // anonymous namespace

int main(int argc, char ** argv)
{
    auto [settings, ret] = parse_opts(argc, argv);
    if (!ret) {
        std::cerr << "Syntax: \ncomm [OPTION] FILE1 FILE2" << std::endl;
        return 1;
    }

    std::ifstream file1;
    std::ifstream file2;
    file1.open(argv[argc - 2]);
    file2.open(argv[argc - 1]);

    if (!(file1.is_open() && file2.is_open())) {
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
            if (!std::getline(file1, cur_line1)) {
                break;
            }
            outdated1 = false;
        }
        if (outdated2) {
            if (!std::getline(file2, cur_line2)) {
                break;
            }
            outdated2 = false;
        }

        int cmp = cur_line1.compare(cur_line2);
        int index = 2;
        if (cmp <= 0) {
            outdated1 = true;
            index++;
        }
        if (cmp >= 0) {
            outdated2 = true;
            index--;
        }
        index %= 3;
        print_if_needed(std::cout, settings, false, index == 1 ? cur_line2 : cur_line1, index);
    }

    do {
        print_if_needed(std::cout, settings, outdated1, cur_line1, 0);
        outdated1 = false;
    } while (std::getline(file1, cur_line1));
    do {
        print_if_needed(std::cout, settings, outdated2, cur_line2, 1);
        outdated2 = false;
    } while (std::getline(file2, cur_line2));

    if (!(file1.eof() && file2.eof())) {
        std::cerr << "IO error occurred while reading files" << std::endl;
        exitcode = 1;
    }

    file1.close();
    file2.close();

    return exitcode;
}
