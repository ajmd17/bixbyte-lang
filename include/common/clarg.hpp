#ifndef CLARG_HPP
#define CLARG_HPP

#include <algorithm>
#include <string>

class Clarg {
public:
  /** check if the option is set */
  static inline bool has(char **begin, char **end, const std::string &opt) {
    return std::find(begin, end, opt) != end;
  }

  /** retrieve the value that is found after an option */
  static inline char *get(char **begin, char **end, const std::string &opt) {
    char **it = std::find(begin, end, opt);

    if (it != end && ++it != end) {
        return *it;
    }

    return nullptr;
  }
};

#endif
