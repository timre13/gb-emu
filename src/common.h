#ifndef COMMON_H_
#define COMMON_H_

#include <iostream>

#ifndef NO_UNIMPLEMENTED_MESSAGE
#define UNIMPLEMENTED() do { std::cerr << "[UNIMPLEMENTED]: file: " << __FILE__ << ", function: " << __PRETTY_FUNCTION__ << ", line: " << __LINE__ << '\n'; std::cerr.flush(); } while (0)
#else
#define UNIMPLEMENTED() do {} while (0)
#endif

#ifndef NO_IMPOSSIBLE_MESSAGE
#define IMPOSSIBLE() do { std::cerr << "THE IMPOSSIBLE happened in file: " << __FILE__ << ", function: " << __PRETTY_FUNCTION__ << ", line: " << __LINE__ << '\n'; std::exit(99); } while (0)
#else
#define IMPOSSIBLE() do {} while (0)
#endif

#endif /* COMMON_H_ */
